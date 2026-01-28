#include <gtest/gtest.h>
#include "event.h" // TODO: Fix this file


TEST(EventAddTest, test1)
{
    auto f = [](){ std::cout << "hello" << std::endl; };
    for (int i = 0; i < 5; i++)
        EventAdd(i, READ, f);
    EXPECT_EQ(GetEventQuantity(), 5);


    for (int i = 0; i < 5; i++)
        EventDel(i, READ);
    EXPECT_EQ(GetEventQuantity(), 0);
}

TEST(EventTest, test0)
{
    EXPECT_EQ(GetEventQuantity(), 0);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}