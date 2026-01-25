#ifndef CLARK_PARK
#define CLARK_PARK
#include <cmath>

namespace
{
    // Basis
    constexpr float one_over_2 = 0.5;
    constexpr float minus_one_over_2 = -0.5;

    constexpr float one_over_3 = 0.33333333333333333333333333333333; // 1/3
    constexpr float two_over_3 = 0.66666666666666666666666666666667; // 2/3

    // √3 Werte (fest, ohne sqrt)
    constexpr float sqrt3 = 1.73205080756887729352744634150587;          // √3
    constexpr float sqrt3_over_2 = 0.86602540378443864676372317075294;   // √3/2
    constexpr float one_over_sqrt3 = 0.57735026918962576450914878050196; // 1/√3
}

// ============================================================
// 1) CLARKE (Amplitude-invariant) - 2 Strommessungen (ia, ib)
//    Annahme: ic = -ia - ib
//    alpha = ia
//    beta  = (ia + 2*ib)/√3
// ============================================================
inline void clarke_2shunt_amp(float ia, float ib, float &alpha, float &beta)
{
    alpha = ia;
    beta = (ia + 2.0 * ib) * one_over_sqrt3;
}

// ============================================================
// 2) CLARKE (Amplitude-invariant) - mit 3 Phasen (ia, ib, ic)
//    alpha = ia
//    beta  = (ib - ic)/√3      (wenn ia+ib+ic=0)
// ============================================================
inline void clarke_3shunt_amp(float ia, float ib, float ic, float &alpha, float &beta)
{
    (void)ib; // optional, falls du nur ic nutzt
    alpha = ia;
    beta = (ib - ic) * one_over_sqrt3;
}

// ============================================================
// 3) CLARKE (Power-invariant, "2/3-Variante") - 3 Phasen (ia,ib,ic)
//    alpha = 2/3 * (ia - 1/2*ib - 1/2*ic)
//    beta  = 2/3 * (√3/2 * (ib - ic))
// ============================================================
inline void clarke_3shunt_power(float ia, float ib, float ic, float &alpha, float &beta)
{
    alpha = two_over_3 * (ia + minus_one_over_2 * ib + minus_one_over_2 * ic);
    beta = two_over_3 * (sqrt3_over_2 * (ib - ic));
}

// ============================================================
// 4) CLARKE (Power-invariant, "2/3-Variante") - nur 2 Messungen (ia, ib)
//    ic = -ia - ib eingesetzt => gleiche Normierung wie oben
// ============================================================
inline void clarke_2shunt_power(float ia, float ib, float &alpha, float &beta)
{
    const float ic = -ia - ib;
    clarke_3shunt_power(ia, ib, ic, alpha, beta);
}

// ============================================================
// 5) INVERSE CLARKE (Amplitude-invariant) -> (a,b,c)
//    a = alpha
//    b = -1/2*alpha + √3/2*beta
//    c = -1/2*alpha - √3/2*beta
// ============================================================
inline void inv_clarke_amp(float alpha, float beta, float &ia, float &ib, float &ic)
{
    ia = alpha;
    ib = minus_one_over_2 * alpha + sqrt3_over_2 * beta;
    ic = minus_one_over_2 * alpha - sqrt3_over_2 * beta;
}

// ============================================================
// 6) INVERSE CLARKE (Power-invariant) -> (a,b,c)
//    Gleiches Rückrechnen wie oben, ABER wenn du power-invariant
//    nutzt, musst du die Normierung konsistent halten.
//    (Meistens nimmt man trotzdem genau diese Rückformel.)
// ============================================================
inline void inv_clarke_power(float alpha, float beta, float &ia, float &ib, float &ic)
{
    // identisch in der Praxis verwendet:
    inv_clarke_amp(alpha, beta, ia, ib, ic);
}

// ============================================================
// 7) PARK (Standard, FOC üblich)
//    d =  cos*alpha + sin*beta
//    q = -sin*alpha + cos*beta
// ============================================================
inline void park_std(float alpha, float beta,
                     float sin_theta, float cos_theta,
                     float &d, float &q)
{
    d = cos_theta * alpha + sin_theta * beta;
    q = -sin_theta * alpha + cos_theta * beta;
}

// ============================================================
// 8) INVERSE PARK (Standard)
//    alpha = cos*d - sin*q
//    beta  = sin*d + cos*q
// ============================================================
inline void inv_park_std(float d, float q,
                         float sin_theta, float cos_theta,
                         float &alpha, float &beta)
{
    alpha = cos_theta * d - sin_theta * q;
    beta = sin_theta * d + cos_theta * q;
}

// ============================================================
// 9) PARK (Alternative Konvention - Vorzeichen anders)
//    d =  cos*alpha - sin*beta
//    q =  sin*alpha + cos*beta
//    (Wird manchmal benutzt -> dann muss inverse auch dazu passen!)
// ============================================================
inline void park_alt(float alpha, float beta,
                     float sin_theta, float cos_theta,
                     float &d, float &q)
{
    d = cos_theta * alpha - sin_theta * beta;
    q = sin_theta * alpha + cos_theta * beta;
}

// ============================================================
// 10) INVERSE PARK (Alternative Konvention passend zu park_alt)
// ============================================================
inline void inv_park_alt(float d, float q,
                         float sin_theta, float cos_theta,
                         float &alpha, float &beta)
{
    alpha = cos_theta * d + sin_theta * q;
    beta = -sin_theta * d + cos_theta * q;
}

#endif