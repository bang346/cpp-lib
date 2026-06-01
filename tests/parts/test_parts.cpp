#include <gtest/gtest.h>
#include <cmath>
#include <iostream>

#include "prittyprinting.hpp"
#include "mock_spi.hpp"
#include "DRV8353.hpp"

using ::testing::NiceMock;
using namespace ::testing;

// GPIO-Policies (plattformabhängig)
template<int Port, int Pin>
struct GpioOut {
    static inline void high() { std::cout << BOLDBLACK << "Set Port: " << Port << " Pin: " << Pin << RESET << std::endl; }
    static inline void low()  { std::cout << BOLDBLACK << "Reset Port: " << Port << " Pin: " << Pin << RESET << std::endl; }
};

struct DelayPolicy {
    static inline void us(uint32_t t) { std::cout << BOLDBLACK << "sleep for" << t << " us" << RESET << std::endl;}
    static inline void ms(uint32_t t) { std::cout << BOLDBLACK << "sleep for" << t << " ms" << RESET << std::endl;}
};

struct GpioIn
{
    static inline bool read(){ std::cout << BOLDBLACK << "Gpio reads true" << RESET << std::endl; return true;}
};



TEST(SPI, TransmitCalled)
{
    NiceMock<mock_spi> mock;
    EXPECT_CALL(mock, transmitreceive(_, _, 2))
        .Times(1)
        .WillOnce(Return(0));

    using CS    = GpioOut<1, 4>;
    using EN    = GpioOut<1, 5>;
    using FAULT = GpioOut<1, 6>;
    using DLY   = DelayPolicy;

    
    DRV8353<CS, EN, FAULT, DLY> drv(mock);

    drv.init();
    drv.ReadReg(0x00);

}

TEST(SPI, ReadReg)
{
    using CS    = GpioOut<1, 4>;
    using EN    = GpioOut<1, 5>;
    using FAULT = GpioIn;
    using DLY   = DelayPolicy;

    NiceMock<mock_spi> mock;
    DRV8353<CS, EN, FAULT, DLY> drv(mock);

    const uint8_t expected_tx[] = {0x00, 0x80};
    const uint8_t fake_rx[]    = {0x55, 0x66};

    EXPECT_CALL(mock, transmitreceive(_, _, 2))
        .WillOnce(Invoke([&](uint8_t* data_tx, uint8_t* data_rx, const uint8_t len){
            EXPECT_EQ(len, 2);

            EXPECT_THAT(
                std::vector<uint8_t>(data_tx, data_tx + len),
                ElementsAreArray(expected_tx)
            );

            std::copy(fake_rx, fake_rx + len, data_rx);

            return 0;
        }));

    drv.fault();
    drv.ReadReg(0x00);
}

TEST(SPI, WriteReg)
{
    using CS    = GpioOut<1, 4>;
    using EN    = GpioOut<1, 5>;
    using FAULT = GpioIn;
    using DLY   = DelayPolicy;

    NiceMock<mock_spi> mock;
    DRV8353<CS, EN, FAULT, DLY> drv(mock);

    const uint8_t expected_tx[] = {0x1, 0xff};
    const uint8_t fake_rx[]    = {0x55, 0x66};

    EXPECT_CALL(mock, transmitreceive(_, _, 2))
        .WillOnce(Invoke([&](uint8_t* data_tx, uint8_t* data_rx, const uint8_t len){
            EXPECT_EQ(len, 2);

            EXPECT_THAT(
                std::vector<uint8_t>(data_tx, data_tx + len),
                ElementsAreArray(expected_tx)
            );

            std::copy(fake_rx, fake_rx + len, data_rx);

            return 0;
        }));


        drv.WriteReg(DRV8353_DRIVER_CONFIG_ADDR, 0x3ff);

}


