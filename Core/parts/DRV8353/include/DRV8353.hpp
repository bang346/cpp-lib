#ifndef DRV8353_HPP
#define DRV8353_HPP

#include <stdint.h>

#include "bus_if.hpp"

// Treiber als Template
template<typename CsPin, typename EnPin, typename FaultPin, typename Delay>
class DRV8353 {
public:
    explicit DRV8353(bus_if& spi) : spi_(spi) {}

    void init() {
        EnPin::high();
        Delay::ms(10);
    }

    void writeReg(uint16_t v) {
        CsPin::low();
        uint8_t tx[2] = { uint8_t(v>>8), uint8_t(v) };
        spi_.transmit(tx, nullptr, 2);
        CsPin::high();
    }

    bool fault() { return FaultPin::read(); }

private:
    bus_if& spi_;
};

#endif
