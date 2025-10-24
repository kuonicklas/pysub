#include "pch.h"
#include "CppUnitTest.h"

#include "../pysub/interface.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace tests
{
	TEST_CLASS(tests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Interface i;
			Assert::AreEqual(2, i.add(1, 1));
		}

		TEST_METHOD(Method1)
		{
			Logger::WriteMessage("In Method1");
			Assert::AreEqual(0, 0);
		}

		TEST_METHOD(Method2)
		{
			Assert::Fail(L"Fail");
		}

		//TEST_METHOD(TestMethod2)
		//{
		//	Interface i;
		//	Assert::AreEqual(3, i.add(1, 2));
		//}
	};
}
