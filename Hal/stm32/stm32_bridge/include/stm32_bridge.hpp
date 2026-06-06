#ifndef STM32_BRIDGE_HPP
#define STM32_BRIDGE_HPP

#include <stdint.h>

#include "sixstep.hpp"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_tim.h"
namespace
{

};

class STM32Bridge
{
private:
    TIM_HandleTypeDef *TimHandle_; // Advanced Timer Handle

public:
    STM32Bridge(TIM_HandleTypeDef *TimHandle)
        : TimHandle_{TimHandle}
    {
    }
    ~STM32Bridge() = default;

    int init()
    {

        TimHandle_->Init.Period = 4095;
        TimHandle_->Init.Prescaler = 0;
        TimHandle_->Init.ClockDivision = 0;
        TimHandle_->Init.CounterMode = TIM_COUNTERMODE_UP;
        TimHandle_->Init.RepetitionCounter = 0;
        TimHandle_->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        if (HAL_TIM_OC_Init(TimHandle_) != HAL_OK)
        {
            /* Initialization Error */
            return -1;
        }

        /*##-2- Configure the output channels ######################################*/
        /* Common configuration for all channels */
        TIM_OC_InitTypeDef sConfig;
        sConfig.OCMode = TIM_OCMODE_TIMING;
        sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        sConfig.OCIdleState = TIM_OCIDLESTATE_SET;
        sConfig.OCNIdleState = TIM_OCNIDLESTATE_SET;
        sConfig.OCFastMode = TIM_OCFAST_DISABLE;

        /* Set the pulse value for channel 1 */
        sConfig.Pulse = 2047;
        if (HAL_TIM_OC_ConfigChannel(TimHandle_, &sConfig, TIM_CHANNEL_1) != HAL_OK)
        {
            /* Configuration Error */
            return -1;
        }

        /* Set the pulse value for channel 2 */
        sConfig.Pulse = 1023;
        if (HAL_TIM_OC_ConfigChannel(TimHandle_, &sConfig, TIM_CHANNEL_2) != HAL_OK)
        {
            /* Configuration Error */
            return -1;
        }

        /* Set the pulse value for channel 3 */
        sConfig.Pulse = 511;
        if (HAL_TIM_OC_ConfigChannel(TimHandle_, &sConfig, TIM_CHANNEL_3) != HAL_OK)
        {
            /* Configuration Error */
            return -1;
        }

        /*##-3- Configure the Break stage ##########################################*/
        TIM_BreakDeadTimeConfigTypeDef sConfigBK;

        sConfigBK.OffStateRunMode = TIM_OSSR_ENABLE;
        sConfigBK.OffStateIDLEMode = TIM_OSSI_ENABLE;
        sConfigBK.LockLevel = TIM_LOCKLEVEL_OFF;
        sConfigBK.BreakState = TIM_BREAK_ENABLE;
        sConfigBK.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
        sConfigBK.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
        sConfigBK.DeadTime = 0;

        if (HAL_TIMEx_ConfigBreakDeadTime(TimHandle_, &sConfigBK) != HAL_OK)
        {
            /* Configuration Error */
            return -1;
        }

        /*##-4- Configure the commutation event: software event ####################*/
        // COM-Interrupt aktivieren
        __HAL_TIM_ENABLE_IT(TimHandle_, TIM_IT_COM);

        // COM-Event per Software konfigurieren
        HAL_TIMEx_ConfigCommutEvent_IT(TimHandle_,
                                       TIM_TS_NONE,
                                       TIM_COMMUTATION_SOFTWARE);

        // PWM / complementary outputs starten
        HAL_TIM_PWM_Start(TimHandle_, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Start(TimHandle_, TIM_CHANNEL_1);

        HAL_TIM_PWM_Start(TimHandle_, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Start(TimHandle_, TIM_CHANNEL_2);

        HAL_TIM_PWM_Start(TimHandle_, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Start(TimHandle_, TIM_CHANNEL_3);
        return 0;
    }

    int set_pwm(SixStep_NS::channel_e chan)
    {
        switch (chan)
        {
        case SixStep_NS::channel_e::phase_A:
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
            LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
            break;
        case SixStep_NS::channel_e::phase_B:
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2N);
            LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1);
            break;
        case SixStep_NS::channel_e::phase_C:
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3N);
            LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_PWM1);
            break;
        }
        return 0;
    }
    int set_high_z(SixStep_NS::channel_e chan)
    {
        switch (chan)
        {
        case SixStep_NS::channel_e::phase_A:
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
            break;
        case SixStep_NS::channel_e::phase_B:
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2N);
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2);
            break;
        case SixStep_NS::channel_e::phase_C:
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3N);
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3);
            break;
        }
        return 0;
    }
    int set_gnd(SixStep_NS::channel_e chan)
    {
        switch (chan)
        {
        case SixStep_NS::channel_e::phase_A:
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
            LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_FORCED_ACTIVE);
            break;
        case SixStep_NS::channel_e::phase_B:
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2);
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2N);
            LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_FORCED_ACTIVE);
            break;
        case SixStep_NS::channel_e::phase_C:
            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3N);                       /////////////////////////////////////// HERE
            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);                         /////////////////////////////////////// HERE
            LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH3N, LL_TIM_OCMODE_FORCED_ACTIVE); /////////////////////////////////////// HERE
            break;
        }
        return 0;
    }

    /// @brief Sets the Duty for all channels
    /// @param value    New duty in percent [0;100]
    void set_duty(const uint16_t &value)
    {
        uint16_t internal_value = value;
        if (value > 100)
        {
            internal_value = 100;
        }
        uint16_t Period = TimHandle_->Init.Period;
        uint16_t new_cnt = Period * internal_value / 100;
        TimHandle_->Instance->CCR1 = new_cnt;
        TimHandle_->Instance->CCR2 = new_cnt;
        TimHandle_->Instance->CCR3 = Period - new_cnt; /////////////////////////////////////// HERE
    }
};

#endif
