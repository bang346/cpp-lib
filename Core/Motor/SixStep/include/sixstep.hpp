#ifndef SIXSTEPP_HPP
#define SIXSTEPP_HPP

#include <stdint.h>

namespace SixStep_NS
{

    enum class channel_state_e : uint8_t
    {
        FLOATING,      // HIGH-Z Phase
        GND,           // Phase Connected to GND
        PWM,           // PWM
        UNINITIALISED, // Phase State is undefinded

    };

    enum class channel_e : uint8_t
    {
        phase_A = 0, // Bezogen auf GHx und GLx
        phase_B = 1,
        phase_C = 2
    };

    enum class step_e : int
    {
        step1 = 0,
        step2 = 1,
        step3 = 2,
        step4 = 3,
        step5 = 4,
        step6 = 5,
        idle = 10, // Floating alle Channel
        stop = 11, // NO
        error = -1 // Fehler
    };

    /// @brief Class for Six-Step Commutation of BLDC-motors
    /// @tparam Bridge  Hardware implementation needs to define
    ///                 the following:
    ///                 - set_gnd(SixStep_NS::channel_e chan)
    ///                 - set_pwm(SixStep_NS::channel_e chan)
    ///                 - set_high_z(SixStep_NS::channel_e chan)
    ///                 - set_duty(const uint16_t &value)
    ///                 - init()
    /// @details        SixStep Explanation:
    ///                 1. Every time the Hall sensor readings change, the state
    ///                 of the three MOSFET half-bridges must change.
    ///                 The new configuration depends on the desired direction of rotation.
    ///                 2. The half-bridges remain in their current state until the Hall sensors change.
    template <typename Bridge>
    class SixStep
    {
    private:
        ///     Constants and LUTS
        Bridge &bridge_; // HW-Implementation
        using state_type = int;
        static inline constexpr int lookup_table_id_[8] = {-1, 0, 2, 1, 4, 5, 3, -1};
        static inline constexpr int lookup_table_step_[2][6] = {{3, 4, 5, 0, 1, 2},
                                                                {0, 1, 2, 3, 4, 5}};
        static inline constexpr channel_state_e sequency[6][3] =
            {
                {channel_state_e::GND, channel_state_e::FLOATING, channel_state_e::PWM},
                {channel_state_e::GND, channel_state_e::PWM, channel_state_e::FLOATING},
                {channel_state_e::FLOATING, channel_state_e::PWM, channel_state_e::GND},
                {channel_state_e::PWM, channel_state_e::FLOATING, channel_state_e::GND},
                {channel_state_e::PWM, channel_state_e::GND, channel_state_e::FLOATING},
                {channel_state_e::FLOATING, channel_state_e::GND, channel_state_e::PWM}};
        ///     Variables
        channel_state_e last_[3]; // Saves the Last State of all three channels
    public:
        /// @brief Class-constructor
        /// @param bridge           Hardware implementation for timer register accesses
        explicit SixStep(Bridge &bridge)
            : bridge_{bridge},
              last_{
                  channel_state_e::UNINITIALISED,
                  channel_state_e::UNINITIALISED,
                  channel_state_e::UNINITIALISED}
        {
        }

        /// @brief Destructor
        ~SixStep() = default;

        /// @brief Return the next step depending on the direction and the Hall-readings
        /// @param hallsensoren    0x00 | Hall C | Hall B | Hall A
        /// @param dir             0 = counterclockwise;
        ///                        1 = clockwise
        /// @return                Next Step [1;6]
        static inline state_type decode(const uint8_t &hallsensoren, const bool &dir)
        {
            if ((hallsensoren < 1) || (hallsensoren > 6)) // Fehlerprüfung
            {
                return -1;
            }
            int index = lookup_table_id_[hallsensoren];
            return lookup_table_step_[dir][index]; // Nächsten Schritt zurückgeben
        }

        void init()
        {
            bridge_.init();
            for (int i = 0; i < 3; i++) // Alle Channel forcen damit der Grundzustand erreicht wird
            {
                bridge_.set_high_z((channel_e)i);
                last_[i] = channel_state_e::FLOATING;
            }
        }

        /// @brief Commutation step
        /// @note           When the new step is added depends
        ///                 on the implementation of the template class
        /// @param step     Next State for the three Phases.
        /// @return         0 = success;
        ///                 -1 = error
        int commutation_step(const step_e &step)
        {
            if (step > step_e::stop || step < step_e::step1)
            {
                return -1;
            }
            else if (step_e::idle == step || step_e::stop == step) // Currently no implementation for stop-case
            {
                for (int i = 0; i < 3; i++) // Alle Channel forcen damit der Grundzustand erreicht wird
                {
                    if (last_[i] == channel_state_e::FLOATING)
                    {
                        continue;
                    }
                    bridge_.set_high_z((channel_e)i);
                    last_[i] = channel_state_e::FLOATING;
                }
                return 0;
            }

            for (int i = 0; i < 3; i++)
            {
                int step_index = static_cast<int>(step);
                if (sequency[step_index][i] == last_[i]) // Überprüfen ob sich die konfiguration vom Channel geändert hat
                {
                    continue; // Sonst überspringen
                }
                last_[i] = sequency[step_index][i]; // Zustand für nächsten Schritt merken
                switch (sequency[step_index][i])
                {
                case channel_state_e::PWM:
                    bridge_.set_pwm((channel_e)i);
                    break;
                case channel_state_e::FLOATING:
                    bridge_.set_high_z((channel_e)i);
                    break;
                case channel_state_e::GND:
                    bridge_.set_gnd((channel_e)i);
                    break;
                }
            }
            return 0;
        }
        /// @brief Sets the Duty for all channels
        /// @param value    New duty in percent [0;100]
        void set_duty(const uint16_t &value)
        {
            bridge_.set_duty(value);
        }
    };

} // SixStep_NS

#endif
