#include <gtest/gtest.h>
#include <iostream>
#include <algorithm>

#include "etl/circular_buffer.h"
#include "etl/vector.h"

#include "prittyprinting.hpp"

TEST(CircularBuffer, initial)
{
    etl::circular_buffer<int, 5> buffer;

    for (size_t i = 0; i < 8; i++)
    {
        buffer.push(i);
    }

    auto size = buffer.size();

    for (size_t i = 0; i < size; i++)
    {
        std::cout << GREEN << buffer[i] << std::endl;
    }

    std::for_each(buffer.cbegin(), buffer.cend(), [](const auto &value)
                  { std::cout << RED << value << std::endl; });
    buffer.fill(1);

    std::for_each(buffer.cbegin(), buffer.cend(), [](const auto &value)
                  { std::cout << BOLDBLACK << value << std::endl; });
    std::cout << RESET << std::endl;
}

TEST(Vector, initial)
{
    etl::vector<int, 7> buffer;

    for (size_t i = 0; i < 7; i++)
    {
        buffer.push_back(i);
    }

    buffer.erase(buffer.begin(), buffer.begin() + 3);

    int *ptr = buffer.data();

    for (size_t i = 0; i < 3; i++)
    {
        std::cout << GREEN << ptr[i] << std::endl;
    }
    std::cout << RESET << std::endl;
}