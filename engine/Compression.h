#pragma once

#include <stdint.h>
#include "TofuMath.h"
#include <cmath>
#include <math.h>

// credit to https://github.com/ThisIsRobokitty/netgame/blob/60ac89873a53b02611a88b0d5b10987d4693a9ad/XX%20-%20Fiedler's%20Cubes/Engine.h

namespace tofu {
	namespace compression {

		TF_INLINE void CompressQuaternion(const math::quat & orientation, uint32_t & compressed_orientation)
		{
			uint32_t largest = 0;
			float a, b, c;
			a = 0;
			b = 0;
			c = 0;

			const float w = orientation.w;
			const float x = orientation.x;
			const float y = orientation.y;
			const float z = orientation.z;

#ifdef DEBUG
			const float epsilon = 0.0001f;
			const float length_squared = w*w + x*x + y*y + z*z;
			assert(length_squared >= 1.0f - epsilon && length_squared <= 1.0f + epsilon);
#endif

			const float abs_w = abs(w);
			const float abs_x = abs(x);
			const float abs_y = abs(y);
			const float abs_z = abs(z);

			float largest_value = abs_x;

			if (abs_y > largest_value)
			{
				largest = 1;
				largest_value = abs_y;
			}

			if (abs_z > largest_value)
			{
				largest = 2;
				largest_value = abs_z;
			}

			if (abs_w > largest_value)
			{
				largest = 3;
				largest_value = abs_w;
			}

			switch (largest)
			{
			case 0:
				if (x >= 0)
				{
					a = y;
					b = z;
					c = w;
				}
				else
				{
					a = -y;
					b = -z;
					c = -w;
				}
				break;

			case 1:
				if (y >= 0)
				{
					a = x;
					b = z;
					c = w;
				}
				else
				{
					a = -x;
					b = -z;
					c = -w;
				}
				break;

			case 2:
				if (z >= 0)
				{
					a = x;
					b = y;
					c = w;
				}
				else
				{
					a = -x;
					b = -y;
					c = -w;
				}
				break;

			case 3:
				if (w >= 0)
				{
					a = x;
					b = y;
					c = z;
				}
				else
				{
					a = -x;
					b = -y;
					c = -z;
				}
				break;
			}
			//		printf( "float: a = %f, b = %f, c = %f\n", a, b, c );

			const float minimum = -1.0f / 1.414214f;		// note: 1.0f / sqrt(2)
			const float maximum = +1.0f / 1.414214f;

			const float normal_a = (a - minimum) / (maximum - minimum);
			const float normal_b = (b - minimum) / (maximum - minimum);
			const float normal_c = (c - minimum) / (maximum - minimum);

			uint32_t integer_a = static_cast<uint32_t>(floor(normal_a * 1024.0f + 0.5f));
			uint32_t integer_b = static_cast<uint32_t>(floor(normal_b * 1024.0f + 0.5f));
			uint32_t integer_c = static_cast<uint32_t>(floor(normal_c * 1024.0f + 0.5f));

			//		printf( "integer: a = %d, b = %d, c = %d, largest = %d\n", 
			//			integer_a, integer_b, integer_c, largest );

			compressed_orientation = (largest << 30) | (integer_a << 20) | (integer_b << 10) | integer_c;
		}

		TF_INLINE void DecompressQuaternion(uint32_t compressed_orientation, math::quat & orientation)
		{
			uint32_t largest = compressed_orientation >> 30;
			uint32_t integer_a = (compressed_orientation >> 20) & ((1 << 10) - 1);
			uint32_t integer_b = (compressed_orientation >> 10) & ((1 << 10) - 1);
			uint32_t integer_c = (compressed_orientation) & ((1 << 10) - 1);

			//		printf( "---------\n" );

			//		printf( "integer: a = %d, b = %d, c = %d, largest = %d\n", 
			//			integer_a, integer_b, integer_c, largest );

			const float minimum = -1.0f / 1.414214f;		// note: 1.0f / sqrt(2)
			const float maximum = +1.0f / 1.414214f;

			const float a = integer_a / 1024.0f * (maximum - minimum) + minimum;
			const float b = integer_b / 1024.0f * (maximum - minimum) + minimum;
			const float c = integer_c / 1024.0f * (maximum - minimum) + minimum;

			//		printf( "float: a = %f, b = %f, c = %f\n", a, b, c );

			switch (largest)
			{
			case 0:
			{
				// (?) y z w

				orientation.y = a;
				orientation.z = b;
				orientation.w = c;
				orientation.x = sqrt(1 - orientation.y*orientation.y
					- orientation.z*orientation.z
					- orientation.w*orientation.w);
			}
			break;

			case 1:
			{
				// x (?) z w

				orientation.x = a;
				orientation.z = b;
				orientation.w = c;
				orientation.y = sqrt(1 - orientation.x*orientation.x
					- orientation.z*orientation.z
					- orientation.w*orientation.w);
			}
			break;

			case 2:
			{
				// x y (?) w

				orientation.x = a;
				orientation.y = b;
				orientation.w = c;
				orientation.z = sqrt(1 - orientation.x*orientation.x
					- orientation.y*orientation.y
					- orientation.w*orientation.w);
			}
			break;

			case 3:
			{
				// x y z (?)

				orientation.x = a;
				orientation.y = b;
				orientation.z = c;
				orientation.w = sqrt(1 - orientation.x*orientation.x
					- orientation.y*orientation.y
					- orientation.z*orientation.z);
			}
			break;
			}

			orientation = math::normalize(orientation);
		}

		void CompressQuaternion(const math::quat &quat, math::float3& float3, bool &negativeW) {
			float3.x = quat.x;
			float3.y = quat.y;
			float3.z = quat.z;

			if (quat.w < 0) {
				negativeW = true;
			}
		}

		void DecompressQuaternion(math::quat &quat, const math::float3& float3, const bool &negativeW) {
			quat.x = float3.x;
			quat.y = float3.y;
			quat.z = float3.z;
			quat.w = sqrt(1 - quat.x * quat.x - quat.y * quat.y - quat.z * quat.z);

			if (negativeW) {
				quat.w = -quat.w;
			}
		}
	}
}
