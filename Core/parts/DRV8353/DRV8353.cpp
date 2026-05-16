#include "DRV8353.hpp"

DRV8353::DRV8353(bus_if &bus, gpio_if &enable)
    : bus_{bus},
      enable_{enable}
{
}



int DRV8353::init(delay_if &delay)
{
	enable_.set_low();
	delay.ms(100);
	enable_.set_high();
	return 0;
}

uint16_t DRV8353::ReadReg(const uint8_t &addr)
{
    uint16_t tx = 0;
    uint16_t rx = 0;

    tx = static_cast<uint16_t>((1U << 15) | ((addr & 0x0F) << 11));

    bus_.transmitreceive(
        reinterpret_cast<uint8_t *>(&tx),
        reinterpret_cast<uint8_t *>(&rx),
        1
    );

    return rx & 0x07FF;
}
