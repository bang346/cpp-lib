#ifndef CLARK_PARK_HPP
#define CLARK_PARK_HPP

/**
 * @file clark_park.hpp
 * @brief Clarke and Park transformations for FOC (Field-Oriented Control).
 *
 * Supports amplitude-invariant and power-invariant Clarke transforms,
 * as well as standard and alternative Park conventions.
 */

namespace clark_park
{
    // ============================================================
    // Clarke transform (amplitude-invariant)
    // ============================================================

    /// @brief Clarke transform (amplitude-invariant) using 2 current measurements (ia, ib).
    /// @details Assumes ic = -ia - ib.
    /// @param ia Phase A current.
    /// @param ib Phase B current.
    /// @param[out] alpha Alpha-axis component.
    /// @param[out] beta  Beta-axis component.
    void clarke_2shunt_amp(double ia, double ib, double &alpha, double &beta);

    /// @brief Clarke transform (amplitude-invariant) using 3 phase currents (ia, ib, ic).
    /// @details Valid when ia + ib + ic = 0.
    /// @param ia Phase A current.
    /// @param ib Phase B current.
    /// @param ic Phase C current.
    /// @param[out] alpha Alpha-axis component.
    /// @param[out] beta  Beta-axis component.
    void clarke_3shunt_amp(double ia, double ib, double ic, double &alpha, double &beta);

    // ============================================================
    // Clarke transform (power-invariant, 2/3 scaling)
    // ============================================================

    /// @brief Clarke transform (power-invariant, 2/3 scaling) using 3 phase currents (ia, ib, ic).
    /// @param ia Phase A current.
    /// @param ib Phase B current.
    /// @param ic Phase C current.
    /// @param[out] alpha Alpha-axis component.
    /// @param[out] beta  Beta-axis component.
    void clarke_3shunt_power(double ia, double ib, double ic, double &alpha, double &beta);

    /// @brief Clarke transform (power-invariant, 2/3 scaling) using 2 phase currents (ia, ib).
    /// @details Assumes ic = -ia - ib internally.
    /// @param ia Phase A current.
    /// @param ib Phase B current.
    /// @param[out] alpha Alpha-axis component.
    /// @param[out] beta  Beta-axis component.
    void clarke_2shunt_power(double ia, double ib, double &alpha, double &beta);

    // ============================================================
    // Inverse Clarke transform
    // ============================================================

    /// @brief Inverse Clarke transform (amplitude-invariant) from (alpha, beta) to (ia, ib, ic).
    /// @param alpha Alpha-axis component.
    /// @param beta  Beta-axis component.
    /// @param[out] ia Phase A current.
    /// @param[out] ib Phase B current.
    /// @param[out] ic Phase C current.
    void inv_clarke_amp(double alpha, double beta, double &ia, double &ib, double &ic);

    /// @brief Inverse Clarke transform (power-invariant usage).
    /// @details In practice often identical to amplitude-invariant inverse Clarke.
    /// @param alpha Alpha-axis component.
    /// @param beta  Beta-axis component.
    /// @param[out] ia Phase A current.
    /// @param[out] ib Phase B current.
    /// @param[out] ic Phase C current.
    void inv_clarke_power(double alpha, double beta, double &ia, double &ib, double &ic);

    // ============================================================
    // Park transform (standard convention)
    // ============================================================

    /// @brief Park transform (standard convention).
    /// @param alpha Alpha-axis component.
    /// @param beta  Beta-axis component.
    /// @param sin_theta Sine of electrical angle.
    /// @param cos_theta Cosine of electrical angle.
    /// @param[out] d D-axis component.
    /// @param[out] q Q-axis component.
    void park_std(double alpha, double beta,
                  double sin_theta, double cos_theta,
                  double &d, double &q);

    /// @brief Inverse Park transform (standard convention).
    /// @param d D-axis component.
    /// @param q Q-axis component.
    /// @param sin_theta Sine of electrical angle.
    /// @param cos_theta Cosine of electrical angle.
    /// @param[out] alpha Alpha-axis component.
    /// @param[out] beta  Beta-axis component.
    void inv_park_std(double d, double q,
                      double sin_theta, double cos_theta,
                      double &alpha, double &beta);

    // ============================================================
    // Park transform (alternative convention)
    // ============================================================

    /// @brief Park transform (alternative sign convention).
    /// @details Must be used consistently together with inv_park_alt().
    /// @param alpha Alpha-axis component.
    /// @param beta  Beta-axis component.
    /// @param sin_theta Sine of electrical angle.
    /// @param cos_theta Cosine of electrical angle.
    /// @param[out] d D-axis component.
    /// @param[out] q Q-axis component.
    void park_alt(double alpha, double beta,
                  double sin_theta, double cos_theta,
                  double &d, double &q);

    /// @brief Inverse Park transform for the alternative convention.
    /// @param d D-axis component.
    /// @param q Q-axis component.
    /// @param sin_theta Sine of electrical angle.
    /// @param cos_theta Cosine of electrical angle.
    /// @param[out] alpha Alpha-axis component.
    /// @param[out] beta  Beta-axis component.
    void inv_park_alt(double d, double q,
                      double sin_theta, double cos_theta,
                      double &alpha, double &beta);

    // ============================================================
    // FLOAT overloads
    // ============================================================

    void clarke_2shunt_amp(float ia, float ib, float &alpha, float &beta);
    void clarke_3shunt_amp(float ia, float ib, float ic, float &alpha, float &beta);
    void clarke_3shunt_power(float ia, float ib, float ic, float &alpha, float &beta);
    void clarke_2shunt_power(float ia, float ib, float &alpha, float &beta);

    void inv_clarke_amp(float alpha, float beta, float &ia, float &ib, float &ic);
    void inv_clarke_power(float alpha, float beta, float &ia, float &ib, float &ic);

    void park_std(float alpha, float beta,
                  float sin_theta, float cos_theta,
                  float &d, float &q);

    void inv_park_std(float d, float q,
                      float sin_theta, float cos_theta,
                      float &alpha, float &beta);

    void park_alt(float alpha, float beta,
                  float sin_theta, float cos_theta,
                  float &d, float &q);

    void inv_park_alt(float d, float q,
                      float sin_theta, float cos_theta,
                      float &alpha, float &beta);
}

#endif // CLARK_PARK_HPP
