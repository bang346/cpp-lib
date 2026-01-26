#ifndef SVPWM_HPP
#define SVPWM_HPP

namespace SVPWM_NS
{
    typedef double svpwm_t;
    static constexpr svpwm_t a = 0.577350269189626;
    static constexpr svpwm_t b = 1.154700538379252;

    // Lut saves the inverse matrizes from [Ux Ux+-60]-1
    static constexpr svpwm_t LUT[6][2][2] = {{{1, -a}, {0, b}},
                                             {{1, a}, {-1, a}},
                                             {{0, b}, {-1, -a}},
                                             {{-1, a}, {0, -b}},
                                             {{-1, -a}, {1, -a}},
                                             {{0, -b}, {1, a}}};

    struct Periods
    {
        int sector;
        svpwm_t T1, T2, T0;
    };
    struct Duty
    {
        svpwm_t da, db, dc;
    };
    static constexpr double PI = 3.14159265359;
    static constexpr svpwm_t sin60 = 0.8660254037844386;
    static constexpr svpwm_t sin30 = 0.5;

    static constexpr int Sector1 = 0;
    static constexpr int Sector2 = 1;
    static constexpr int Sector3 = 2;
    static constexpr int Sector4 = 3;
    static constexpr int Sector5 = 4;
    static constexpr int Sector6 = 5;

} // namespace SVPWM_NS

// ============================================================
//                          Sektormap
// ============================================================
/*
                           q-axis (β)
                                ↑
                                |
                    U120 (010)  |   U60 (110)
                         \      |      /
                          \     |     /
                           \    |    /
                            \   |   /
                             \  |  /
                              \ | /
                               \|/
             U180 (011) -------- O -------- U0 (100)     → d-axis (α)
                               /|\
                              / | \
                             /  |  \
                            /   |   \
                           /    |    \
                          /     |     \
                         /      |      \
                    U240 (001)  |   U300 (101)
                                |
                                ↓

Sector definition (each sector is between two neighboring active vectors):
Sector 1 : between U0   (100) and U60  (110)
Sector 2 : between U60  (110) and U120 (010)
Sector 3 : between U120 (010) and U180 (011)
Sector 4 : between U180 (011) and U240 (001)
Sector 5 : between U240 (001) and U300 (101)
Sector 6 : between U300 (101) and U0   (100)
*/

template <typename T>
int sgn(T val)
{
    return (T(0) < val) ? 1 : 0;
}

class SVPWM
{
private:
    const SVPWM_NS::svpwm_t Tpwm_; // Periode PWM
    /// @see Application Report SPRA524 p.8
    static constexpr int N_to_Sector[6] = {1, 5, 0, 3, 2, 4};

public:
    SVPWM(const SVPWM_NS::svpwm_t Tpwm) : Tpwm_(Tpwm) {}
    ~SVPWM() = default;

    /// @brief Caclulates the Periods for the algorithm
    /// @param Ualpha   Stationary voltage alpha
    /// @param Ubetha   Stationary voltage alpha
    /// @return         T1, T2 T0
    SVPWM_NS::Periods clc_periods(const SVPWM_NS::svpwm_t &Ualpha, const SVPWM_NS::svpwm_t &Ubeta);

    /// @brief
    /// @param Periods
    /// @return Duty cylcles for Phase A, B and C
    SVPWM_NS::Duty clc_duty(const SVPWM_NS::Periods &Periods);
};

#endif