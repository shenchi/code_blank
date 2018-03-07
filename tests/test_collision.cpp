#include "../engine/Collision.h"

using namespace tofu;

int test_collision()
{

	// frustum
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
	}

	return 0;
}