
#define CHECK(x) {int ret = 0; if ((ret = (x)) != 0) return ret; }

extern int test_math();

int main()
{
	CHECK(test_math());
	return 0;
}