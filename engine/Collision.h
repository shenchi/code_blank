#pragma once

#include "TofuMath.h"
#include "Transform.h"

// algorithms are mostly ported from DirectXMath
// https://github.com/Microsoft/DirectXMath


namespace tofu
{
	struct BoundingSphere;
	struct BoundingBox;
	struct OrientedBoundingBox;
	struct Frustum;

	struct BoundingSphere
	{
		math::float3		center;
		float				radius;

		TF_INLINE BoundingSphere(const math::float3& c, float r) : center(c), radius(r) {}

		void Transform(const Transform& t);

		bool Intersects(const BoundingSphere& sphere) const;
		bool Intersects(const BoundingBox& aabb) const;
		bool Intersects(const OrientedBoundingBox& obb) const;
		bool Intersects(const Frustum& frustum) const;
	};

	struct BoundingBox
	{
		math::float3		center;
		math::float3		extends;

		TF_INLINE BoundingBox(const math::float3& c, const math::float3& e) : center(c), extends(e) {}

		void Transform(const Transform& t);

		bool Intersects(const BoundingSphere& sphere) const;
		bool Intersects(const BoundingBox& aabb) const;
		bool Intersects(const OrientedBoundingBox& obb) const;
		bool Intersects(const Frustum& frustum) const;
	};

	struct OrientedBoundingBox
	{
		math::float3		center;
		math::float3		extends;
		math::quat			orientation;

		TF_INLINE OrientedBoundingBox(const math::float3& c, const math::float3& e, const math::quat& o)
			: center(c), extends(e), orientation(o) {}

		void Transform(const Transform& t);

		bool Intersects(const BoundingSphere& sphere) const;
		bool Intersects(const BoundingBox& aabb) const;
		bool Intersects(const OrientedBoundingBox& obb) const;
		bool Intersects(const Frustum& frustum) const;
	};

	struct Frustum
	{
		math::float3		origin;
		math::quat			orientation;

		float				rightSlope; // x/z
		float				leftSlope;
		float				topSlope; // y/z
		float				bottomSlope;
		float				near;
		float				far;

		TF_INLINE Frustum(const math::float3& orig, const math::quat& orient, float r, float l, float t, float b, float n, float f)
			: origin(orig), orientation(orient), rightSlope(r), leftSlope(l), topSlope(t), bottomSlope(b), near(n), far(f) {}

		void Transform(const Transform& t);

		bool Intersects(const BoundingSphere& sphere) const;
		bool Intersects(const BoundingBox& aabb) const;
		bool Intersects(const OrientedBoundingBox& obb) const;
		bool Intersects(const Frustum& frustum) const;
	};


	TF_INLINE void BoundingSphere::Transform(const tofu::Transform& t)
	{

	}

	TF_INLINE bool BoundingSphere::Intersects(const BoundingSphere& sphere) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool BoundingSphere::Intersects(const BoundingBox& aabb) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool BoundingSphere::Intersects(const OrientedBoundingBox& obb) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool BoundingSphere::Intersects(const Frustum& frustum) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE void BoundingBox::Transform(const tofu::Transform& t)
	{
		math::float3 corner = t.TransformPosition(center + extends);
		math::float3 minC = corner, maxC = corner;

		for (int32_t i = 0; i < 7; i++)
		{
			int32_t x = ((i & 1) << 1) - 1;
			int32_t y = ((i & 2)) - 1;
			int32_t z = ((i & 4) >> 1) - 1;

			corner = t.TransformPosition(center +
				extends * math::float3(float(x), float(y), float(z)));

			minC = math::min(minC, corner);
			maxC = math::max(maxC, corner);
		}

		center = (maxC + minC) * 0.5f;
		extends = (maxC - minC) * 0.5f;
	}

	TF_INLINE bool BoundingBox::Intersects(const BoundingSphere& sphere) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool BoundingBox::Intersects(const BoundingBox& aabb) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool BoundingBox::Intersects(const OrientedBoundingBox& obb) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool BoundingBox::Intersects(const Frustum& frustum) const
	{
		return frustum.Intersects(*this);
	}

	TF_INLINE void OrientedBoundingBox::Transform(const tofu::Transform& t)
	{
		orientation = t.GetRotation() * orientation;
		center = t.TransformPosition(center);
		extends *= t.GetScale();
	}

	TF_INLINE bool OrientedBoundingBox::Intersects(const BoundingSphere& sphere) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool OrientedBoundingBox::Intersects(const BoundingBox& aabb) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool OrientedBoundingBox::Intersects(const OrientedBoundingBox& obb) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool OrientedBoundingBox::Intersects(const Frustum& frustum) const
	{
		return frustum.Intersects(*this);
	}

	TF_INLINE void Frustum::Transform(const tofu::Transform& t)
	{
		orientation = t.GetRotation() * orientation;
		origin = t.TransformPosition(origin);
		float scale = scale = t.GetScale().z;
		near *= scale;
		far *= scale;
	}

	TF_INLINE bool Frustum::Intersects(const BoundingSphere& sphere) const
	{
		assert(false && "this function unimplemented yet.");
		return false;
	}

	TF_INLINE bool Frustum::Intersects(const BoundingBox& aabb) const
	{
		OrientedBoundingBox obb(aabb.center, aabb.extends, math::quat());
		return Intersects(obb);
	}

	TF_INLINE bool Frustum::Intersects(const OrientedBoundingBox& obb) const
	{
		// construct planes
		math::float3 axis[6] =
		{
			{ 0.0f, 0.0f, -1.0f },  // near plane
			{ 0.0f, 0.0f, 1.0f },  // far plane
			{ 1.0f, 0.0f, -rightSlope }, // right plane
			{ -1.0f, 0.0f, leftSlope }, // left plane
			{ 0.0f, 1.0f, -topSlope }, // top plane
			{ 0.0f, -1.0f, bottomSlope }, // bottom plane
		};

		float dists[6] = { -near, far, 0, 0, 0, 0 };

		math::float3 center = obb.center;
		math::float3 extends = obb.extends;
		math::quat boxOrient = obb.orientation;

		math::quat invOrient = math::inverse(orientation);
		center = invOrient * (center - origin);
		boxOrient = invOrient * boxOrient;

		math::float3x3 R = math::mat3_cast(boxOrient);

		bool outside = false;
		bool insideAll = true;
		bool centerInsideAll = true;

		for (size_t i = 0; i < 6; ++i)
		{
			float dist = math::dot(center, axis[i]) - dists[i];
			math::float3 radius{
				math::dot(R[0], axis[i]),
				math::dot(R[1], axis[i]),
				math::dot(R[2], axis[i])
			};

			float projRadius = math::dot(math::abs(radius), extends);

			outside |= (dist > projRadius);
			insideAll &= (dist <= -projRadius);
			centerInsideAll &= (dist <= 0);
		}

		if (outside) return false;

		if (insideAll) return true;

		if (centerInsideAll) return true;

		// construct corners of frustum
		math::float3 rightTopA{ rightSlope, topSlope, 1.0f };
		math::float3 rightBottomA{ rightSlope, bottomSlope, 1.0f };
		math::float3 leftTopA{ leftSlope, topSlope, 1.0f };
		math::float3 leftBottomA{ leftSlope, bottomSlope, 1.0f };
		math::float3 nearA{ near, near, near };
		math::float3 farA{ far, far, far };

		math::float3 corners[8] =
		{
			rightTopA * nearA,		// near right top
			rightBottomA * nearA,		// near right bottom
			leftTopA * nearA,			// near left top
			leftBottomA * nearA,		// near left bottom
			rightTopA * farA,			// far right top
			rightBottomA * farA,		// far right bottom
			leftTopA * farA,			// far left top
			leftBottomA * farA,		// far left bottom
		};

		// project frustum to box axes
		{
			math::float3 minC{
				math::dot(corners[0], R[0]),
				math::dot(corners[0], R[1]),
				math::dot(corners[0], R[2])
			};
			math::float3 maxC = minC;

			for (size_t i = 1; i < 8; ++i)
			{
				math::float3 temp
				{
					math::dot(corners[i], R[0]),
					math::dot(corners[i], R[1]),
					math::dot(corners[i], R[2])
				};

				minC = math::min(minC, temp);
				maxC = math::max(maxC, temp);
			}

			math::float3 boxDist{
				math::dot(center, R[0]),
				math::dot(center, R[1]),
				math::dot(center, R[2])
			};

			math::bool3 r = math::greaterThan(minC, boxDist + extends) ||
				math::lessThan(maxC, boxDist - extends);

			if (r.x || r.y || r.z) return false;
		}

		// construct edge axis of frustum
		math::float3 edgeAxes[6] =
		{
			rightTopA,
			rightBottomA,
			leftTopA,
			leftBottomA,
			rightTopA - leftTopA,
			leftBottomA - leftTopA,
		};

		for (size_t i = 0; i < 3; ++i)
		{
			for (size_t j = 0; j < 6; ++j)
			{
				math::float3 axis = math::cross(R[i], edgeAxes[j]);

				float minD = math::dot(axis, corners[0]);
				float maxD = minD;

				for (size_t k = 1; k < 8; ++k)
				{
					float temp = math::dot(axis, corners[k]);
					minD = math::min(minD, temp);
					maxD = math::max(maxD, temp);
				}

				float dist = math::dot(center, axis);

				math::float3 radius{
					math::dot(R[0], axis),
					math::dot(R[1], axis),
					math::dot(R[2], axis)
				};

				float projRadius = math::dot(math::abs(radius), extends);

				outside |= ((dist > maxD + projRadius) || (dist < minD - projRadius));
			}
		}

		if (outside) return false;

		return true;
	}

	TF_INLINE bool Frustum::Intersects(const Frustum& frustum) const
	{
		math::float3 originA = origin;
		math::quat orientationA = orientation;

		// construct planes of A
		math::float3 axisA[6] =
		{
			{ 0.0f, 0.0f, -1.0f },  // near plane
			{ 0.0f, 0.0f, 1.0f },  // far plane
			{ 1.0f, 0.0f, -rightSlope }, // right plane
			{ -1.0f, 0.0f, leftSlope }, // left plane
			{ 0.0f, 1.0f, -topSlope }, // top plane
			{ 0.0f, -1.0f, bottomSlope }, // bottom plane
		};

		float distA[6] = { -near, far, 0, 0, 0, 0 };

		math::float3 originB = frustum.origin;
		math::quat orientationB = frustum.orientation;

		// convert B into A's local space
		math::quat invOrientA = math::inverse(orientationA);
		originB = invOrientA * (originB - originA);
		orientationB = invOrientA * orientationB;

		// construct corners of B
		math::float3 rightTopB{ frustum.rightSlope, frustum.topSlope, 1.0f };
		math::float3 rightBottomB{ frustum.rightSlope, frustum.bottomSlope, 1.0f };
		math::float3 leftTopB{ frustum.leftSlope, frustum.topSlope, 1.0f };
		math::float3 leftBottomB{ frustum.leftSlope, frustum.bottomSlope, 1.0f };
		math::float3 nearB{ frustum.near, frustum.near, frustum.near };
		math::float3 farB{ frustum.far, frustum.far, frustum.far };

		rightTopB = orientationB * rightTopB;
		rightBottomB = orientationB * rightBottomB;
		leftTopB = orientationB * leftTopB;
		leftBottomB = orientationB * leftBottomB;

		math::float3 cornersB[8] =
		{
			originB + rightTopB * nearB,		// near right top
			originB + rightBottomB * nearB,		// near right bottom
			originB + leftTopB * nearB,			// near left top
			originB + leftBottomB * nearB,		// near left bottom
			originB + rightTopB * farB,			// far right top
			originB + rightBottomB * farB,		// far right bottom
			originB + leftTopB * farB,			// far left top
			originB + leftBottomB * farB,		// far left bottom
		};

		bool outside = false;
		bool insideAll = true;

		// test B's corners against each of A's planes
		for (size_t i = 0; i < 6; ++i)
		{
			float minD = math::dot(axisA[i], cornersB[0]);
			float maxD = minD;
			for (size_t j = 1; j < 8; ++j)
			{
				float dist = math::dot(axisA[i], cornersB[j]);
				minD = math::min(minD, dist);
				maxD = math::max(maxD, dist);
			}

			outside |= (minD > distA[i]);
			insideAll &= (maxD <= distA[i]);
		}

		// if B is outside any of A's planes
		if (outside) return false;

		// if B is fully inside A
		if (insideAll) return true;

		// construct planes of B
		math::float3 axisB[6] =
		{
			{ 0.0f, 0.0f, -1.0f },  // near plane
			{ 0.0f, 0.0f, 1.0f },  // far plane
			{ 1.0f, 0.0f, -frustum.rightSlope }, // right plane
			{ -1.0f, 0.0f, frustum.leftSlope }, // left plane
			{ 0.0f, 1.0f, -frustum.topSlope }, // top plane
			{ 0.0f, -1.0f, frustum.bottomSlope }, // bottom plane
		};

		axisB[0] = orientationB * axisB[0];
		axisB[1] = orientationB * axisB[1];
		axisB[2] = orientationB * axisB[2];
		axisB[3] = orientationB * axisB[3];
		axisB[4] = orientationB * axisB[4];
		axisB[5] = orientationB * axisB[5];

		float distB[6] = {
			math::dot(axisB[0], cornersB[0]),
			math::dot(axisB[1], cornersB[4]),
			math::dot(axisB[2], originB),
			math::dot(axisB[3], originB),
			math::dot(axisB[4], originB),
			math::dot(axisB[5], originB),
		};

		// construct corners of A
		math::float3 rightTopA{ rightSlope, topSlope, 1.0f };
		math::float3 rightBottomA{ rightSlope, bottomSlope, 1.0f };
		math::float3 leftTopA{ leftSlope, topSlope, 1.0f };
		math::float3 leftBottomA{ leftSlope, bottomSlope, 1.0f };
		math::float3 nearA{ near, near, near };
		math::float3 farA{ far, far, far };

		math::float3 cornersA[8] =
		{
			rightTopA * nearA,		// near right top
			rightBottomA * nearA,		// near right bottom
			leftTopA * nearA,			// near left top
			leftBottomA * nearA,		// near left bottom
			rightTopA * farA,			// far right top
			rightBottomA * farA,		// far right bottom
			leftTopA * farA,			// far left top
			leftBottomA * farA,		// far left bottom
		};

		// test A's corners against each of B's planes
		for (size_t i = 0; i < 6; ++i)
		{
			float minD = math::dot(axisB[i], cornersA[0]);

			for (size_t j = 1; j < 8; ++j)
			{
				float dist = math::dot(axisB[i], cornersA[j]);
				minD = math::min(minD, dist);
			}

			outside |= (minD > distA[i]);
		}

		// if A is outside any of B's planes
		if (outside) return false;

		// construct edge axis of A and B
		math::float3 edgeAxisA[6] =
		{
			rightTopA,
			rightBottomA,
			leftTopA,
			leftBottomA,
			rightTopA - leftTopA,
			leftBottomA - leftTopA,
		};

		math::float3 edgeAxisB[6] =
		{
			rightTopB,
			rightBottomB,
			leftTopB,
			leftBottomB,
			rightTopB - leftTopB,
			leftBottomB - leftTopB,
		};

		// find separating plane for each edge pair
		for (size_t i = 0; i < 6; ++i)
		{
			for (size_t j = 0; j < 6; ++j)
			{
				math::float3 axis = math::cross(edgeAxisA[i], edgeAxisB[j]);

				float minA = math::dot(axis, cornersA[0]);
				float maxA = minA;
				float minB = math::dot(axis, cornersB[0]);
				float maxB = minB;

				for (size_t k = 0; k < 8; ++k)
				{
					float distA = math::dot(axis, cornersA[k]);
					minA = math::min(minA, distA);
					maxA = math::max(maxA, distA);

					float distB = math::dot(axis, cornersB[k]);
					minB = math::min(minB, distB);
					maxB = math::max(maxB, distB);
				}

				outside |= ((minA > maxB) || (minB > maxA));
			}
		}

		// there is a separating plane
		if (outside) return false;

		return true;
	}
}
