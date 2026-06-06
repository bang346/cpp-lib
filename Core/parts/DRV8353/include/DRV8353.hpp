#ifndef DRV8353_HPP
#define DRV8353_HPP

#include <stdint.h>

#include "DRV8353_defs.hpp"
#include "bus_if.hpp"

/// @brief DRV8353 Driver Class
/// @tparam CsPin       Target dependet chip select-GPIO implementation
/// @tparam EnPin       Target dependet Enable-GPIO implementation
/// @tparam FaultPin    Target dependet CS-GPIO implementation
/// @tparam Delay       Target dependet blocking delay implementation
template <typename CsPin, typename EnPin, typename FaultPin, typename Delay>
class DRV8353
{
public:
    /// @brief Class Constructor
    /// @note           explicit prohibits unwanted initializations
    /// @param spi      Spi-object
    explicit DRV8353(bus_if &spi)
        : spi_(spi)
    {
    }

    /// @brief
    /// @param delay
    /// @return
    int init()
    {
        CsPin::high();
        EnPin::high();
        return 0;
    }

    /// @brief
    /// @param addr     4-Bit register address
    /// @return         0xffff = error,
    ///                 otherwise the register value
    uint16_t ReadReg(const uint8_t &addr) const
    {
        if (addr > DRV8353_DRIVER_CONFIG_ADDR)
        {
            return 0xffff;
        }
        uint16_t tx = 0;
        uint16_t rx = 0;

        tx = static_cast<uint16_t>((1U << 15) | ((addr & 0x0F) << 11));

        CsPin::low();
        spi_.transmitreceive(
            reinterpret_cast<uint8_t *>(&tx),
            reinterpret_cast<uint8_t *>(&rx),
            2);
        CsPin::high();
        return rx & DRV8353_REG_DATA_MASK;
    }

    /// @brief
    /// @param addr     4-Bit register address
    /// @param data     10-Bit data
    /// @return         0 = success,
    ///                 0xffff = error
    uint16_t WriteReg(const uint8_t &addr, const uint16_t &data)
    {
        if (addr > DRV8353_DRIVER_CONFIG_ADDR)
        {
            return 0xffff;
        }

        uint16_t frame =
            static_cast<uint16_t>((0U << 15) |
                                  ((addr & 0x0F) << 10) |
                                  (data & 0x03FF));

        uint8_t tx[2] = {
            static_cast<uint8_t>((frame >> 8) & 0xFF),
            static_cast<uint8_t>(frame & 0xFF)};

        uint8_t rx[2] = {0, 0};

        return spi_.transmitreceive(tx, rx, 2);
    }

    bool fault() const { return FaultPin::read(); }

private:
    bus_if &spi_;
};

#endif
