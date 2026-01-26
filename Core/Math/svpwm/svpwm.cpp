#include <math.h>
#include "svpwm.hpp"

SVPWM_NS::Periods SVPWM::clc_periods(const SVPWM_NS::svpwm_t &Ualpha, const SVPWM_NS::svpwm_t &Ubeta)
{
    // 1. Evaluate sektor
    const SVPWM_NS::svpwm_t Vref1 = Ubeta;
    const SVPWM_NS::svpwm_t Vref2 = SVPWM_NS::sin60 * Ualpha - SVPWM_NS::sin30 * Ubeta;
    const SVPWM_NS::svpwm_t Vref3 = -SVPWM_NS::sin60 * Ualpha - SVPWM_NS::sin30 * Ubeta;

    // Formula minus one because array index starts at 0
    int N = (sgn<double>(Vref1) + 2 * sgn<double>(Vref2) + 4 * sgn<double>(Vref3)) - 1;

    // For safety
    if (N < 0)
        N = 0;
    if (N > 5)
        N = 5;
    SVPWM_NS::Periods Periods;
    Periods.sector = N_to_Sector[N];

    // 2. Calculate Periods with LUT
    Periods.T1 = Tpwm_ * SVPWM_NS::LUT[N_to_Sector[N]][0][0] * Ualpha + Tpwm_ * SVPWM_NS::LUT[N_to_Sector[N]][0][1] * Ubeta;
    Periods.T2 = Tpwm_ * SVPWM_NS::LUT[N_to_Sector[N]][1][0] * Ualpha + Tpwm_ * SVPWM_NS::LUT[N_to_Sector[N]][1][1] * Ubeta;
    Periods.T0 = Tpwm_ - Periods.T1 - Periods.T2;

    return Periods;
}

SVPWM_NS::Duty SVPWM::clc_duty(const SVPWM_NS::Periods &Periods)
{
    SVPWM_NS::Duty Duty;

    const double half_T0 = Periods.T0 / 2.0;
    switch (Periods.sector)
    {
    // V0 => V1 => V2 => V7 => V2 => V1
    case SVPWM_NS::Sector1:
        Duty.da = Periods.T1 + Periods.T2 + half_T0;
        Duty.db = Periods.T2 + half_T0;
        Duty.dc = half_T0;
        break;
    case SVPWM_NS::Sector2:
        Duty.da = Periods.T1 + half_T0;
        Duty.db = Periods.T1 + Periods.T2 + half_T0;
        Duty.dc = half_T0;
        break;
    case SVPWM_NS::Sector3:
        Duty.da = half_T0;
        Duty.db = Periods.T1 + Periods.T2 + half_T0;
        Duty.dc = Periods.T2 + half_T0;
        break;
    case SVPWM_NS::Sector4:
        Duty.da = half_T0;
        Duty.db = Periods.T1 + half_T0;
        Duty.dc = Periods.T1 + Periods.T2 + half_T0;
        break;
    case SVPWM_NS::Sector5:
        Duty.da = Periods.T2 + half_T0;
        Duty.db = half_T0;
        Duty.dc = Periods.T1 + Periods.T2 + half_T0;
        break;
    case SVPWM_NS::Sector6:
        Duty.da = Periods.T1 + Periods.T2 + half_T0;
        Duty.db = half_T0;
        Duty.dc = Periods.T1 + half_T0;
        break;
    }
    Duty.da /= Tpwm_;
    Duty.db /= Tpwm_;
    Duty.dc /= Tpwm_;
    return Duty;
}