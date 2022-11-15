#ifndef __ANGEL_QUAT_H__
#define __ANGEL_QUAT_H__
#pragma once

struct mat4;
struct vec4;
struct vec3;

namespace Angel
{
	struct quat
	{
		float w, x, y, z;

		quat(const float d = float(0.0));

		quat(const float w_, const float x_, const float y_, const float z_);

		quat operator * (const quat& b);
	};

	float Determinant(const mat4& a);

	mat4 Adjugate(const mat4& a);

	mat4 Inverse(const mat4& m);

	vec3 XDir(const quat& q);
	
	vec3 YDir(const quat& q);
	
	vec3 ZDir(const quat& q);

	mat4 Rotate(const quat& q);

	quat Quat(const vec3& axis, const float theta_deg);
}
#endif;