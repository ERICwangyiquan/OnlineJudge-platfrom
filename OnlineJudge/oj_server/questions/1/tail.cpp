// only to make coding tail.cpp easily
#ifndef COMPILER_ONLINE
#include "head.cpp"
#endif

void Test1()
{
    // define a temp instance to call func
    bool ret = Solution().isPalindrome(121);
    if(ret)
    {
        std::cout << "Test 1 pass, case: x = 121" << std::endl;
    }
    else
    {
        std::cout << "Failed to pass Test 1" << std::endl;
    }
}

void Test2()
{
    // define a temp instance to call func
    bool ret = Solution().isPalindrome(-10);
    if(!ret)
    {
        std::cout << "Test 2 pass, case: x = -10" << std::endl;
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

    return 0;
}