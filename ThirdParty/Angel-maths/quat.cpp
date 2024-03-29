#include "mat.h"
#include "quat.hpp"

namespace Angel
{

	quat::quat(const float d /*= float(0.0)*/)
	{
		w = d; x = d;  y = d;  z = d;
	}

	quat::quat(const float w_, const float x_, const float y_, const float z_)
	{
		w = w_; x = x_;  y = y_;  z = z_;
	}

	Angel::quat quat::operator*(const quat& b)
	{
		return quat
		{
			w * b.w - x * b.x - y * b.y - z * b.z,
			w * b.x + b.w * x + y * b.z - b.y * z,
			w * b.y + b.w * y + z * b.x - b.z * x,
			w * b.z + b.w * z + x * b.y - b.x * y,
		};
	}

	Angel::vec3 x_dir(const quat& q)
	{
		return vec3
		{
			q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z,
			(q.x * q.y + q.z * q.w) * 2,
			(q.z * q.x - q.y * q.w) * 2
		};
	}

	Angel::vec3 y_dir(const quat& q)
	{
		return vec3
		{
			(q.x * q.y - q.z * q.w) * 2,
			q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z,
			(q.y * q.z + q.x * q.w) * 2
		};
	}

	Angel::vec3 z_dir(const quat& q)
	{
		return vec3
		{
			(q.z * q.x + q.y * q.w) * 2,
			(q.y * q.z - q.x * q.w) * 2,
			q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z
		};
	}

	Angel::mat4 rotate(const quat& q)
	{
		mat4 rotation;
		const vec3 x = x_dir(q);
		const vec3 y = y_dir(q);
		const vec3 z = z_dir(q);
		rotation[0] = { x.x, y.x, z.x, 0 };
		rotation[1] = { x.y, y.y, z.y, 0 };
		rotation[2] = { x.z, y.z, z.z, 0 };
		rotation[3] = { 0,   0,   0, 1 };
		return rotation;
	}

	Angel::quat quat_from_axis_angle(const vec3& axis, const float theta_deg)
	{
		const vec3 xyz = axis * sinf(theta_deg * DegreesToRadians / 2.0f);
		return{ cosf(theta_deg * DegreesToRadians / 2.0f), xyz.x, xyz.y, xyz.z };
	}

	float radians(float theta)
	{
		return theta * DegreesToRadians;
	}

}