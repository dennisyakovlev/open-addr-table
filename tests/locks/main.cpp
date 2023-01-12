#include <iostream>
#include <string>

#include "Correctness.h"
#include "QueueFairness.h"

int main(int argc, char const *argv[])
{
    auto res = correctness_tests();
    std::cout  << "\n";
    res += queue_fairness_test();

    return res;
}
