#include "../engine/Collision.h"

using namespace tofu;

int test_collision()
{

	// frustum / frustum
	{
		Frustum a{ math::float3{0, 0, 0}, math::quat(1, 0, 0, 0), 1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 1.0f };
		Frustum b = a;

		Transform t{};
		t.SetRotation(math::radians(90.0f), math::float3{ 0, 1, 0 });
		b.Transform(t);

		if (!a.Intersects(b) || !a.Intersects(b))
		{
			return __LINE__;
		}

		t.SetTranslation(1, 0, 0);
		b = a;
		b.Transform(t);

		if (a.Intersects(b) || a.Intersects(b))
		{
			return __LINE__;
		}

		t.SetRotation(math::radians(-90.0f), math::float3{ 0, 1, 0 });
		b = a;
		b.Transform(t);

		if (!a.Intersects(b) || !a.Intersects(b))
		{
			return __LINE__;
		}

		t.SetRotation(math::radians(90.0f), math::float3{ 1, 0, 0 });
		t.SetTranslation(2.001f, 2.001f, 2.001f);
		b = a;
		b.Transform(t);

		if (a.Intersects(b) || a.Intersects(b))
		{
			return __LINE__;
		}

		t.SetTranslation(1.999f, 1.999f, 1.999f);
		b = a;
		b.Transform(t);

		if (!a.Intersects(b) || !a.Intersects(b))
		{
			return __LINE__;
		}

		// b is slim
		Frustum c = Frustum{ math::float3(), math::quat(), 0.1f, -0.1f, 0.1f, -0.1f, 0.0f, 10.0f };
		t.SetTranslation(0.0f, 5.0f, 2.0f);
		b = c;
		b.Transform(t);

		if (a.Intersects(b) || a.Intersects(b))
		{
			return __LINE__;
		}

		// penetration
		t.SetTranslation(0.0f, 5.0f, 0.5f);
		b = c;
		b.Transform(t);

		if (!a.Intersects(b) || !a.Intersects(b))
		{
			return __LINE__;
		}

		Transform ta, tb;
		ta.SetRotation(math::radians(45.0f), math::radians(90.0f), 0.0f);
		tb.SetRotation(math::radians(45.0f), math::radians(-90.0f), 0.0f);
		ta.SetTranslation(-5, 5, 0);
		tb.SetTranslation(5, 5, 0);
		a = c;
		b = c;
		a.Transform(ta);
		b.Transform(tb);
		if (!a.Intersects(b) || !a.Intersects(b))
		{
			return __LINE__;
		}

		float l = 10 / math::sqrt(2.0f);
		float d = math::sqrt(2.0f);
		ta.SetRotation(math::radians(45.0f), 0.0f, math::radians(45.0f));
		tb.SetRotation(math::radians(135.0f), 0.0f, math::radians(45.0f));
		ta.SetTranslation(0, 0, 0);
		tb.SetTranslation(d - 0.001f, 0, l);
		a = c;
		b = c;
		a.Transform(ta);
		b.Transform(tb);

		if (!a.Intersects(b) || !a.Intersects(b))
		{
			return __LINE__;
		}

		tb.SetTranslation(d + 0.001f, 0, l);
		a = c;
		b = c;
		a.Transform(ta);
		b.Transform(tb);

		if (a.Intersects(b) || a.Intersects(b))
		{
			return __LINE__;
		}
	}

	// frustum / obb
	{
		Frustum a{ math::float3(), math::quat(), 0.1f, -0.1f, 0.1f, -0.1f, 0.0f, 10.0f };
		OrientedBoundingBox b(math::float3(), math::float3{ 1, 1, 1 }, math::quat());
		Frustum c = a;
		OrientedBoundingBox d = b;

		Transform t;
		t.SetTranslation(0, 0, 5);
		b.Transform(t);

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(0, 0, 11);
		b.Transform(t);

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(0, 0, 11.001f);
		b.Transform(t);

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(0, 0, -1);
		b.Transform(t);

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(0, 0, -1.001f);
		b.Transform(t);

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(2, 2, 11);
		b.Transform(t);

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(2.001f, 2, 11);
		b.Transform(t);

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		b = d;
		t.SetTranslation(2, 2.001f, 11);
		b.Transform(t);

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		float l = math::sqrt(3.0f);
		t.SetRotation(0.0, math::atan(1.0f / math::sqrt(2.0f)), math::radians(45.f));
		t.SetTranslation(0.5f + l - 0.001f, 0, 5);
		b = d;
		b.Transform(t);

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		t.SetTranslation(0.5f + l + 0.001f, 0, 5);
		b = d;
		b.Transform(t);

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		l = (10 / math::sqrt(2.0f) + 1);
		float l2 = l / math::sqrt(2.0f);
		Transform ta, tb;
		ta.SetRotation(math::radians(45.0f), math::radians(45.0f), 0);
		ta.SetTranslation(-l2 + 0.001f, l - 0.001f, -l2 + 0.001f);
		tb.SetRotation(0.0f, math::radians(45.0f), 0.0f);
		a = c;
		b = d;
		a.Transform(ta);
		b.Transform(tb);

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		ta.SetTranslation(-l2 - 0.001f, l + 0.001f, -l2 - 0.001f);
		a = c;
		b = d;
		a.Transform(ta);
		b.Transform(tb);

		if (a.Intersects(b))
		{
			return __LINE__;
		}
	}

	// frustum / sphere
	{
		Frustum a{ math::float3(), math::quat(), 0.1f, -0.1f, 0.1f, -0.1f, 0.0f, 10.0f };
		BoundingSphere b(math::float3(), 1);
		b.center.z = -1;

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b.center.z = -1.001f;

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		b.center.z = 11;

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b.center.z = 11.001f;

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		b.center = math::float3{ 1, 1, 10 } + (1 / math::sqrt(3.0f)) - 0.001f;

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b.center = math::float3{ 1, 1, 10 } +(1 / math::sqrt(3.0f)) + 0.001f;

		if (a.Intersects(b))
		{
			return __LINE__;
		}

		a = Frustum{ math::float3{}, math::quat{}, 1, -1, 1, -1, 0, 1 };
		a.orientation = math::euler(0, math::radians(-45.0f), 0);

		b.center = { 1, 0, 0.5f };

		if (!a.Intersects(b))
		{
			return __LINE__;
		}

		b.center = { 1.001f, 0, 0.5f };

		if (a.Intersects(b))
		{
			return __LINE__;
		}
	}

	// obb / obb
	{
		OrientedBoundingBox c(math::float3(), math::float3{ 0.5f, 0.5f, 0.5f }, math::quat());
		OrientedBoundingBox a = c;
		OrientedBoundingBox b = c;

		Transform ta, tb;

		ta.SetRotation(0.0f, 0.0f, math::radians(-45.0f));

		math::quat rot = math::euler(0.0, math::atan(1.0f / math::sqrt(2.0f)), math::radians(45.f));
		rot = ta.GetRotation() * rot;
		tb.SetRotation(rot);

		float l = (0.5f + math::sqrt(3.0f) * 0.5f) / math::sqrt(2.0f);
		tb.SetTranslation(l - 0.001f, -l + 0.001f, 0);

		a.Transform(ta);
		b.Transform(tb);

		if (!a.Intersects(b) || !b.Intersects(a))
		{
			return __LINE__;
		}

		tb.SetTranslation(l + 0.001f, -l - 0.001f, 0);

		b = c;
		b.Transform(tb);

		if (a.Intersects(b) || b.Intersects(a))
		{
			return __LINE__;
		}
	}

	return 0;
}