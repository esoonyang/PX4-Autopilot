/****************************************************************************
 *
 *   Copyright (c) 2024 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file HingedQuadControl.cpp
 *
 * Dual-axis hinged-arm quadrotor control module.
 *
 * Motor / geometry convention (plus quad, PX4 body frame X=fwd, Y=left, Z=up):
 *
 *          M1 (front, CCW)   idx 0
 *               |
 *  M4 (left,CW) +------ M2 (right, CW)   idx 1
 *               |         idx 3
 *          M3 (rear, CCW)   idx 2
 *
 * Servo mapping (actuator_servos, 0-based):
 *   idx 0 = S1 below M1 (front)
 *   idx 1 = S2 below M2 (right)
 *   idx 2 = S3 below M3 (rear)
 *   idx 3 = S4 below M4 (left)
 *
 * 4-axis standard plus mix:
 *   M1(0)  thrust + pitch + yaw
 *   M2(1)  thrust - roll  - yaw
 *   M3(2)  thrust - pitch + yaw
 *   M4(3)  thrust + roll  - yaw
 *
 * Dual-axis (HQUAD_FAIL_DIAG=0, M1/M3 idle, M2/M4 active):
 *   M2  thrust - roll  - yaw * DAXIS_MOTOR_YAW_FRACTION
 *   M4  thrust + roll  + yaw * DAXIS_MOTOR_YAW_FRACTION
 *   Yaw primarily via differential servo deflection.
 */

#include "HingedQuadControl.hpp"

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <lib/mathlib/mathlib.h>

using math::constrain;

/**
 * Fraction of yaw torque mixed into active motors in dual-axis mode.
 * The bulk of yaw authority comes from the control surfaces; this small
 * motor contribution improves initial responsiveness.
 */
static constexpr float DAXIS_MOTOR_YAW_FRACTION = 0.3f;

HingedQuadControl::HingedQuadControl() :
	ModuleParams(nullptr),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::rate_ctrl)
{
}

bool HingedQuadControl::init()
{
	if (!_vehicle_thrust_sub.registerCallback()) {
		PX4_ERR("vehicle_thrust_setpoint callback registration failed");
		return false;
	}

	ScheduleNow();
	return true;
}

void HingedQuadControl::updateParams()
{
	ModuleParams::updateParams();
}

void HingedQuadControl::Run()
{
	if (should_exit()) {
		_vehicle_thrust_sub.unregisterCallback();
		exit_and_cleanup();
		return;
	}

	if (_parameter_update_sub.updated()) {
		parameter_update_s pupdate;
		_parameter_update_sub.copy(&pupdate);
		updateParams();
	}

	vehicle_thrust_setpoint_s thrust_sp{};

	if (!_vehicle_thrust_sub.copy(&thrust_sp)) {
		return;
	}

	vehicle_torque_setpoint_s torque_sp{};
	_vehicle_torque_sub.copy(&torque_sp);

	manual_control_setpoint_s manual{};
	_manual_control_sub.copy(&manual);

	/* Determine mode switch from RC channel (1=aux1 … 6=aux6) */
	bool dual_axis_requested = false;
	const int ch = _param_mode_ch.get();

	switch (ch) {
	case 1:  dual_axis_requested = (manual.aux1 > 0.5f); break;
	case 2:  dual_axis_requested = (manual.aux2 > 0.5f); break;
	case 3:  dual_axis_requested = (manual.aux3 > 0.5f); break;
	case 4:  dual_axis_requested = (manual.aux4 > 0.5f); break;
	case 5:  dual_axis_requested = (manual.aux5 > 0.5f); break;
	case 6:  dual_axis_requested = (manual.aux6 > 0.5f); break;
	default: break;
	}

	/* Advance blend factor linearly */
	const hrt_abstime now = hrt_absolute_time();
	float dt = 0.f;

	if (_last_run_time != 0) {
		dt = (float)(now - _last_run_time) * 1e-6f;
	}

	_last_run_time = now;

	const float trans_rate = (dt > 0.f && _param_trans_time.get() > 0.f)
				 ? dt / _param_trans_time.get()
				 : 0.f;

	if (dual_axis_requested) {
		_transition_blend = constrain(_transition_blend + trans_rate, 0.f, 1.f);

	} else {
		_transition_blend = constrain(_transition_blend - trans_rate, 0.f, 1.f);
	}

	/* Extract normalised commands (NED thrust: down positive → negate for throttle) */
	const float thrust = -thrust_sp.xyz[2];
	const float roll   =  torque_sp.xyz[0];
	const float pitch  =  torque_sp.xyz[1];
	const float yaw    =  torque_sp.xyz[2];

	computeMix(thrust, roll, pitch, yaw, _transition_blend);
}

void HingedQuadControl::computeMix(float thrust, float roll, float pitch, float yaw, float blend)
{
	/*
	 * HQUAD_FAIL_DIAG selects which diagonal becomes idle:
	 *   0 → M0(front) + M2(rear)  idle;  M1(right) + M3(left) active
	 *   1 → M1(right) + M3(left)  idle;  M0(front) + M2(rear) active
	 */
	const bool fail_ns = (_param_fail_diag.get() == 0);

	const int idx_act_a  = fail_ns ? 1 : 0;
	const int idx_act_b  = fail_ns ? 3 : 2;

	/* ---- 4-axis plus mix -------------------------------------------------- */
	float motor_4ax[4];
	motor_4ax[0] = constrain(thrust + pitch + yaw, 0.f, 1.f);
	motor_4ax[1] = constrain(thrust - roll  - yaw, 0.f, 1.f);
	motor_4ax[2] = constrain(thrust - pitch + yaw, 0.f, 1.f);
	motor_4ax[3] = constrain(thrust + roll  - yaw, 0.f, 1.f);

	/* ---- Dual-axis mix ---------------------------------------------------- */
	float motor_dax[4] = {0.f, 0.f, 0.f, 0.f};

	if (fail_ns) {
		/* Active = M2(idx1, right) + M4(idx3, left) — E-W axis */
		motor_dax[idx_act_a] = constrain(thrust - roll - yaw * DAXIS_MOTOR_YAW_FRACTION, 0.f, 1.f);
		motor_dax[idx_act_b] = constrain(thrust + roll + yaw * DAXIS_MOTOR_YAW_FRACTION, 0.f, 1.f);

	} else {
		/* Active = M1(idx0, front) + M3(idx2, rear) — N-S axis */
		motor_dax[idx_act_a] = constrain(thrust + pitch + yaw * DAXIS_MOTOR_YAW_FRACTION, 0.f, 1.f);
		motor_dax[idx_act_b] = constrain(thrust - pitch - yaw * DAXIS_MOTOR_YAW_FRACTION, 0.f, 1.f);
	}

	/* ---- Blend and publish motors ---------------------------------------- */
	actuator_motors_s motors{};
	motors.timestamp        = hrt_absolute_time();
	motors.timestamp_sample = motors.timestamp;

	for (int i = 0; i < 4; i++) {
		motors.control[i] = constrain(
					    motor_4ax[i] * (1.f - blend) + motor_dax[i] * blend,
					    0.f, 1.f);
	}

	_actuator_motors_pub.publish(motors);

	/* ---- Servo / control surface mix -------------------------------------
	 * 4-axis:  servos provide supplementary aerodynamic moments
	 *   S0 (front): +pitch +yaw
	 *   S1 (right): -roll  -yaw
	 *   S2 (rear):  -pitch +yaw
	 *   S3 (left):  +roll  -yaw
	 *
	 * Dual-axis: all four servos are primary yaw actuators;
	 *   differential deflection produces yaw torque about Z.
	 */
	const float cs  = _param_ctrl_surf.get();
	const float dyw = _param_daxis_yaw.get();

	float servo_4ax[4];
	servo_4ax[0] = cs * ( pitch + yaw);
	servo_4ax[1] = cs * (-roll  - yaw);
	servo_4ax[2] = cs * (-pitch + yaw);
	servo_4ax[3] = cs * ( roll  - yaw);

	const float yc = yaw * dyw * cs;
	float servo_dax[4];
	servo_dax[0] =  yc;
	servo_dax[1] = -yc;
	servo_dax[2] =  yc;
	servo_dax[3] = -yc;

	actuator_servos_s servos{};
	servos.timestamp = hrt_absolute_time();

	for (int i = 0; i < 4; i++) {
		servos.control[i] = constrain(
					    servo_4ax[i] * (1.f - blend) + servo_dax[i] * blend,
					    -1.f, 1.f);
	}

	_actuator_servos_pub.publish(servos);
}

int HingedQuadControl::task_spawn(int argc, char *argv[])
{
	HingedQuadControl *instance = new HingedQuadControl();

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init()) {
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

int HingedQuadControl::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int HingedQuadControl::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Dual-axis hinged-arm quadrotor control module for the HAKRC hinged quad
airframe.

Monitors an RC channel for mode switch requests and smoothly transitions
between standard 4-motor operation and a reduced dual-axis mode in which
one diagonal pair of motors is ramped to idle while control surfaces provide
yaw authority.
)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("mc_hinged_quad", "controller");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

	return 0;
}

extern "C" __EXPORT int mc_hinged_quad_main(int argc, char *argv[])
{
	return HingedQuadControl::main(argc, argv);
}
