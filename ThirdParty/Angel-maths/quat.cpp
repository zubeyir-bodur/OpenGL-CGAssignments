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

	float Determinant(const mat4& a)
	{
		return a[0].x * (a[1].y * a[2].z * a[3].w + a[3].y * a[1].z * a[2].w + a[2].y * a[3].z * a[1].w - a[1].y * a[3].z * a[2].w - a[2].y * a[1].z * a[3].w - a[3].y * a[2].z * a[1].w)
			+ a[0].y * (a[1].z * a[3].w * a[2].x + a[2].z * a[1].w * a[3].x + a[3].z * a[2].w * a[1].x - a[1].z * a[2].w * a[3].x - a[3].z * a[1].w * a[2].x - a[2].z * a[3].w * a[1].x)
			+ a[0].z * (a[1].w * a[2].x * a[3].y + a[3].w * a[1].x * a[2].y + a[2].w * a[3].x * a[1].y - a[1].w * a[3].x * a[2].y - a[2].w * a[1].x * a[3].y - a[3].w * a[2].x * a[1].y)
			+ a[0].w * (a[1].x * a[3].y * a[2].z + a[2].x * a[1].y * a[3].z + a[3].x * a[2].y * a[1].z - a[1].x * a[2].y * a[3].z - a[3].x * a[1].y * a[2].z - a[2].x * a[3].y * a[1].z);
	}

	Angel::mat4 Adjugate(const mat4& a)
	{
		return mat4
		{
			vec4
			{
				a[1].y * a[2].z * a[3].w + a[3].y * a[1].z * a[2].w + a[2].y * a[3].z * a[1].w - a[1].y * a[3].z * a[2].w - a[2].y * a[1].z * a[3].w - a[3].y * a[2].z * a[1].w,
				a[1].z * a[3].w * a[2].x + a[2].z * a[1].w * a[3].x + a[3].z * a[2].w * a[1].x - a[1].z * a[2].w * a[3].x - a[3].z * a[1].w * a[2].x - a[2].z * a[3].w * a[1].x,
				a[1].w * a[2].x * a[3].y + a[3].w * a[1].x * a[2].y + a[2].w * a[3].x * a[1].y - a[1].w * a[3].x * a[2].y - a[2].w * a[1].x * a[3].y - a[3].w * a[2].x * a[1].y,
				a[1].x * a[3].y * a[2].z + a[2].x * a[1].y * a[3].z + a[3].x * a[2].y * a[1].z - a[1].x * a[2].y * a[3].z - a[3].x * a[1].y * a[2].z - a[2].x * a[3].y * a[1].z,
			},
			vec4
			{
				a[0].y * a[3].z * a[2].w + a[2].y * a[0].z * a[3].w + a[3].y * a[2].z * a[0].w - a[3].y * a[0].z * a[2].w - a[2].y * a[3].z * a[0].w - a[0].y * a[2].z * a[3].w,
				a[0].z * a[2].w * a[3].x + a[3].z * a[0].w * a[2].x + a[2].z * a[3].w * a[0].x - a[0].z * a[3].w * a[2].x - a[2].z * a[0].w * a[3].x - a[3].z * a[2].w * a[0].x,
				a[0].w * a[3].x * a[2].y + a[2].w * a[0].x * a[3].y + a[3].w * a[2].x * a[0].y - a[0].w * a[2].x * a[3].y - a[3].w * a[0].x * a[2].y - a[2].w * a[3].x * a[0].y,
				a[0].x * a[2].y * a[3].z + a[3].x * a[0].y * a[2].z + a[2].x * a[3].y * a[0].z - a[0].x * a[3].y * a[2].z - a[2].x * a[0].y * a[3].z - a[3].x * a[2].y * a[0].z,
			},
			vec4
			{
				a[0].y * a[1].z * a[3].w + a[3].y * a[0].z * a[1].w + a[1].y * a[3].z * a[0].w - a[0].y * a[3].z * a[1].w - a[1].y * a[0].z * a[3].w - a[3].y * a[1].z * a[0].w,
				a[0].z * a[3].w * a[1].x + a[1].z * a[0].w * a[3].x + a[3].z * a[1].w * a[0].x - a[0].z * a[1].w * a[3].x - a[3].z * a[0].w * a[1].x - a[1].z * a[3].w * a[0].x,
				a[0].w * a[1].x * a[3].y + a[3].w * a[0].x * a[1].y + a[1].w * a[3].x * a[0].y - a[0].w * a[3].x * a[1].y - a[1].w * a[0].x * a[3].y - a[3].w * a[1].x * a[0].y,
				a[0].x * a[3].y * a[1].z + a[1].x * a[0].y * a[3].z + a[3].x * a[1].y * a[0].z - a[0].x * a[1].y * a[3].z - a[3].x * a[0].y * a[1].z - a[1].x * a[3].y * a[0].z
			},
			vec4
			{
				a[0].y * a[2].z * a[1].w + a[1].y * a[0].z * a[2].w + a[2].y * a[1].z * a[0].w - a[0].y * a[1].z * a[2].w - a[2].y * a[0].z * a[1].w - a[1].y * a[2].z * a[0].w,
				a[0].z * a[1].w * a[2].x + a[2].z * a[0].w * a[1].x + a[1].z * a[2].w * a[0].x - a[0].z * a[2].w * a[1].x - a[1].z * a[0].w * a[2].x - a[2].z * a[1].w * a[0].x,
				a[0].w * a[2].x * a[1].y + a[1].w * a[0].x * a[2].y + a[2].w * a[1].x * a[0].y - a[0].w * a[1].x * a[2].y - a[2].w * a[0].x * a[1].y - a[1].w * a[2].x * a[0].y,
				a[0].x * a[1].y * a[2].z + a[2].x * a[0].y * a[1].z + a[1].x * a[2].y * a[0].z - a[0].x * a[2].y * a[1].z - a[1].x * a[0].y * a[2].z - a[2].x * a[1].y * a[0].z
			}
		};
	}

	Angel::mat4 Inverse(const mat4& m)
	{
		mat4 adjugate = Adjugate(m);
		float det = 1.0f / Determinant(m);
		return mat4{ adjugate[0] * det, adjugate[1] * det, adjugate[2] * det , adjugate[3] * det };
	}

	Angel::vec3 XDir(const quat& q)
	{
		return vec3
		{
			q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z,
			(q.x * q.y + q.z * q.w) * 2,
			(q.z * q.x - q.y * q.w) * 2
		};
	}

	Angel::vec3 YDir(const quat& q)
	{
		return vec3
		{
			(q.x * q.y - q.z * q.w) * 2,
			q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z,
			(q.y * q.z + q.x * q.w) * 2
		};
	}

	Angel::vec3 ZDir(const quat& q)
	{
		return vec3
		{
			(q.z * q.x + q.y * q.w) * 2,
			(q.y * q.z - q.x * q.w) * 2,
			q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z
		};
	}

	Angel::mat4 Rotate(const quat& q)
	{
		mat4 rotation;
		const vec3 x = XDir(q);
		const vec3 y = YDir(q);
		const vec3 z = ZDir(q);
		rotation[0] = { x.x, y.x, z.x, 0 };
		rotation[1] = { x.y, y.y, z.y, 0 };
		rotation[2] = { x.z, y.z, z.z, 0 };
		rotation[3] = { 0,   0,   0, 1 };
		return rotation;
	}

	Angel::quat Quat(const vec3& axis, const float theta_deg)
	{
		const vec3 xyz = axis * sinf(theta_deg * DegreesToRadians / 2.0f);
		return{ cosf(theta_deg * DegreesToRadians / 2.0f), xyz.x, xyz.y, xyz.z };
	}

}