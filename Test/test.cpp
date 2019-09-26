#include <asyncworker.h>

AsyncWorker worker;

std::atomic<int> counter1 = 0;
const int timeout = 5;

void f(int a, double b)
{
	std::this_thread::sleep_for(std::chrono::seconds(timeout));
}

void g()
{
	++counter1;
	std::cout << "DONE1!" << std::endl;
}

TEST(TestSyntax, SimpleFunctions) 
{
	const int N = 6;

	for (int i = 0; i < N; ++i)
	{
		worker.executeWithCallback(f, g, 2*i+1, 2*i+2);
	}
	std::this_thread::sleep_for(std::chrono::seconds(timeout * static_cast<int>(std::ceil(N/3.0)) + 1));
	EXPECT_EQ(counter1, N);
}


std::atomic<int> counter2 = 0;
struct A
{
	void f(int a, double& b) const
	{
		std::cout << a << " " << (b += 1.0);
		std::this_thread::sleep_for(std::chrono::seconds(timeout));
	}
};

class B
{
public:
	static void g()
	{
		++counter2;
		std::cout << "DONE2!" << std::endl;
	}
};

TEST(TestSyntax, ClassMembersAndLambdas)
{
	const A a;

	double ddd = 10;
	worker.executeWithCallback([&a](int _a, double& _b) { a.f(_a, _b); }, B::g, 9, std::ref(ddd));
	worker.executeWithCallback([&a](int _a, double& _b) { a.f(_a, _b); }, B::g, 11, std::ref(ddd));
	std::this_thread::sleep_for(std::chrono::seconds(timeout + 1));
	EXPECT_EQ(counter2, 2);
	EXPECT_EQ(ddd, 12);
}

std::tuple<int, double, char> fArg(const int& a, double b, char* c)
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
	worker.executeWithCallback(fArg, gArg, 2, 4, &c);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	EXPECT_TRUE(true);
}