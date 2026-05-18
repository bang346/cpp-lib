#include <gtest/gtest.h>
#include <cmath>
#include <iostream>

#include "prittyprinting.hpp"
#include "mock_spi.hpp"
#include "DRV8353.hpp"

// GPIO-Policies (plattformabhängig)
template<int Port, int Pin>
struct GpioOut {
    static inline void high() { std::cout << BOLDBLUE << "Set Port: " << Port << " Pin: " << Pin << RESET << std::endl; }
    static inline void low()  { std::cout << BOLDBLUE << "Reset Port: " << Port << " Pin: " << Pin << RESET << std::endl; }
};

struct DelayPolicy {
    static inline void us(uint32_t t) { std::cout << BOLDBLUE << "sleep for" << t << " us" << std::endl;}
    static inline void ms(uint32_t t) { std::cout << BOLDBLUE << "sleep for" << t << " ms" << std::endl; }
};



TEST(SPI, Example)
{
using CS    = GpioOut<1, 4>;
using EN    = GpioOut<1, 5>;
using FAULT = GpioOut<1, 6>;
using DLY   = DelayPolicy;

mock_spi mock;
DRV8353<CS, EN, FAULT, DLY> drv(mock);

drv.init();
}


