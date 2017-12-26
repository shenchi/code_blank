#pragma once

//#define TOFU_USE_GLM

#if defined(TOFU_FORCE_GLM)

#ifndef TOFU_USE_GLM
#define TOFU_USE_GLM
#endif

#elif defined(TOFU_FORCE_NOT_GLM)

#ifdef TOFU_USE_GLM
#undef TOFU_USE_GLM
#endif

#endif

#ifdef TOFU_USE_GLM

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#else // TOFU_USE_GLM

#include <cstdint>
#include <cmath>

#endif // TOFU_USE_GLM

#ifndef TF_INLINE
#ifdef _MSC_VER
#define TF_INLINE __forceinline
#else
#define TF_INLINE inline
#endif
#endif

namespace tofu
{
	namespace math
	{
		constexpr float PI = 3.141592653589f;

#ifdef TOFU_USE_GLM
		typedef glm::vec2 float2;
		typedef glm::vec3 float3;
		typedef glm::vec4 float4;
		typedef glm::quat quat;

		typedef glm::ivec2 int2;
		typedef glm::ivec3 int3;
		typedef glm::ivec4 int4;

		typedef glm::mat4 float4x4;

		using namespace glm;

		// order : roll, pitch, yaw
		TF_INLINE quat euler(float pitch, float yaw, float roll)
		{
			float cp = std::cosf(pitch * 0.5f);
			float sp = std::sinf(pitch * 0.5f);
			float cy = std::cosf(yaw * 0.5f);
			float sy = std::sinf(yaw * 0.5f);
			float cr = std::cosf(roll * 0.5f);
			float sr = std::sinf(roll * 0.5f);

			//x = sy * cp * sr + cy * sp * cr;
			//y = sy * cp * cr - cy * sp * sr;
			//z = cy * cp * sr - sy * sp * cr;
			//w =	sy * sp * sr + cy * cp * cr;

			return quat(
				sy * sp * sr + cy * cp * cr, // this is w
				sy * cp * sr + cy * sp * cr,
				sy * cp * cr - cy * sp * sr,
				cy * cp * sr - sy * sp * cr);
		}

		//// apply in order of scale, rotation, translation
		//TF_INLINE float4x4 transform(const float3& t, const quat& r, const float3& s)
		//{

		//	float a_sqr = r.w * r.w;
		//	float b_sqr = r.x * r.x;
		//	float c_sqr = r.y * r.y;
		//	float d_sqr = r.z * r.z;

		//	float a_b_2 = r.w * r.x * 2;
		//	float a_c_2 = r.w * r.y * 2;
		//	float a_d_2 = r.w * r.z * 2;

		//	float b_c_2 = r.x * r.y * 2;
		//	float b_d_2 = r.x * r.z * 2;

		//	float c_d_2 = r.y * r.z * 2;

		//	return float4x4(
		//		s.x * (a_sqr + b_sqr - c_sqr - d_sqr), s.y * (b_c_2 - a_d_2), s.z * (a_c_2 + b_d_2), t.x,
		//		s.x * (a_d_2 + b_c_2), s.y * (a_sqr - b_sqr + c_sqr - d_sqr), s.z * (c_d_2 - a_b_2), t.y,
		//		s.x * (b_d_2 - a_c_2), s.y * (a_b_2 + c_d_2), s.z * (a_sqr - b_sqr - c_sqr + d_sqr), t.z,
		//		0.0f, 0.0f, 0.0f, 1.0f
		//	);
		//}

		//TF_INLINE float4x4 lookTo(const float3& position, const float3& direction, const float3& up)
		//{
		//	return lookAt(position, position + direction, up);
		//}

#else

		template<typename T>
		struct vec2
		{
			T	x;
			T	y;
		};

		template<typename T>
		struct vec3
		{
			T	x;
			T	y;
			T	z;
		};

		template<typename T>
		struct vec4
		{
			T	x;
			T	y;
			T	z;
			T	w;
		};

		typedef vec2<float>	float2;
		typedef vec3<float>	float3;
		typedef vec4<float>	float4;

		struct float4x4
		{
			float4 x;
			float4 y;
			float4 z;
			float4 w;
		};

		typedef vec2<int32_t>	int2;
		typedef vec3<int32_t>	int3;
		typedef vec4<int32_t>	int4;

		typedef vec2<uint32_t>	uint2;
		typedef vec3<uint32_t>	uint3;
		typedef vec4<uint32_t>	uint4;

		TF_INLINE float mix(float a, float b, float t)
		{
			return a * (1.0f - t) + b * t;
		}

		// float2

		TF_INLINE float2& operator += (float2& a, const float2 b)
		{
			a.x += b.x;
			a.y += b.y;
			return a;
		}

		TF_INLINE float2& operator -= (float2& a, const float2 b)
		{
			a.x -= b.x;
			a.y -= b.y;
			return a;
		}

		TF_INLINE float2& operator *= (float2& a, const float2 b)
		{
			a.x *= b.x;
			a.y *= b.y;
			return a;
		}

		TF_INLINE float2& operator *= (float2& a, float b)
		{
			a.x *= b;
			a.y *= b;
			return a;
		}

		TF_INLINE float2& operator /= (float2& a, float b)
		{
			a.x /= b;
			a.y /= b;
			return a;
		}

		TF_INLINE float2 operator + (const float2& a, const float2& b)
		{
			return float2{ a.x + b.x, a.y + b.y };
		}

		TF_INLINE float2 operator - (const float2& a, const float2& b)
		{
			return float2{ a.x - b.x, a.y - b.y };
		}

		TF_INLINE float2 operator - (const float2& a)
		{
			return float2{ -a.x, -a.y };
		}

		TF_INLINE float2 operator * (const float2& a, const float2& b)
		{
			return float2{ a.x * b.x, a.y * b.y };
		}

		TF_INLINE float2 operator * (const float2& a, float b)
		{
			return float2{ a.x * b, a.y * b };
		}

		TF_INLINE float2 operator * (float a, const float2& b)
		{
			return float2{ a * b.x, a * b.y };
		}

		TF_INLINE float2 operator / (const float2& a, float b)
		{
			return float2{ a.x / b, a.y / b };
		}

		TF_INLINE float dot(const float2& a, const float2& b)
		{
			return a.x * b.x + a.y * b.y;
		}

		TF_INLINE float cross(const float2& a, const float2& b)
		{
			return a.x * b.y - a.y * b.x;
		}

		TF_INLINE float length(const float2& a)
		{
			return std::sqrtf(dot(a, a));
		}

		TF_INLINE float2 normalize(const float2& a)
		{
			float l = length(a);
			return a / l;
		}

		TF_INLINE float2 mix(const float2& a, const float2& b, float t)
		{
			return a * (1.0f - t) + b * t;
		}

		// float3

		TF_INLINE float3& operator += (float3& a, const float3 b)
		{
			a.x += b.x;
			a.y += b.y;
			a.z += b.z;
			return a;
		}

		TF_INLINE float3& operator -= (float3& a, const float3 b)
		{
			a.x -= b.x;
			a.y -= b.y;
			a.z -= b.z;
			return a;
		}

		TF_INLINE float3& operator *= (float3& a, const float3 b)
		{
			a.x *= b.x;
			a.y *= b.y;
			a.z *= b.z;
			return a;
		}

		TF_INLINE float3& operator *= (float3& a, float b)
		{
			a.x *= b;
			a.y *= b;
			a.z *= b;
			return a;
		}

		TF_INLINE float3& operator /= (float3& a, float b)
		{
			a.x /= b;
			a.y /= b;
			a.z /= b;
			return a;
		}

		TF_INLINE float3 operator + (const float3& a, const float3& b)
		{
			return float3{ a.x + b.x, a.y + b.y, a.z + b.z };
		}

		TF_INLINE float3 operator - (const float3& a, const float3& b)
		{
			return float3{ a.x - b.x, a.y - b.y, a.z - b.z };
		}

		TF_INLINE float3 operator - (const float3& a)
		{
			return float3{ -a.x, -a.y, -a.z };
		}

		TF_INLINE float3 operator * (const float3& a, const float3& b)
		{
			return float3{ a.x * b.x, a.y * b.y, a.z * b.z };
		}

		TF_INLINE float3 operator * (const float3& a, float b)
		{
			return float3{ a.x * b, a.y * b, a.z * b };
		}

		TF_INLINE float3 operator * (float a, const float3& b)
		{
			return float3{ a * b.x, a * b.y, a * b.z };
		}

		TF_INLINE float3 operator / (const float3& a, float b)
		{
			return float3{ a.x / b, a.y / b, a.z / b };
		}

		TF_INLINE float dot(const float3& a, const float3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		TF_INLINE float3 cross(const float3& a, const float3& b)
		{
			return float3{
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
			};
		}

		TF_INLINE float length(const float3& a)
		{
			return std::sqrtf(dot(a, a));
		}

		TF_INLINE float3 normalize(const float3& a)
		{
			float l = length(a);
			return a / l;
		}

		TF_INLINE float3 mix(const float3& a, const float3& b, float t)
		{
			return a * (1.0f - t) + b * t;
		}

		// float4

		TF_INLINE float4& operator += (float4& a, const float4 b)
		{
			a.x += b.x;
			a.y += b.y;
			a.z += b.z;
			a.w += b.w;
			return a;
		}

		TF_INLINE float4& operator -= (float4& a, const float4 b)
		{
			a.x -= b.x;
			a.y -= b.y;
			a.z -= b.z;
			a.w -= b.w;
			return a;
		}

		TF_INLINE float4& operator *= (float4& a, const float4 b)
		{
			a.x *= b.x;
			a.y *= b.y;
			a.z *= b.z;
			a.w *= b.w;
			return a;
		}

		TF_INLINE float4& operator *= (float4& a, float b)
		{
			a.x *= b;
			a.y *= b;
			a.z *= b;
			a.w *= b;
			return a;
		}

		TF_INLINE float4& operator /= (float4& a, float b)
		{
			a.x /= b;
			a.y /= b;
			a.z /= b;
			a.w /= b;
			return a;
		}

		TF_INLINE float4 operator + (const float4& a, const float4& b)
		{
			return float4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
		}

		TF_INLINE float4 operator - (const float4& a, const float4& b)
		{
			return float4{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
		}

		TF_INLINE float4 operator - (const float4& a)
		{
			return float4{ -a.x, -a.y, -a.z, -a.w };
		}

		TF_INLINE float4 operator * (const float4& a, const float4& b)
		{
			return float4{ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
		}

		TF_INLINE float4 operator * (const float4& a, float b)
		{
			return float4{ a.x * b, a.y * b, a.z * b, a.w * b };
		}

		TF_INLINE float4 operator * (float a, const float4& b)
		{
			return float4{ a * b.x, a * b.y, a * b.z, a * b.w };
		}

		TF_INLINE float4 operator / (const float4& a, float b)
		{
			return float4{ a.x / b, a.y / b, a.z / b, a.w / b };
		}

		TF_INLINE float dot(const float4& a, const float4& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		// w is ignored
		TF_INLINE float4 cross(const float4& a, const float4& b)
		{
			return float4{
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
			};
		}

		TF_INLINE float length(const float4& a)
		{
			return std::sqrtf(dot(a, a));
		}

		TF_INLINE float4 normalize(const float4& a)
		{
			float l = length(a);
			return a / l;
		}

		TF_INLINE float4 mix(const float4& a, const float4& b, float t)
		{
			return a * (1.0f - t) + b * t;
		}

		// quaternion

		TF_INLINE float4 hamilton(const float4& a, const float4& b)
		{
			return float4{
				 a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x,
				-a.x * b.z + a.y * b.w + a.z * b.x + a.w * b.y,
				 a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z,
				-a.x * b.x - a.y * b.y - a.z * b.z + a.w * b.w
			};
		}

		struct quat
		{
			//    i  j  k  1
			float x, y, z, w;

			TF_INLINE quat()
				:
				x(0.0f), y(0.0f), z(0.0f), w(1.0f)
			{ }

			TF_INLINE quat(const float4& v)
				:
				x(v.x), y(v.y), z(v.z), w(v.w)
			{ }

			TF_INLINE quat(float _x, float _y, float _z, float _w)
				:
				x(_x), y(_y), z(_z), w(_w)
			{ }

			//TF_INLINE quat(float theta, const float3& axis)
			//{
			//	float s = std::sinf(theta * 0.5f);
			//	float c = std::cosf(theta * 0.5f);
			//	x = s * axis.x;
			//	y = s * axis.y;
			//	z = s * axis.z;
			//	w = c;
			//}


			//TF_INLINE operator float4() const
			//{
			//	return reinterpret_cast<const float4&>(*this);// float4{ x, y, z, w };
			//}


			// a * b,  apply rotation b and then rotation a
			TF_INLINE quat operator* (const quat& b) const
			{
				return quat{
					x * b.w + y * b.z - z * b.y + w * b.x,
					-x * b.z + y * b.w + z * b.x + w * b.y,
					x * b.y - y * b.x + z * b.w + w * b.z,
					-x * b.x - y * b.y - z * b.z + w * b.w
				};
			}

			TF_INLINE float3 to_eular() const
			{
				return float3();
			}
		};

		TF_INLINE quat conjugate(const quat& q)
		{
			return quat(-q.x, -q.y, -q.z, q.w);
		}

		TF_INLINE float dot(const quat& a, const quat& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		TF_INLINE float length(const quat& q)
		{
			return std::sqrtf(dot(q, q));
		}

		TF_INLINE quat operator+ (const quat& a, const quat& b)
		{
			return quat{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
		}

		TF_INLINE quat operator- (const quat& a, const quat& b)
		{
			return quat{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
		}

		TF_INLINE quat operator- (const quat& a)
		{
			return quat{ -a.x,  -a.y,  -a.z,  -a.w };
		}

		TF_INLINE float3 operator* (const quat& q, const float3& v)
		{
			quat v4{ v.x, v.y, v.z, 0.0f };
			v4 = (q * v4) * conjugate(q);
			return float3{ v4.x, v4.y, v4.z };
		}

		TF_INLINE quat operator* (const quat& q, float f)
		{
			return quat{ f * q.x, f * q.y, f * q.z, f * q.w };
		}

		TF_INLINE quat operator* (float f, const quat& q)
		{
			return quat{ f * q.x, f * q.y, f * q.z, f * q.w };
		}

		TF_INLINE quat operator/ (const quat& q, float f)
		{
			return quat{ q.x / f, q.y / f, q.z / f, q.w / f };
		}

		TF_INLINE quat angleAxis(float theta, const float3& axis)
		{
			float s = std::sinf(theta * 0.5f);
			float c = std::cosf(theta * 0.5f);
			//x = s * axis.x;
			//y = s * axis.y;
			//z = s * axis.z;
			//w = c;
			return quat(s * axis.x, s * axis.y, s * axis.z, c);
		}

		// order : roll, pitch, yaw
		TF_INLINE quat euler(float pitch, float yaw, float roll)
		{
			float cp = std::cosf(pitch * 0.5f);
			float sp = std::sinf(pitch * 0.5f);
			float cy = std::cosf(yaw * 0.5f);
			float sy = std::sinf(yaw * 0.5f);
			float cr = std::cosf(roll * 0.5f);
			float sr = std::sinf(roll * 0.5f);

			//x = sy * cp * sr + cy * sp * cr;
			//y = sy * cp * cr - cy * sp * sr;
			//z = cy * cp * sr - sy * sp * cr;
			//w =	sy * sp * sr + cy * cp * cr;

			return quat(
				sy * cp * sr + cy * sp * cr,
				sy * cp * cr - cy * sp * sr,
				cy * cp * sr - sy * sp * cr,
				sy * sp * sr + cy * cp * cr);
		}

		TF_INLINE quat mix(const quat& a, const quat& b, float t)
		{
			return a * (1.0f - t) + b * t;
		}

		TF_INLINE quat slerp(const quat& a, const quat& b, float t)
		{
			quat c = b;

			float cosAB = dot(a, b);

			if (cosAB < 0.0f)
			{
				cosAB = -cosAB;
				c = -b;
			}

			if (cosAB > 0.9995f)
			{
				return mix(a, b, t);
			}

			float omega = std::acosf(cosAB);

			return (std::sinf((1.0f - t) * omega) * a + std::sinf(t * omega) * c) / std::sinf(omega);
		}

		// float4x4

		// row vector
		// TODO optimize it
		TF_INLINE float4 operator * (const float4& a, const float4x4& b)
		{
			return float4{
				a.x * b.x.x + a.y * b.y.x + a.z * b.z.x + a.w * b.w.x,
				a.x * b.x.y + a.y * b.y.y + a.z * b.z.y + a.w * b.w.y,
				a.x * b.x.z + a.y * b.y.z + a.z * b.z.z + a.w * b.w.z,
				a.x * b.x.w + a.y * b.y.w + a.z * b.z.w + a.w * b.w.w
			};
		}

		// column vector
		// TODO optimize it
		TF_INLINE float4 operator * (const float4x4& a, const float4& b)
		{
			return float4{
				a.x.x * b.x + a.x.y * b.y + a.x.z * b.z + a.x.w * b.w,
				a.y.x * b.x + a.y.y * b.y + a.y.z * b.z + a.y.w * b.w,
				a.z.x * b.x + a.z.y * b.y + a.z.z * b.z + a.z.w * b.w,
				a.w.x * b.x + a.w.y * b.y + a.w.z * b.z + a.w.w * b.w
			};
		}

		// TODO optimize it
		TF_INLINE float4x4 operator * (const float4x4& a, const float4x4& b)
		{
			return float4x4{
				operator * (a.x, b),
				operator * (a.y, b),
				operator * (a.z, b),
				operator * (a.w, b)
			};
		}

		TF_INLINE float4x4 mix(const float4x4& a, const float4x4& b, float t)
		{
			return float4x4
			{
				mix(a.x, b.x, t),
				mix(a.y, b.y, t),
				mix(a.z, b.z, t),
				mix(a.w, b.w, t)
			};
		}

		TF_INLINE float4x4 transpose(const float4x4& a)
		{
			return float4x4{
				float4{ a.x.x, a.y.x, a.z.x, a.w.x },
				float4{ a.x.y, a.y.y, a.z.y, a.w.y },
				float4{ a.x.z, a.y.z, a.z.z, a.w.z },
				float4{ a.x.w, a.y.w, a.z.w, a.w.w }
			};
		}

		TF_INLINE float4x4 identity()
		{
			return float4x4{
				float4{ 1.0f, 0.0f, 0.0f, 0.0f },
				float4{ 0.0f, 1.0f, 0.0f, 0.0f },
				float4{ 0.0f, 0.0f, 1.0f, 0.0f },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}


#endif

		TF_INLINE float4x4 translate(const float3& t)
		{
			return float4x4{
				float4{ 1.0f, 0.0f, 0.0f, t.x },
				float4{ 0.0f, 1.0f, 0.0f, t.y },
				float4{ 0.0f, 0.0f, 1.0f, t.z },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 translate(float x, float y, float z)
		{
			return float4x4{
				float4{ 1.0f, 0.0f, 0.0f, x },
				float4{ 0.0f, 1.0f, 0.0f, y },
				float4{ 0.0f, 0.0f, 1.0f, z },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 rotate(const quat& q)
		{
			float a_sqr = q.w * q.w;
			float b_sqr = q.x * q.x;
			float c_sqr = q.y * q.y;
			float d_sqr = q.z * q.z;

			float a_b_2 = q.w * q.x * 2;
			float a_c_2 = q.w * q.y * 2;
			float a_d_2 = q.w * q.z * 2;

			float b_c_2 = q.x * q.y * 2;
			float b_d_2 = q.x * q.z * 2;

			float c_d_2 = q.y * q.z * 2;

			return float4x4{
				float4{ a_sqr + b_sqr - c_sqr - d_sqr, b_c_2 - a_d_2, a_c_2 + b_d_2, 0.0f },
				float4{ a_d_2 + b_c_2, a_sqr - b_sqr + c_sqr - d_sqr, c_d_2 - a_b_2, 0.0f },
				float4{ b_d_2 - a_c_2, a_b_2 + c_d_2, a_sqr - b_sqr - c_sqr + d_sqr, 0.0f },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 scale(const float3& s)
		{
			return float4x4{
				float4{ s.x, 0.0f, 0.0f, 0.0f },
				float4{ 0.0f, s.y, 0.0f, 0.0f },
				float4{ 0.0f, 0.0f, s.z, 0.0f },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 scale(float s)
		{
			return float4x4{
				float4{ s, 0.0f, 0.0f, 0.0f },
				float4{ 0.0f, s, 0.0f, 0.0f },
				float4{ 0.0f, 0.0f, s, 0.0f },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 scale(float x, float y, float z)
		{
			return float4x4{
				float4{ x, 0.0f, 0.0f, 0.0f },
				float4{ 0.0f, y, 0.0f, 0.0f },
				float4{ 0.0f, 0.0f, z, 0.0f },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		// apply in order of scale, rotation, translation
		TF_INLINE float4x4 transform(const float3& t, const quat& r, const float3& s)
		{

			float a_sqr = r.w * r.w;
			float b_sqr = r.x * r.x;
			float c_sqr = r.y * r.y;
			float d_sqr = r.z * r.z;

			float a_b_2 = r.w * r.x * 2;
			float a_c_2 = r.w * r.y * 2;
			float a_d_2 = r.w * r.z * 2;

			float b_c_2 = r.x * r.y * 2;
			float b_d_2 = r.x * r.z * 2;

			float c_d_2 = r.y * r.z * 2;

			return float4x4{
				float4{ s.x * (a_sqr + b_sqr - c_sqr - d_sqr), s.y * (b_c_2 - a_d_2), s.z * (a_c_2 + b_d_2), t.x },
				float4{ s.x * (a_d_2 + b_c_2), s.y * (a_sqr - b_sqr + c_sqr - d_sqr), s.z * (c_d_2 - a_b_2), t.y },
				float4{ s.x * (b_d_2 - a_c_2), s.y * (a_b_2 + c_d_2), s.z * (a_sqr - b_sqr - c_sqr + d_sqr), t.z },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 lookTo(const float3& position, const float3& direction, const float3& up)
		{
			float3 z = normalize(direction);
			float3 x = normalize(cross(normalize(up), z));
			float3 y = cross(z, x);
			return float4x4{
				float4{ x.x, x.y, x.z, -dot(x, position) },
				float4{ y.x, y.y, y.z, -dot(y, position) },
				float4{ z.x, z.y, z.z, -dot(z, position) },
				float4{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
		}

		TF_INLINE float4x4 lookAt(const float3& position, const float3& target, const float3& up)
		{
			return lookTo(position, target - position, up);
		}

		TF_INLINE float4x4 perspective(float fov, float aspect, float zNear, float zFar)
		{
			float yScale = 1.0f / std::tanf(fov * 0.5f);
			float xScale = yScale / aspect;
			float zScale = zFar / (zFar - zNear);
			float zOffset = zFar * zNear / (zNear - zFar);

			return float4x4{
				float4{ xScale, 0.0f, 0.0f, 0.0f },
				float4{ 0.0f, yScale, 0.0f, 0.0f },
				float4{ 0.0f, 0.0f, zScale, zOffset },
				float4{ 0.0f, 0.0f, 1.0, 0.0f }
			};
		}

	}
}