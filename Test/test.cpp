#include <asyncworker.h>

void f(int a, double b)
{
	std::cout << a << " " << b << std::endl;
}

void g()
{
	std::cout << "Done!" << std::endl;
}

TEST(TestSyntax, SimpleFunctions) 
{
	AsyncWorker::instance().execute(f, g, 2, 4);
	EXPECT_TRUE(true);
}


struct A
{
	void f(int a, double b) const
	{
		std::cout << a << " " << b << std::endl;
	}
};

class B
{
public:
	static void g()
	{
		std::cout << "Done!" << std::endl;
	}
};

TEST(TestSyntax, ClassMembersAndLambdas)
{
	const A a;

	double ddd = 4;
	AsyncWorker::instance().execute([&a](int _a, double& _b) { a.f(_a, _b); }, B::g, 2, ddd);

	EXPECT_TRUE(true);
}

std::tuple<int, double, char> fArg(const int& a, double&& b, char* c)
{
	std::cout << a << " " << b << " " << *c << std::endl;
	return { a, b, *c };
}

void gArg(std::tuple<int, double, char> res)
{
	auto[a, b, c] = res;
	std::cout << "It's really this values: " << a << " " << b << " " << c << std::endl;
}

TEST(TestSyntax, CallbacksWithArgs)
{
	char c = 'X';
	AsyncWorker::instance().execute(fArg, gArg, 2, 4, &c);
	EXPECT_TRUE(true);
}