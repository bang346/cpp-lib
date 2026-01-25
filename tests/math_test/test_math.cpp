#include <gtest/gtest.h>
#include <cmath>

// Include deine Header-Datei mit den Funktionen
// z.B. #include "clark_park.hpp"
#include "clark_park.hpp"

// =======================================================
// Helper
// =======================================================
static constexpr double EPS = 1e-12;

static inline bool almost_equal(double a, double b, double eps = EPS)
{
    return std::abs(a - b) <= eps;
}

static inline void expect_near(double a, double b, double eps = EPS)
{
    EXPECT_NEAR(a, b, eps);
}

// =======================================================
// Test: Clarke 2-Shunt Amplitude ↔ Inverse Clarke
// =======================================================
TEST(ClarkePark, Clarke2ShuntAmp_InvClarkeAmp_RoundtripABC)
{
    // nur ia, ib gemessen -> ic = -(ia+ib)
    const double ia = 2.0;
    const double ib = -1.25;
    const double ic = -ia - ib;

    double alpha = 0.0, beta = 0.0;
    clarke_2shunt_amp(ia, ib, alpha, beta);

    double ia_r = 0.0, ib_r = 0.0, ic_r = 0.0;
    inv_clarke_amp(alpha, beta, ia_r, ib_r, ic_r);

    // roundtrip muss wieder die gleichen Ströme ergeben
    expect_near(ia_r, ia);
    expect_near(ib_r, ib);
    expect_near(ic_r, ic);
}

// =======================================================
// Test: Clarke 3-Shunt Amplitude ↔ Inverse Clarke
// =======================================================
TEST(ClarkePark, Clarke3ShuntAmp_InvClarkeAmp_RoundtripABC)
{
    const double ia = 1.1;
    const double ib = -0.4;
    const double ic = -ia - ib;

    double alpha = 0.0, beta = 0.0;
    clarke_3shunt_amp(ia, ib, ic, alpha, beta);

    double ia_r = 0.0, ib_r = 0.0, ic_r = 0.0;
    inv_clarke_amp(alpha, beta, ia_r, ib_r, ic_r);

    expect_near(ia_r, ia);
    expect_near(ib_r, ib);
    expect_near(ic_r, ic);
}

// =======================================================
// Test: Clarke 2-Shunt Power vs 3-Shunt Power Konsistenz
// =======================================================
TEST(ClarkePark, ClarkePower_2Shunt_Equals_3Shunt)
{
    const double ia = 3.0;
    const double ib = -0.7;
    const double ic = -ia - ib;

    double a2 = 0.0, b2 = 0.0;
    double a3 = 0.0, b3 = 0.0;

    clarke_2shunt_power(ia, ib, a2, b2);
    clarke_3shunt_power(ia, ib, ic, a3, b3);

    expect_near(a2, a3);
    expect_near(b2, b3);
}

// =======================================================
// Test: Park Standard ↔ Inv Park Standard
// =======================================================
TEST(ClarkePark, ParkStd_InvParkStd_RoundtripAlphaBeta)
{
    const double alpha = 1.234;
    const double beta = -0.987;

    const double theta = 0.7; // rad
    const double sin_t = std::sin(theta);
    const double cos_t = std::cos(theta);

    double d = 0.0, q = 0.0;
    park_std(alpha, beta, sin_t, cos_t, d, q);

    double alpha_r = 0.0, beta_r = 0.0;
    inv_park_std(d, q, sin_t, cos_t, alpha_r, beta_r);

    expect_near(alpha_r, alpha);
    expect_near(beta_r, beta);
}

// =======================================================
// Test: Park Alternative ↔ Inv Park Alternative
// =======================================================
TEST(ClarkePark, ParkAlt_InvParkAlt_RoundtripAlphaBeta)
{
    const double alpha = -0.22;
    const double beta = 4.5;

    const double theta = -1.3; // rad
    const double sin_t = std::sin(theta);
    const double cos_t = std::cos(theta);

    double d = 0.0, q = 0.0;
    park_alt(alpha, beta, sin_t, cos_t, d, q);

    double alpha_r = 0.0, beta_r = 0.0;
    inv_park_alt(d, q, sin_t, cos_t, alpha_r, beta_r);

    expect_near(alpha_r, alpha);
    expect_near(beta_r, beta);
}

// =======================================================
// Test: Gesamt Roundtrip (Clarke + Park + InvPark + InvClarke)
//      Amp-Invariant 2-Shunt
// =======================================================
TEST(ClarkePark, FullRoundtrip_2ShuntAmp)
{
    const double ia = 1.0;
    const double ib = -2.0;
    const double ic = -ia - ib;

    const double theta = 1.1;
    const double sin_t = std::sin(theta);
    const double cos_t = std::cos(theta);

    double alpha = 0.0, beta = 0.0;
    clarke_2shunt_amp(ia, ib, alpha, beta);

    double d = 0.0, q = 0.0;
    park_std(alpha, beta, sin_t, cos_t, d, q);

    double alpha_r = 0.0, beta_r = 0.0;
    inv_park_std(d, q, sin_t, cos_t, alpha_r, beta_r);

    double ia_r = 0.0, ib_r = 0.0, ic_r = 0.0;
    inv_clarke_amp(alpha_r, beta_r, ia_r, ib_r, ic_r);

    expect_near(ia_r, ia);
    expect_near(ib_r, ib);
    expect_near(ic_r, ic);
}
