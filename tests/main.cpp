
#define CHECK(x) {int ret = 0; if ((ret = (x)) != 0) return ret; }

extern int test_math();
extern int test_collision();

int main()
{
	CHECK(test_math());
	CHECK(test_collision());
	return 0;
}