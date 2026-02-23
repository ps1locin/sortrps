#include "pch.h"
#include "CppUnitTest.h"
#include <vector>

#include "merge_sort.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace sortrpsTests
{
    TEST_CLASS(MergeSortTests)
    {
    private:
        static bool is_sorted(const std::vector<int>& a)
        {
            for (size_t i = 1; i < a.size(); ++i)
                if (a[i - 1] > a[i]) return false;
            return true;
        }

    public:
        TEST_METHOD(ReverseArray)
        {
            std::vector<int> a{ 5,4,3,2,1 };
            sorting::merge_sort(a);
            Assert::IsTrue(is_sorted(a));
        }

        TEST_METHOD(EmptyArray)
        {
            std::vector<int> a;
            sorting::merge_sort(a);
            Assert::IsTrue(a.empty());
        }

        TEST_METHOD(Duplicates)
        {
            std::vector<int> a{ 3,1,3,0,2 };
            sorting::merge_sort(a);
            Assert::IsTrue(is_sorted(a));
        }
    };
}