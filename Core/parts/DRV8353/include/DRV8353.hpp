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
    using register_t = uint16_t;

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
        register_t register_value = 0;
        // 1. Gate Drive HS Field
        // @see p.60
        Delay::us(10);
        register_value |= DRV8353_LOCK_UNLOCK | DRV8353_IDRIVEP_HS_100MA | DRV8353_IDRIVEN_HS_200MA;
        WriteReg(DRV8353_GATE_DRIVE_HS_ADDR, register_value);
        uint16_t buffer = 0;
        buffer = (ReadReg(DRV8353_GATE_DRIVE_HS_ADDR) & DRV8353_REG_DATA_MASK);
        if (buffer != register_value)
        {
            return -1;
        }
        register_value = 0;
        buffer = 0;

        // 2. Gate Drive LS Register Field
        // @see p.61
        Delay::us(10);
        register_value |= DRV8353_TDRIVE_1000NS | DRV8353_IDRIVEP_LS_100MA | DRV8353_IDRIVEN_LS_200MA;
        WriteReg(DRV8353_GATE_DRIVE_LS_ADDR, register_value);
        buffer = (ReadReg(DRV8353_GATE_DRIVE_LS_ADDR) & DRV8353_REG_DATA_MASK);
        if (buffer != register_value)
        {
            return -1;
        }
        register_value = 0;
        buffer = 0;
        // 3. OCP Control Field
        // @see p.62
        // @details         To clear the latched reset toggle enable or reset CLR_FLT
        Delay::us(10);
        register_value |= DRV8353_OCP_MODE_LATCHED_FAULT | DRV8353_VDS_LVL_0P40V;
        WriteReg(DRV8353_OCP_CONTROL_ADDR, register_value);
        buffer = (ReadReg(DRV8353_OCP_CONTROL_ADDR) & DRV8353_REG_DATA_MASK);
        if (buffer != register_value)
        {
            return -1;
        }
        register_value = 0;
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

        // CsPin::low();
        spi_.transmitreceive(
            reinterpret_cast<uint8_t *>(&tx),
            reinterpret_cast<uint8_t *>(&rx),
            1);
        // CsPin::high();
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
        uint16_t data_rx = 0;
        uint16_t data_tx = static_cast<uint16_t>((0U << 15) | ((addr & 0x0F) << 11) | (data & DRV8353_REG_DATA_MASK));
        // CsPin::low();
        uint16_t ret = spi_.transmitreceive(reinterpret_cast<uint8_t *>(&data_tx),
                                            reinterpret_cast<uint8_t *>(&data_rx),
                                            1);
        // CsPin::high();
        return ret;
    }

    bool fault() const { return FaultPin::read(); }

private:
    bus_if &spi_;
};

#endif
