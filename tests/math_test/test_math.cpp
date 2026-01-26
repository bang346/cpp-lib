#include <gtest/gtest.h>
#include <cmath>

#include "clark_park.hpp"

// =======================================================
// Helpers
// =======================================================
static constexpr double EPS = 1e-12;

static inline void expect_near(double a, double b, double eps = EPS)
{
    EXPECT_NEAR(a, b, eps);
}

// =======================================================
// Test: Clarke 2-shunt amplitude-invariant ↔ inverse Clarke (roundtrip abc)
// =======================================================
TEST(ClarkePark, Clarke2ShuntAmp_InvClarkeAmp_RoundtripABC)
{
    // Only ia and ib are measured -> assume ic = -(ia + ib)
    const double ia = 2.0;
    const double ib = -1.25;
    const double ic = -ia - ib;

    double alpha = 0.0, beta = 0.0;
    clark_park::clarke_2shunt_amp(ia, ib, alpha, beta);

    double ia_r = 0.0, ib_r = 0.0, ic_r = 0.0;
    clark_park::inv_clarke_amp(alpha, beta, ia_r, ib_r, ic_r);

    // Roundtrip must reproduce the original phase currents
    expect_near(ia_r, ia);
    expect_near(ib_r, ib);
    expect_near(ic_r, ic);
}

// =======================================================
// Test: Clarke 3-shunt amplitude-invariant ↔ inverse Clarke (roundtrip abc)
// =======================================================
TEST(ClarkePark, Clarke3ShuntAmp_InvClarkeAmp_RoundtripABC)
{
    // Balanced system: ia + ib + ic = 0
    const double ia = 1.1;
    const double ib = -0.4;
    const double ic = -ia - ib;

    double alpha = 0.0, beta = 0.0;
    clark_park::clarke_3shunt_amp(ia, ib, ic, alpha, beta);

    double ia_r = 0.0, ib_r = 0.0, ic_r = 0.0;
    clark_park::inv_clarke_amp(alpha, beta, ia_r, ib_r, ic_r);

    // Roundtrip must reproduce the original phase currents
    expect_near(ia_r, ia);
    expect_near(ib_r, ib);
    expect_near(ic_r, ic);
}

// =======================================================
// Test: Clarke 2-shunt power-invariant vs 3-shunt power-invariant consistency
// =======================================================
TEST(ClarkePark, ClarkePower_2Shunt_Equals_3Shunt)
{
    // Only ia and ib measured, ic reconstructed
    const double ia = 3.0;
    const double ib = -0.7;
    const double ic = -ia - ib;

    double a2 = 0.0, b2 = 0.0;
    double a3 = 0.0, b3 = 0.0;

    clark_park::clarke_2shunt_power(ia, ib, a2, b2);
    clark_park::clarke_3shunt_power(ia, ib, ic, a3, b3);

    // Both implementations must produce identical alpha/beta results
    expect_near(a2, a3);
    expect_near(b2, b3);
}

// =======================================================
// Test: Park standard ↔ inverse Park standard (roundtrip alpha/beta)
// =======================================================
TEST(ClarkePark, ParkStd_InvParkStd_RoundtripAlphaBeta)
{
    const double alpha = 1.234;
    const double beta  = -0.987;

    const double theta = 0.7; // rad
    const double sin_t = std::sin(theta);
    const double cos_t = std::cos(theta);

    double d = 0.0, q = 0.0;
    clark_park::park_std(alpha, beta, sin_t, cos_t, d, q);

    double alpha_r = 0.0, beta_r = 0.0;
    clark_park::inv_park_std(d, q, sin_t, cos_t, alpha_r, beta_r);

    // Roundtrip must reproduce the original alpha/beta components
    expect_near(alpha_r, alpha);
    expect_near(beta_r, beta);
}

// =======================================================
// Test: Park alternative ↔ inverse Park alternative (roundtrip alpha/beta)
// =======================================================
TEST(ClarkePark, ParkAlt_InvParkAlt_RoundtripAlphaBeta)
{
    const double alpha = -0.22;
    const double beta  = 4.5;

    const double theta = -1.3; // rad
    const double sin_t = std::sin(theta);
    const double cos_t = std::cos(theta);

    double d = 0.0, q = 0.0;
    clark_park::park_alt(alpha, beta, sin_t, cos_t, d, q);

    double alpha_r = 0.0, beta_r = 0.0;
    clark_park::inv_park_alt(d, q, sin_t, cos_t, alpha_r, beta_r);

    // Roundtrip must reproduce the original alpha/beta components
    expect_near(alpha_r, alpha);
    expect_near(beta_r, beta);
}

// =======================================================
// Test: Full roundtrip (Clarke -> Park -> InvPark -> InvClarke)
//       Using amplitude-invariant Clarke (2-shunt version)
// =======================================================
TEST(ClarkePark, FullRoundtrip_2ShuntAmp)
{
    const double ia = 1.0;
    const double ib = -2.0;
    const double ic = -ia - ib;

    const double theta = 1.1; // rad
    const double sin_t = std::sin(theta);
    const double cos_t = std::cos(theta);

    // Clarke transform (abc -> alpha/beta)
    double alpha = 0.0, beta = 0.0;
    clark_park::clarke_2shunt_amp(ia, ib, alpha, beta);

    // Park transform (alpha/beta -> d/q)
    double d = 0.0, q = 0.0;
    clark_park::park_std(alpha, beta, sin_t, cos_t, d, q);

    // Inverse Park transform (d/q -> alpha/beta)
    double alpha_r = 0.0, beta_r = 0.0;
    clark_park::inv_park_std(d, q, sin_t, cos_t, alpha_r, beta_r);

    // Inverse Clarke transform (alpha/beta -> abc)
    double ia_r = 0.0, ib_r = 0.0, ic_r = 0.0;
    clark_park::inv_clarke_amp(alpha_r, beta_r, ia_r, ib_r, ic_r);

    // Final result must match the original phase currents
    expect_near(ia_r, ia);
    expect_near(ib_r, ib);
    expect_near(ic_r, ic);
}
