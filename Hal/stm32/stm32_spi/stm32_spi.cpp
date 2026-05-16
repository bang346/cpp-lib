#include "stm32_spi.hpp"

stm32_spi::stm32_spi(SPI_HandleTypeDef *const hspi)
    : hspi_{hspi},
      port_{nullptr},
      pin_{GPIO_PIN_All}
{
}

stm32_spi::stm32_spi(SPI_HandleTypeDef *const hspi, GPIO_TypeDef* port, const uint16_t &pin)
    : hspi_{hspi},
      port_{port},
      pin_{pin}
{
    HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
}

int stm32_spi::transmit(const uint8_t *const data, const uint8_t len) const
{
    int ret = 0;
    if (pin_ != GPIO_PIN_All)
    {
        HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_RESET);
        ret = HAL_SPI_Transmit(hspi_, data, len, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
    }
    else
    {
    	ret = HAL_SPI_Transmit(hspi_, data, len, HAL_MAX_DELAY);
    }
    return ret;
}
int stm32_spi::receive(uint8_t *data, const uint8_t len) const
{
    return HAL_SPI_Receive(hspi_, data, len, HAL_MAX_DELAY);
}

int stm32_spi::transmitreceive(uint8_t *const data_tx, uint8_t *data_rx, const uint8_t len) const
{
    int ret = 0;
    if (pin_ != GPIO_PIN_All)
    {
        HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_RESET);
        ret = HAL_SPI_TransmitReceive(hspi_, data_tx, data_rx, len, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
    }
    else
    {
        ret = HAL_SPI_TransmitReceive(hspi_, data_tx, data_rx, len, HAL_MAX_DELAY);
    }

    return ret;
}
