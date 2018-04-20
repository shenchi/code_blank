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
		TF_INLINE BoundingSphere() : center(), radius(1.0f) {}

		TF_INLINE void Transform(const Transform& t);

		TF_INLINE bool Intersects(const BoundingSphere& sphere) const;
		TF_INLINE bool Intersects(const BoundingBox& aabb) const;
		TF_INLINE bool Intersects(const OrientedBoundingBox& obb) const;
		TF_INLINE bool Intersects(const Frustum& frustum) const;
	};

	struct BoundingBox
	{
		math::float3		center;
		math::float3		extends;

		TF_INLINE BoundingBox(const math::float3& c, const math::float3& e) : center(c), extends(e) {}
		TF_INLINE BoundingBox() : center(), extends(0.5f) {}

		TF_INLINE void Transform(const Transform& t);

		TF_INLINE bool Intersects(const BoundingSphere& sphere) const;
		TF_INLINE bool Intersects(const BoundingBox& aabb) const;
		TF_INLINE bool Intersects(const OrientedBoundingBox& obb) const;
		TF_INLINE bool Intersects(const Frustum& frustum) const;
	};

	struct OrientedBoundingBox
	{
		math::float3		center;
		math::float3		extends;
		math::quat			orientation;

		TF_INLINE OrientedBoundingBox(const math::float3& c, const math::float3& e, const math::quat& o)
			: center(c), extends(e), orientation(o) {}
		TF_INLINE OrientedBoundingBox() : center(), extends(0.5f), orientation() {}
		
		TF_INLINE void Transform(const Transform& t);

		TF_INLINE bool Intersects(const BoundingSphere& sphere) const;
		TF_INLINE bool Intersects(const BoundingBox& aabb) const;
		TF_INLINE bool Intersects(const OrientedBoundingBox& obb) const;
		TF_INLINE bool Intersects(const Frustum& frustum) const;
	};

	struct Frustum
	{
		math::float3		origin;
		math::quat			orientation;

		float				rightSlope; // x/z
		float				leftSlope;
		float				topSlope; // y/z
		float				bottomSlope;
		float				nearPlane;
		float				farPlane;

		TF_INLINE Frustum(const math::float3& orig, const math::quat& orient, float r, float l, float t, float b, float n, float f)
			: origin(orig), orientation(orient), rightSlope(r), leftSlope(l), topSlope(t), bottomSlope(b), nearPlane(n), farPlane(f) {}

		TF_INLINE Frustum() : origin(), orientation(), rightSlope(1), leftSlope(-1), topSlope(1), bottomSlope(-1), nearPlane(0), farPlane(1) {}

		TF_INLINE void Transform(const Transform& t);
		TF_INLINE void TransformWithoutScaling(const tofu::Transform& t);
		 
		TF_INLINE bool Intersects(const BoundingSphere& sphere) const;
		TF_INLINE bool Intersects(const BoundingBox& aabb) const;
		TF_INLINE bool Intersects(const OrientedBoundingBox& obb) const;
		TF_INLINE bool Intersects(const Frustum& frustum) const;
	};


	TF_INLINE math::float3 NearestPointOnLineSegment(const math::float3& s1, const math::float3& s2, const math::float3& p)
	{
		math::float3 dir = s2 - s1;
		float proj = math::dot(p, dir) - math::dot(s1, dir);
		float lengthSq = math::dot(dir, dir);

		float t = proj / lengthSq;
		math::float3 point = t * dir + s1;

		int selectS1 = int(proj < 0);
		int selectS2 = int(proj > lengthSq);

		point =
		{
			selectS1 * s1.x + (1 - selectS1) * point.x,
			selectS1 * s1.y + (1 - selectS1) * point.y,
			selectS1 * s1.z + (1 - selectS1) * point.z
		};
			
		point =
		{
			selectS2 * s2.x + (1 - selectS2) * point.x,
			selectS2 * s2.y + (1 - selectS2) * point.y,
			selectS2 * s2.z + (1 - selectS2) * point.z
		};

		return point;
	}

	TF_INLINE void BoundingSphere::Transform(const tofu::Transform& t)
	{
		center = t.TransformPosition(center);
	}

	TF_INLINE bool BoundingSphere::Intersects(const BoundingSphere& sphere) const
	{
		math::float3 dist = sphere.center - center;
		float dist_Sqr = math::dot(dist, dist);
		float radiusSum = radius + sphere.radius;
		return dist_Sqr <= radiusSum * radiusSum;
	}

	TF_INLINE bool BoundingSphere::Intersects(const BoundingBox& aabb) const
	{
		return aabb.Intersects(*this);
	}

	TF_INLINE bool BoundingSphere::Intersects(const OrientedBoundingBox& obb) const
	{
		return obb.Intersects(*this);
	}

	TF_INLINE bool BoundingSphere::Intersects(const Frustum& frustum) const
	{
		return frustum.Intersects(*this);
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
		math::float3 boxMin = center - extends;
		math::float3 boxMax = center + extends;

		math::int3 less = math::lessThan(sphere.center, boxMin);
		math::int3 greater = math::greaterThan(sphere.center, boxMax);
		math::float3 minDelta = sphere.center - boxMin;
		math::float3 maxDelta = sphere.center - boxMax;

		math::float3 d{
			less.x * minDelta.x,
			less.y * minDelta.y,
			less.z * minDelta.z,
		};

		d = math::float3{
			greater.x * maxDelta.x + (1 - greater.x) * d.x,
			greater.y * maxDelta.y + (1 - greater.y) * d.y,
			greater.z * maxDelta.z + (1 - greater.z) * d.z,
		};

		float dist = math::dot(d, d);

		return dist <= sphere.radius * sphere.radius;
	}

	TF_INLINE bool BoundingBox::Intersects(const BoundingBox& aabb) const
	{
		math::float3 minA = center - extends;
		math::float3 maxA = center + extends;

		math::float3 minB = aabb.center - aabb.extends;
		math::float3 maxB = aabb.center + aabb.extends;

		math::bool3 result = math::greaterThan(minA, maxB) ||
			math::greaterThan(minB, maxA);

		return result.x || result.y || result.z;
	}

	TF_INLINE bool BoundingBox::Intersects(const OrientedBoundingBox& obb) const
	{
		return obb.Intersects(*this);
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
		math::float3 sphereCenter = math::inverse(orientation) * (sphere.center - center);

		math::int3 less = math::lessThan(sphereCenter, -extends);
		math::int3 greater = math::greaterThan(sphereCenter, extends);
		math::float3 minDelta = sphereCenter + extends;
		math::float3 maxDelta = sphereCenter - extends;

		math::float3 d{
			less.x * minDelta.x,
			less.y * minDelta.y,
			less.z * minDelta.z,
		};

		d = math::float3{
			greater.x * maxDelta.x + (1 - greater.x) * d.x,
			greater.y * maxDelta.y + (1 - greater.y) * d.y,
			greater.z * maxDelta.z + (1 - greater.z) * d.z,
		};

		float dist = math::dot(d, d);

		return dist <= sphere.radius * sphere.radius;
	}

	TF_INLINE bool OrientedBoundingBox::Intersects(const BoundingBox& aabb) const
	{
		OrientedBoundingBox obb(aabb.center, aabb.extends, math::quat());
		return Intersects(obb);
	}

	TF_INLINE bool OrientedBoundingBox::Intersects(const OrientedBoundingBox& obb) const
	{
		math::float3 centerA = center;
		math::quat orientA = orientation;

		math::quat invOrientA = math::inverse(orientA);
		math::quat orientB = obb.orientation;
		math::float3 centerB = invOrientA * (obb.center - centerA);

		math::quat Q = math::inverse(orientB) * orientA;
		math::float3x3 R = math::mat3_cast(Q);

		math::float3 extendsA = extends;
		math::float3 extendsB = obb.extends;

		math::float3 R0X = R[0];
		math::float3 R1X = R[1];
		math::float3 R2X = R[2];

		R = math::transpose(R);

		math::float3 RX0 = R[0];
		math::float3 RX1 = R[1];
		math::float3 RX2 = R[2];

		math::float3 AR0X = math::abs(R0X);
		math::float3 AR1X = math::abs(R1X);
		math::float3 AR2X = math::abs(R2X);

		math::float3 ARX0 = math::abs(RX0);
		math::float3 ARX1 = math::abs(RX1);
		math::float3 ARX2 = math::abs(RX2);

		float d, dA, dB;

		// B against A's axes

		d = centerB.x;
		dA = extendsA.x;
		dB = math::dot(extendsB, AR0X);

		bool noIntersection = (math::abs(d) > (dA + dB));

		d = centerB.y;
		dA = extendsA.y;
		dB = math::dot(extendsB, AR1X);

		noIntersection |= (math::abs(d) > (dA + dB));

		d = centerB.z;
		dA = extendsA.z;
		dB = math::dot(extendsB, AR2X);

		noIntersection |= (math::abs(d) > (dA + dB));

		// A against B's axes

		d = math::dot(centerB, RX0);
		dA = math::dot(extendsA, ARX0);
		dB = extendsB.x;

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, RX1);
		dA = math::dot(extendsA, ARX1);
		dB = extendsB.y;

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, RX2);
		dA = math::dot(extendsA, ARX2);
		dB = extendsB.z;

		noIntersection |= (math::abs(d) > (dA + dB));

		// cross or each pair of axes

		d = math::dot(centerB, math::float3{0, -RX0.z, RX0.y});
		dA = math::dot(extendsA, math::float3{ 0, ARX0.z, ARX0.y });
		dB = math::dot(extendsB, math::float3{ 0, AR0X.z, AR0X.y });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ 0, -RX1.z, RX1.y });
		dA = math::dot(extendsA, math::float3{ 0, ARX1.z, ARX1.y });
		dB = math::dot(extendsB, math::float3{ AR0X.z, 0, AR0X.x });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ 0, -RX2.z, RX2.y });
		dA = math::dot(extendsA, math::float3{ 0, ARX2.z, ARX2.y });
		dB = math::dot(extendsB, math::float3{ AR0X.y, AR0X.x, 0 });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ RX0.z, 0, -RX0.x });
		dA = math::dot(extendsA, math::float3{ ARX0.z, 0, ARX0.x });
		dB = math::dot(extendsB, math::float3{ 0, AR1X.z, AR1X.y });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ RX1.z, 0, -RX1.x });
		dA = math::dot(extendsA, math::float3{ ARX1.z, 0, ARX1.x });
		dB = math::dot(extendsB, math::float3{ AR1X.z, 0, AR1X.x });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ RX2.z, 0, -RX2.x });
		dA = math::dot(extendsA, math::float3{ ARX2.z, 0, ARX2.x });
		dB = math::dot(extendsB, math::float3{ AR1X.y, AR1X.x, 0 });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ -RX0.y, RX0.x, 0 });
		dA = math::dot(extendsA, math::float3{ ARX0.y, ARX0.x, 0 });
		dB = math::dot(extendsB, math::float3{ 0, AR2X.z, AR2X.y });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ -RX1.y, RX1.x, 0 });
		dA = math::dot(extendsA, math::float3{ ARX1.y, ARX1.x, 0 });
		dB = math::dot(extendsB, math::float3{ AR2X.z, 0, AR2X.x });

		noIntersection |= (math::abs(d) > (dA + dB));

		d = math::dot(centerB, math::float3{ -RX2.y, RX2.x, 0 });
		dA = math::dot(extendsA, math::float3{ ARX2.y, ARX2.x, 0 });
		dB = math::dot(extendsB, math::float3{ AR2X.y, AR2X.x, 0 });

		noIntersection |= (math::abs(d) > (dA + dB));

		return !noIntersection;
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
		nearPlane *= scale;
		farPlane *= scale;
	}

	TF_INLINE void Frustum::TransformWithoutScaling(const tofu::Transform& t)
	{
		orientation = t.GetRotation() * orientation;
		origin = t.TransformPosition(origin);
	}

	TF_INLINE bool Frustum::Intersects(const BoundingSphere& sphere) const
	{
		// construct planes
		math::float3 planeAxes[6] =
		{
			{ 0.0f, 0.0f, -1.0f },  // near plane
			{ 0.0f, 0.0f, 1.0f },  // far plane
			{ 1.0f, 0.0f, -rightSlope }, // right plane
			{ -1.0f, 0.0f, leftSlope }, // left plane
			{ 0.0f, 1.0f, -topSlope }, // top plane
			{ 0.0f, -1.0f, bottomSlope }, // bottom plane
		};

		float planeDists[6] = { -nearPlane, farPlane, 0, 0, 0, 0 };

		planeAxes[2] = math::normalize(planeAxes[2]);
		planeAxes[3] = math::normalize(planeAxes[3]);
		planeAxes[4] = math::normalize(planeAxes[4]);
		planeAxes[5] = math::normalize(planeAxes[5]);

		float radius = sphere.radius;
		math::float3 sphereCenter = math::inverse(orientation) * (sphere.center - origin);

		bool outside = false;
		bool insideAll = true;
		bool centerInsideAll = true;

		float dist[6] = {};

		for (size_t i = 0; i < 6; ++i)
		{
			dist[i] = math::dot(sphereCenter, planeAxes[i]) - planeDists[i];

			outside |= (dist[i] > radius);
			insideAll &= (dist[i] <= -radius);
			centerInsideAll &= (dist[i] <= 0);
		}

		if (outside) return false;
		if (insideAll) return true;
		if (centerInsideAll) return true;

		static const size_t adjacentFaces[6][4] =
		{
			{ 2, 3, 4, 5 }, // 0, near plane
			{ 2, 3, 4, 5 }, // 1, far plane
			{ 0, 1, 4, 5 }, // 2, right plane
			{ 0, 1, 4, 5 }, // 3, left plane
			{ 0, 1, 2, 3 }, // 4, top plane
			{ 0, 1, 2, 3 }, // 5, bottom plane
		};

		bool intersects = false;

		for (size_t i = 0; i < 6; ++i)
		{
			math::float3 point = sphereCenter - planeAxes[i] * dist[i];

			bool insideFace = true;

			for (size_t j = 0; j < 4; ++j)
			{
				size_t planeIdx = adjacentFaces[i][j];

				insideFace &= (math::dot(point, planeAxes[planeIdx]) <= planeDists[planeIdx]);
			}

			intersects |= ((dist[i] > 0) && insideFace);
		}

		if (intersects) return true;

		// construct corners of frustum
		math::float3 rightTopA{ rightSlope, topSlope, 1.0f };
		math::float3 rightBottomA{ rightSlope, bottomSlope, 1.0f };
		math::float3 leftTopA{ leftSlope, topSlope, 1.0f };
		math::float3 leftBottomA{ leftSlope, bottomSlope, 1.0f };
		math::float3 nearA{ nearPlane, nearPlane, nearPlane };
		math::float3 farA{ farPlane, farPlane, farPlane };

		math::float3 corners[8] =
		{
			rightTopA * nearA,			// 0, near right top
			rightBottomA * nearA,		// 1, near right bottom
			leftTopA * nearA,			// 2, near left top
			leftBottomA * nearA,		// 3, near left bottom
			rightTopA * farA,			// 4, far right top
			rightBottomA * farA,		// 5, far right bottom
			leftTopA * farA,			// 6, far left top
			leftBottomA * farA,			// 7, far left bottom
		};

		static const size_t edges[12][2] =
		{
			{ 0, 1 }, { 2, 3 }, { 0, 2 }, { 1, 3 },
			{ 4, 5 }, { 6, 7 }, { 4, 6 }, { 5, 7 },
			{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
		};

		float radiusSq = sphere.radius * sphere.radius;

		for (size_t i = 0; i < 12; ++i)
		{
			size_t ei0 = edges[i][0];
			size_t ei1 = edges[i][1];

			math::float3 point = NearestPointOnLineSegment(corners[ei0], corners[ei1], sphereCenter);

			math::float3 delta = sphereCenter - point;
			float distSq = math::dot(delta, delta);

			intersects |= (distSq <= radiusSq);
		}

		return intersects;
	}

	TF_INLINE bool Frustum::Intersects(const BoundingBox& aabb) const
	{
		OrientedBoundingBox obb(aabb.center, aabb.extends, math::quat());
		return Intersects(obb);
	}

	TF_INLINE bool Frustum::Intersects(const OrientedBoundingBox& obb) const
	{
		// construct planes
		math::float3 planeAxes[6] =
		{
			{ 0.0f, 0.0f, -1.0f },  // near plane
			{ 0.0f, 0.0f, 1.0f },  // far plane
			{ 1.0f, 0.0f, -rightSlope }, // right plane
			{ -1.0f, 0.0f, leftSlope }, // left plane
			{ 0.0f, 1.0f, -topSlope }, // top plane
			{ 0.0f, -1.0f, bottomSlope }, // bottom plane
		};

		float planeDists[6] = { -nearPlane, farPlane, 0, 0, 0, 0 };

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
			float dist = math::dot(center, planeAxes[i]) - planeDists[i];
			math::float3 radius{
				math::dot(R[0], planeAxes[i]),
				math::dot(R[1], planeAxes[i]),
				math::dot(R[2], planeAxes[i])
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
		math::float3 nearA{ nearPlane, nearPlane, nearPlane };
		math::float3 farA{ farPlane, farPlane, farPlane };

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

		float distA[6] = { -nearPlane, farPlane, 0, 0, 0, 0 };

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
		math::float3 nearB{ frustum.nearPlane, frustum.nearPlane, frustum.nearPlane };
		math::float3 farB{ frustum.farPlane, frustum.farPlane, frustum.farPlane };

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
		math::float3 nearA{ nearPlane, nearPlane, nearPlane };
		math::float3 farA{ farPlane, farPlane, farPlane };

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

			outside |= (minD > distB[i]);
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
