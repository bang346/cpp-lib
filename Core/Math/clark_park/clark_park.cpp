#include "clark_park.hpp"
#include <cmath>

namespace
{
    // Basic constants
    constexpr double one_over_2 = 0.5;
    constexpr double minus_one_over_2 = -0.5;

    constexpr double two_over_3 = 0.66666666666666666666666666666667; // 2/3

    // √3 related constants (fixed values, no sqrt at runtime)
    constexpr double sqrt3_over_2 = 0.86602540378443864676372317075294;   // √3/2
    constexpr double one_over_sqrt3 = 0.57735026918962576450914878050196; // 1/√3
}

namespace clark_park
{
    // ============================================================
    // Clarke (amplitude-invariant)
    // ============================================================

    void clarke_2shunt_amp(double ia, double ib, double &alpha, double &beta)
    {
        alpha = ia;
        beta  = (ia + 2.0 * ib) * one_over_sqrt3;
    }

    void clarke_3shunt_amp(double ia, double ib, double ic, double &alpha, double &beta)
    {
        (void)ib;
        alpha = ia;
        beta  = (ib - ic) * one_over_sqrt3;
    }

    // ============================================================
    // Clarke (power-invariant, 2/3 scaling)
    // ============================================================

    void clarke_3shunt_power(double ia, double ib, double ic, double &alpha, double &beta)
    {
        alpha = two_over_3 * (ia + minus_one_over_2 * ib + minus_one_over_2 * ic);
        beta  = two_over_3 * (sqrt3_over_2 * (ib - ic));
    }

    void clarke_2shunt_power(double ia, double ib, double &alpha, double &beta)
    {
        const double ic = -ia - ib;
        clarke_3shunt_power(ia, ib, ic, alpha, beta);
    }

    // ============================================================
    // Inverse Clarke
    // ============================================================

    void inv_clarke_amp(double alpha, double beta, double &ia, double &ib, double &ic)
    {
        ia = alpha;
        ib = minus_one_over_2 * alpha + sqrt3_over_2 * beta;
        ic = minus_one_over_2 * alpha - sqrt3_over_2 * beta;
    }

    void inv_clarke_power(double alpha, double beta, double &ia, double &ib, double &ic)
    {
        inv_clarke_amp(alpha, beta, ia, ib, ic);
    }

    // ============================================================
    // Park (standard)
    // ============================================================

    void park_std(double alpha, double beta,
                  double sin_theta, double cos_theta,
                  double &d, double &q)
    {
        d =  cos_theta * alpha + sin_theta * beta;
        q = -sin_theta * alpha + cos_theta * beta;
    }

    void inv_park_std(double d, double q,
                      double sin_theta, double cos_theta,
                      double &alpha, double &beta)
    {
        alpha = cos_theta * d - sin_theta * q;
        beta  = sin_theta * d + cos_theta * q;
    }

    // ============================================================
    // Park (alternative convention)
    // ============================================================

    void park_alt(double alpha, double beta,
                  double sin_theta, double cos_theta,
                  double &d, double &q)
    {
        d = cos_theta * alpha - sin_theta * beta;
        q = sin_theta * alpha + cos_theta * beta;
    }

    void inv_park_alt(double d, double q,
                      double sin_theta, double cos_theta,
                      double &alpha, double &beta)
    {
        alpha =  cos_theta * d + sin_theta * q;
        beta  = -sin_theta * d + cos_theta * q;
    }

    // ============================================================
    // FLOAT overloads
    // ============================================================

    void clarke_2shunt_amp(float ia, float ib, float &alpha, float &beta)
    {
        alpha = ia;
        beta  = (ia + 2.0f * ib) * static_cast<float>(one_over_sqrt3);
    }

    void clarke_3shunt_amp(float ia, float ib, float ic, float &alpha, float &beta)
    {
        (void)ib;
        alpha = ia;
        beta  = (ib - ic) * static_cast<float>(one_over_sqrt3);
    }

    void clarke_3shunt_power(float ia, float ib, float ic, float &alpha, float &beta)
    {
        alpha = static_cast<float>(two_over_3) *
                (ia + static_cast<float>(minus_one_over_2) * ib + static_cast<float>(minus_one_over_2) * ic);

        beta  = static_cast<float>(two_over_3) *
                (static_cast<float>(sqrt3_over_2) * (ib - ic));
    }

    void clarke_2shunt_power(float ia, float ib, float &alpha, float &beta)
    {
        const float ic = -ia - ib;
        clarke_3shunt_power(ia, ib, ic, alpha, beta);
    }

    void inv_clarke_amp(float alpha, float beta, float &ia, float &ib, float &ic)
    {
        ia = alpha;
        ib = static_cast<float>(minus_one_over_2) * alpha + static_cast<float>(sqrt3_over_2) * beta;
        ic = static_cast<float>(minus_one_over_2) * alpha - static_cast<float>(sqrt3_over_2) * beta;
    }

    void inv_clarke_power(float alpha, float beta, float &ia, float &ib, float &ic)
    {
        inv_clarke_amp(alpha, beta, ia, ib, ic);
    }

    void park_std(float alpha, float beta,
                  float sin_theta, float cos_theta,
                  float &d, float &q)
    {
        d =  cos_theta * alpha + sin_theta * beta;
        q = -sin_theta * alpha + cos_theta * beta;
    }

    void inv_park_std(float d, float q,
                      float sin_theta, float cos_theta,
                      float &alpha, float &beta)
    {
        alpha = cos_theta * d - sin_theta * q;
        beta  = sin_theta * d + cos_theta * q;
    }

    void park_alt(float alpha, float beta,
                  float sin_theta, float cos_theta,
                  float &d, float &q)
    {
        d = cos_theta * alpha - sin_theta * beta;
        q = sin_theta * alpha + cos_theta * beta;
    }

    void inv_park_alt(float d, float q,
                      float sin_theta, float cos_theta,
                      float &alpha, float &beta)
    {
        alpha = cos_theta * d + sin_theta * q;
        beta  = -sin_theta * d + cos_theta * q;
    }
}
