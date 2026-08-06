#include <gtest/gtest.h>
#include <iostream>
#include <algorithm>

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

    etl::vector<int, 20> buffer;

    buffer.uninitialized_resize(10);
    auto ptr = buffer.data();
    for (size_t i = 0; i < 4; i++)
    {
        ptr[i] = i;
    }

    etl::vector<int, 20> buffer2;
    for (auto &&i : buffer)
    {
        std::cout << i << "\n";
    }
    int i = 0;
}