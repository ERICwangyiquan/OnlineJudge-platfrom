// only to make coding tail.cpp easily
#ifndef COMPILER_ONLINE
#include "head.cpp"
#endif

void Test1()
{
    int max = Solution().Max({1, 2, 3, 4, 5, 6, 7});
    if (max == 7)
    {
        std::cout << "Test 1 pass, case: {1, 2, 3, 4, 5, 6, 7}" << std::endl;
    }
    else
    {
        std::cout << "Failed to pass Test 1" << std::endl;
    }
}

void Test2()
{
    int max = Solution().Max({-1, -2, -3, -4, -5, -6, -7});
    if (max == -1)
    {
        std::cout << "Test 2 pass, case: {-1, -2, -3, -4, -5, -6, -7}" << std::endl;
    }
    else
    {
        std::cout << "Failed to pass Test 2" << std::endl;
    }
}

int main()
{
    Test1();
    Test2();
}