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
template<typename CsPin, typename EnPin, typename FaultPin, typename Delay>
class DRV8353 {
public:
    /// @brief Class Constructor
    /// @note           explicit prohibits unwanted initializations
    /// @param spi      Spi-object
    explicit DRV8353(bus_if &spi)
    : spi_(spi)
    {}

    /// @brief 
    /// @param delay 
    /// @return 
    int init()
    {
        CsPin::high();
        return 0;
    }
    uint16_t ReadReg(const uint8_t &addr) const
    {
        if(addr > DRV8353_DRIVER_CONFIG_ADDR)
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


    bool fault() const { return FaultPin::read(); }

private:
    bus_if& spi_;
};

#endif
