#include "../TofuMath.h"

#define GLM_FORCE_RADIANS 1
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace
{
	constexpr float global_err = 0.0001f;

	std::default_random_engine gen;

	bool equal(float a, float b)
	{
		return fabsf(a - b) <= global_err;
	}

	bool equal(float a, float b, float err)
	{
		return fabsf(a - b) <= err;
	}

	template<typename T1, typename T2>
	bool check_vec_equality(const T1& a, const T2& b)
	{
		constexpr size_t num_float = sizeof(T1) / sizeof(float);

		const float* arr_a = reinterpret_cast<const float*>(&a);
		const float* arr_b = reinterpret_cast<const float*>(&b);

		for (size_t i = 0; i < num_float; i++)
		{
			if (!equal(arr_a[i], arr_b[i]))
				return false;
		}

		return true;
	}

	template<typename T1, typename T2>
	bool check_vec_equality(const T1& a, const T2& b, float err)
	{
		constexpr size_t num_float = sizeof(T1) / sizeof(float);

		const float* arr_a = reinterpret_cast<const float*>(&a);
		const float* arr_b = reinterpret_cast<const float*>(&b);

		for (size_t i = 0; i < num_float; i++)
		{
			if (!equal(arr_a[i], arr_b[i], err))
				return false;
		}

		return true;
	}

	template<typename T1, typename T2>
	bool test_vec(float* a, float* b)
	{
		constexpr size_t CASE = 12;
		T1 c_ref[CASE];
		T2 c[CASE];

		// +=
		c_ref[0] = *reinterpret_cast<T1*>(a);
		c_ref[0] += *reinterpret_cast<T1*>(b);

		c[0] = *reinterpret_cast<T2*>(a);
		c[0] += *reinterpret_cast<T2*>(b);

		// -=
		c_ref[1] = *reinterpret_cast<T1*>(a);
		c_ref[1] -= *reinterpret_cast<T1*>(b);

		c[1] = *reinterpret_cast<T2*>(a);
		c[1] -= *reinterpret_cast<T2*>(b);

		// *=
		c_ref[2] = *reinterpret_cast<T1*>(a);
		c_ref[2] *= *reinterpret_cast<T1*>(b);

		c[2] = *reinterpret_cast<T2*>(a);
		c[2] *= *reinterpret_cast<T2*>(b);

		// *=
		c_ref[3] = *reinterpret_cast<T1*>(a);
		c_ref[3] *= b[0];

		c[3] = *reinterpret_cast<T2*>(a);
		c[3] *= b[0];

		// /=
		c_ref[4] = *reinterpret_cast<T1*>(a);
		c_ref[4] /= b[0];

		c[4] = *reinterpret_cast<T2*>(a);
		c[4] /= b[0];

		// +
		c_ref[5] = *reinterpret_cast<T1*>(a)
			+ *reinterpret_cast<T1*>(b);

		c[5] = *reinterpret_cast<T2*>(a)
			+ *reinterpret_cast<T2*>(b);

		// -
		c_ref[6] = *reinterpret_cast<T1*>(a)
			- *reinterpret_cast<T1*>(b);

		c[6] = *reinterpret_cast<T2*>(a)
			- *reinterpret_cast<T2*>(b);

		// - (unary)
		c_ref[7] = -(*reinterpret_cast<T1*>(a));

		c[7] = -(*reinterpret_cast<T2*>(a));

		// *
		c_ref[8] = *reinterpret_cast<T1*>(a)
			* *reinterpret_cast<T1*>(b);

		c[8] = *reinterpret_cast<T2*>(a)
			* *reinterpret_cast<T2*>(b);

		// *
		c_ref[9] = *reinterpret_cast<T1*>(a)
			* b[0];

		c[9] = *reinterpret_cast<T2*>(a)
			* b[0];

		// /
		c_ref[10] = *reinterpret_cast<T1*>(a)
			/ b[0];

		c[10] = *reinterpret_cast<T2*>(a)
			/ b[0];

		// normalize
		c_ref[11] = glm::normalize(*reinterpret_cast<T1*>(a));

		c[11] = tofu::math::normalize(*reinterpret_cast<T2*>(a));


		bool ret = true;
		
		for (int i = 0; i < CASE; i++)
		{
			ret = ret && check_vec_equality<T1, T2>(c_ref[i], c[i]);
		}

		return ret;
	}



}

int test_math()
{
	{
		std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
		std::uniform_real_distribution<float> dist1(-1.0f, 1.0f);
		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
		
		for (int i = 0; i < 100; i++)
		{
			float a[3] = { dist(gen), dist(gen), dist(gen) };
			float b[3] = { dist(gen), dist(gen), dist(gen) };

			if (!test_vec<glm::vec2, tofu::math::float2>(a, b)) return -1;

			if (!test_vec<glm::vec3, tofu::math::float3>(a, b)) return -1;

			if (!test_vec<glm::vec4, tofu::math::float4>(a, b)) return -1;

			if (!check_vec_equality(glm::cross(*reinterpret_cast<glm::vec3*>(a), *reinterpret_cast<glm::vec3*>(b)),
				tofu::math::cross(*reinterpret_cast<tofu::math::float3*>(a), *reinterpret_cast<tofu::math::float3*>(b))))
			{
				return __LINE__;
			}
		}


		for (int i = 0; i < 100; i++)
		{
			float pitch = dist(gen);
			float yaw = dist(gen);
			float roll = dist(gen);

			float tx = dist1(gen);
			float ty = dist1(gen);
			float tz = dist1(gen);

			float sx = dist1(gen);
			float sy = dist1(gen);
			float sz = dist1(gen);

			float vec[4] = { dist(gen), dist(gen), dist(gen), 1.0f };

			glm::quat q1 = glm::quat(glm::vec3(0.0f, yaw, 0.0f))
				* glm::quat(glm::vec3(pitch, 0.0f, 0.0f))
				* glm::quat(glm::vec3(0.0f, 0.0f, roll))
				;

			tofu::math::quat q2 = tofu::math::quat(pitch, yaw, roll);
			
			if (!check_vec_equality(q1, q2))
			{
				return __LINE__;
			}

			glm::vec3 r1 = q1 * *reinterpret_cast<glm::vec3*>(vec);
			tofu::math::float3 r2 = q2.rotate(*reinterpret_cast<tofu::math::float3*>(vec));
			
			if (!check_vec_equality(r1, r2, 0.001f))
			{ 
				return __LINE__;
			}

			glm::mat4 m1 = glm::transpose(glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz))
				* glm::toMat4(q1) * glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz)));
			
			tofu::math::float4x4 m2 = tofu::math::matrix::transform(
				tofu::math::float3{ tx, ty, tz },
				q2,
				tofu::math::float3{ sx, sy, sz }
			);

			if (!check_vec_equality(m1, m2))
			{
				return __LINE__;
			}

			r1 = *reinterpret_cast<glm::vec4*>(vec) * m1;

			tofu::math::float4 t = m2 * *reinterpret_cast<tofu::math::float4*>(vec);
			r2 = tofu::math::float3{ t.x, t.y, t.z };

			if (!check_vec_equality(r1, r2, 0.001f))
			{
				return __LINE__;
			}

		}

		for (uint32_t i = 0; i < 100; i++)
		{
			float axii[3] = { dist1(gen), dist1(gen), dist1(gen) };
			float theta = dist(gen);

			glm::quat q1 = glm::angleAxis(
				theta, 
				glm::normalize(*reinterpret_cast<glm::vec3*>(axii))
			);

			tofu::math::quat q2(
				theta,
				tofu::math::normalize(*reinterpret_cast<tofu::math::float3*>(axii))
			);

			if (!check_vec_equality(q1, q2))
			{
				return __LINE__;
			}

			q1 = glm::angleAxis(
				tofu::math::PI,
				glm::normalize(*reinterpret_cast<glm::vec3*>(axii))
			);

			q2 = tofu::math::quat(
				tofu::math::PI,
				tofu::math::normalize(*reinterpret_cast<tofu::math::float3*>(axii))
			);

			if (!check_vec_equality(q1, q2))
			{
				return __LINE__;
			}

			q1 = glm::angleAxis(
				0.0f,
				glm::normalize(*reinterpret_cast<glm::vec3*>(axii))
			);

			q2 = tofu::math::quat(
				0.0f,
				tofu::math::normalize(*reinterpret_cast<tofu::math::float3*>(axii))
			);

			if (!check_vec_equality(q1, q2))
			{
				return __LINE__;
			}
		}

		for (uint32_t i = 0; i < 100; i++)
		{
			float pitch1 = dist(gen);
			float yaw1 = dist(gen);
			float roll1 = dist(gen);

			float pitch2 = dist(gen);
			float yaw2 = dist(gen);
			float roll2 = dist(gen);

			float t = dist01(gen);

			glm::quat a1 = glm::quat(glm::vec3(0.0f, yaw1, 0.0f))
				* glm::quat(glm::vec3(pitch1, 0.0f, 0.0f))
				* glm::quat(glm::vec3(0.0f, 0.0f, roll1));
			glm::quat b1 = glm::quat(glm::vec3(0.0f, yaw2, 0.0f))
				* glm::quat(glm::vec3(pitch2, 0.0f, 0.0f))
				* glm::quat(glm::vec3(0.0f, 0.0f, roll2));

			tofu::math::quat a2(pitch1, yaw1, roll1);
			tofu::math::quat b2(pitch2, yaw2, roll2);

			glm::quat c1 = glm::slerp(a1, b1, t);
			tofu::math::quat c2 = tofu::math::slerp(a2, b2, t);
			
			if (!check_vec_equality(c1, c2))
			{
				return __LINE__;
			}

			c1 = glm::slerp(a1, a1, 0.5f);
			c2 = tofu::math::slerp(a2, a2, 0.5f);

			if (!check_vec_equality(c1, c2))
			{
				return __LINE__;
			}
		}
	}

	return 0;
}
