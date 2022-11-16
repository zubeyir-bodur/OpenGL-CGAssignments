#ifndef __ANGEL_QUAT_H__
#define __ANGEL_QUAT_H__
#pragma once

struct mat4;
struct vec4;
struct vec3;

namespace Angel
{
	/// <summary>
	/// Quat implementation just in case
	/// </summary>
	struct quat
	{
		float w, x, y, z;

		quat(const float d = float(0.0));

		quat(const float w_, const float x_, const float y_, const float z_);

		quat operator * (const quat& b);
	};

	vec3 x_dir(const quat& q);
	
	vec3 y_dir(const quat& q);
	
	vec3 z_dir(const quat& q);

	mat4 rotate(const quat& q);

	quat quat_from_axis_angle(const vec3& axis, const float theta_deg);

	float radians(float theta);
}
#endif;