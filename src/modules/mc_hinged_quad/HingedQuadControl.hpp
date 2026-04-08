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
 * @file HingedQuadControl.hpp
 *
 * Dual-axis hinged-arm quadrotor control module.
 *
 * Architecture
 * ============
 * This module sits between the standard PX4 attitude/rate controllers and
 * the actuator output layer.  It subscribes to vehicle_torque_setpoint and
 * vehicle_thrust_setpoint (published by the rate controller), reads the RC
 * channel selected for mode switching, and publishes actuator_motors and
 * actuator_servos.  It replaces the standard control_allocator for this
 * airframe (control_allocator is stopped in rc.board_extras).
 *
 * Flight modes
 * ============
 * 4-axis mode : All four motors are active.  Servos track the roll/pitch/yaw
 *               demand scaled by HQUAD_CTRL_SURF.
 *
 * Dual-axis   : One diagonal pair (HQUAD_FAIL_DIAG) ramps to idle over
 *               HQUAD_TRANS_TIME.  The surviving pair handles thrust and
 *               differential pitch or roll.  Servos provide yaw authority
 *               scaled by HQUAD_DAXIS_YAW.
 *
 * Transition  : Motor commands are linearly blended over HQUAD_TRANS_TIME.
 */

#pragma once

#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>

#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>
#include <uORB/topics/actuator_motors.h>
#include <uORB/topics/actuator_servos.h>
#include <uORB/topics/manual_control_setpoint.h>
#include <uORB/topics/parameter_update.h>
#include <uORB/topics/vehicle_thrust_setpoint.h>
#include <uORB/topics/vehicle_torque_setpoint.h>

using namespace time_literals;

class HingedQuadControl : public ModuleBase<HingedQuadControl>, public ModuleParams,
	public px4::ScheduledWorkItem
{
public:
	HingedQuadControl();
	~HingedQuadControl() override = default;

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	bool init();

private:
	void Run() override;
	void updateParams() override;

	/**
	 * Compute actuator mix and publish.
	 *
	 * @param thrust  Normalised collective thrust [0, 1]
	 * @param roll    Normalised roll torque  [-1, 1]
	 * @param pitch   Normalised pitch torque [-1, 1]
	 * @param yaw     Normalised yaw torque   [-1, 1]
	 * @param blend   0 = full 4-axis, 1 = full dual-axis
	 */
	void computeMix(float thrust, float roll, float pitch, float yaw, float blend);

	/* Subscriptions */
	uORB::SubscriptionCallbackWorkItem _vehicle_thrust_sub{this, ORB_ID(vehicle_thrust_setpoint)};
	uORB::Subscription _vehicle_torque_sub{ORB_ID(vehicle_torque_setpoint)};
	uORB::Subscription _manual_control_sub{ORB_ID(manual_control_setpoint)};
	uORB::Subscription _parameter_update_sub{ORB_ID(parameter_update)};

	/* Publications */
	uORB::Publication<actuator_motors_s> _actuator_motors_pub{ORB_ID(actuator_motors)};
	uORB::Publication<actuator_servos_s> _actuator_servos_pub{ORB_ID(actuator_servos)};

	/* Internal state */
	float       _transition_blend{0.f};  /**< 0 = full 4-axis, 1 = full dual-axis */
	hrt_abstime _last_run_time{0};

	DEFINE_PARAMETERS(
		(ParamInt<px4::params::HQUAD_MODE_CH>)      _param_mode_ch,
		(ParamFloat<px4::params::HQUAD_TRANS_TIME>) _param_trans_time,
		(ParamFloat<px4::params::HQUAD_DAXIS_YAW>)  _param_daxis_yaw,
		(ParamFloat<px4::params::HQUAD_CTRL_SURF>)  _param_ctrl_surf,
		(ParamInt<px4::params::HQUAD_FAIL_DIAG>)    _param_fail_diag
	)
};
