#include <gtest/gtest.h>
#include <cmath>
#include "svpwm.hpp"

// Hilfsfunktion: Grad -> Rad
static double deg2rad(double deg)
{
    return deg * SVPWM_NS::PI / 180.0;
}

// Rekonstruiert Uout aus T1/T2 und den zwei aktiven Sektor-Vektoren (normiert)
static void reconstruct_uout(int sector, double T1, double T2, double Ts,
                             double &Ualpha_rec, double &Ubeta_rec)
{
    // sector 0..5
    const double base_deg = sector * 60.0; // 0,60,120...
    const double th1 = deg2rad(base_deg);
    const double th2 = deg2rad(base_deg + 60.0);

    const double Ux_a = std::cos(th1);
    const double Ux_b = std::sin(th1);

    const double Ux60_a = std::cos(th2);
    const double Ux60_b = std::sin(th2);

    // Uout = (T1/Ts)*Ux + (T2/Ts)*Ux+60
    const double k1 = T1 / Ts;
    const double k2 = T2 / Ts;

    Ualpha_rec = k1 * Ux_a + k2 * Ux60_a;
    Ubeta_rec = k1 * Ux_b + k2 * Ux60_b;
}

// Erwarteter geometrischer Sektor aus Winkel (0..360)
static int expected_sector_from_angle_deg(double ang_deg)
{
    // bring angle to [0..360)
    while (ang_deg < 0)
        ang_deg += 360.0;
    while (ang_deg >= 360.0)
        ang_deg -= 360.0;

    // sector 0: [0..60), 1:[60..120), ...
    return static_cast<int>(ang_deg / 60.0);
}

TEST(SVPWM, SectorDetection_AllSixSectors)
{
    const double Ts = 1.0;
    SVPWM sv(Ts);

    // Test jeweils in der Mitte jedes Sektors: 30°, 90°, 150°, ...
    const double mag = 0.4; // klein halten => kein Overmod
    for (int k = 0; k < 6; ++k)
    {
        const double ang_deg = k * 60.0 + 30.0;
        const double ang = deg2rad(ang_deg);

        const double Ualpha = mag * std::cos(ang);
        const double Ubeta = mag * std::sin(ang);

        auto P = sv.clc_periods(Ualpha, Ubeta);

        ASSERT_GE(P.sector, 0);
        ASSERT_LE(P.sector, 5);

        const int expected = expected_sector_from_angle_deg(ang_deg);
        EXPECT_EQ(P.sector, expected) << "Angle=" << ang_deg << " deg";
    }
}

TEST(SVPWM, Periods_ArePlausible)
{
    const double Ts = 1.0;
    SVPWM sv(Ts);

    // Test ein paar Winkel
    const double mag = 0.35;
    for (int i = 0; i < 360; i += 15)
    {
        const double ang = deg2rad(i);
        const double Ualpha = mag * std::cos(ang);
        const double Ubeta = mag * std::sin(ang);

        auto P = sv.clc_periods(Ualpha, Ubeta);

        // Nicht negativ
        EXPECT_GE(P.T1, 0.0);
        EXPECT_GE(P.T2, 0.0);
        EXPECT_GE(P.T0, 0.0);

        // Summe T1+T2+T0 ~= Ts (bei dir clamp möglich => daher toleranter)
        EXPECT_NEAR(P.T0 + P.T1 + P.T2, Ts, 1e-9);
    }
}

TEST(SVPWM, LUTInverse_ReconstructsUoutCorrectly)
{
    const double Ts = 1.0;
    SVPWM sv(Ts);

    // für jeden Sektor: Winkel in der Mitte, magnitude klein
    const double mag = 0.3;
    for (int k = 0; k < 6; ++k)
    {
        const double ang_deg = k * 60.0 + 20.0; // nicht genau 30°, damit nicht "symmetrisch trivial"
        const double ang = deg2rad(ang_deg);

        const double Ualpha = mag * std::cos(ang);
        const double Ubeta = mag * std::sin(ang);

        auto P = sv.clc_periods(Ualpha, Ubeta);

        // Rekonstruktion des Referenzvektors aus den 2 aktiven Vektoren
        double Ualpha_rec = 0.0, Ubeta_rec = 0.0;
        reconstruct_uout(P.sector, P.T1, P.T2, Ts, Ualpha_rec, Ubeta_rec);

        // sollte sehr nah am Original liegen
        EXPECT_NEAR(Ualpha_rec, Ualpha, 1e-9);
        EXPECT_NEAR(Ubeta_rec, Ubeta, 1e-9);
    }
}

TEST(SVPWM, Duty_IsInRange_0to1)
{
    const double Ts = 1.0;
    SVPWM sv(Ts);

    const double mag = 0.35;
    for (int i = 0; i < 360; i += 10)
    {
        const double ang = deg2rad(i);
        const double Ualpha = mag * std::cos(ang);
        const double Ubeta = mag * std::sin(ang);

        auto P = sv.clc_periods(Ualpha, Ubeta);
        auto D = sv.clc_duty(P);

        EXPECT_GE(D.da, 0.0);
        EXPECT_GE(D.db, 0.0);
        EXPECT_GE(D.dc, 0.0);

        EXPECT_LE(D.da, 1.0);
        EXPECT_LE(D.db, 1.0);
        EXPECT_LE(D.dc, 1.0);
    }
}
