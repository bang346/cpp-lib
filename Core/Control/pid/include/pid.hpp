#ifndef PID_HPP
#define PID_HPP

/// @brief Template PID-Controller
/// @details    Time diskrete PID controller direcetly with
///             the tunstin approximation
///             u[k] = u[k-1]+b0*e[k]+b1*e[k-1]+b2*e[k-2]
/// @warning    The D-component is directly derived, so
///             it will not work with Kd set correctly!
/// @tparam     T value type
/// @note       default double
template <typename T = double>
class PID
{
private:
    T const b0_, b1_, b2_, TA_;
    T u_k0_, u_k1_;
    T e_k1_, e_k2_;

public:
    /// @brief Initialization
    /// @param Kp Proportional Part
    /// @param Ki Integral Part
    /// @param Kd Differentiating Part
    /// @param TA Sample time
    PID(const T Kp, const T Ki, const T Kd, const T TA)
        : b0_(Kp + Ki * TA / 2 + 2 * Kd / TA),
          b1_(-Kp + Ki * TA / 2 - 4 * Kd / TA),
          b2_(2 * Kd / TA),
          TA_(TA),
          u_k0_(0),
          u_k1_(0),
          e_k1_(0),
          e_k2_(0)
    {
    }

    ~PID() = default;

    T update(const T e)
    {

        u_k0_ = u_k1_ + b0_ * e + b1_ * e_k1_ + b2_ * e_k2_;

        // Shift values back
        u_k1_ = u_k0_;
        e_k2_ = e_k1_;
        e_k1_ = e;

        return u_k0_;
    }
};

template <typename T = double>
class PIDF
{
private:
    const T i_coeff_, d1_coeff_, d2_coeff_;
    T e_k1_;
    T d_k1, i_k1_;
    const T Kp_;
    const T u_max_, u_min_;

public:
    // /// @brief Initialization
    // /// @param Kp Proportional Part
    // /// @param Ki Integral Part
    // /// @param Kd Differentiating Part
    // /// @param TA Sample time
    // /// @param TF Filter coefficient (TF = 10*Ts)
    // PIDF(const T Kp, const T Ki, const T Kd, const T TA, const T TF)
    //     : i_coeff_((Ki * TA) / 2),
    //       d1_coeff_((2 * TF - TA) / (2 * TF + TA)),
    //       d2_coeff_((2 * Kd) / (2 * TF + TA)),
    //       e_k1_(0),
    //       d_k1(0),
    //       i_k1_(0),
    //       Kp_(Kp)
    // {
    // }

    /// @brief Initialization
    /// @param Kp       Proportional Part
    /// @param Ki       Integral Part
    /// @param Kd       Differentiating Part
    /// @param TA       Sample time
    /// @param TF       Filter coefficient (TF = 10*Ts)
    /// @param u_min    Lower saturation limit
    /// @param u_max    Lower saturation limit
    PIDF(const T Kp, const T Ki, const T Kd, const T TA, const T TF, const T u_max, const T u_min)
        : i_coeff_((Ki * TA) / 2),
          d1_coeff_((2 * TF - TA) / (2 * TF + TA)),
          d2_coeff_((2 * Kd) / (2 * TF + TA)),
          e_k1_(0),
          d_k1(0),
          i_k1_(0),
          Kp_(Kp),
          u_max_(u_max),
          u_min_(u_min)
    {
    }
    ~PIDF() = default;

    // T update(const T e)
    // {
    //     i_k1_ = i_k1_ + i_coeff_ * (e + e_k1_);
    //     d_k1 = d1_coeff_ * d_k1 + d2_coeff_ * (e - e_k1_);

    //     e_k1_ = e;

    //     return Kp_ * e + i_k1_ + d_k1;
    // }
    T update(const T e)
    {
        d_k1 = d1_coeff_ * d_k1 + d2_coeff_ * (e - e_k1_);
        const T i_candidate = i_k1_ + i_coeff_ * (e + e_k1_);

        const T u_raw = Kp_ * e + i_candidate + d_k1;
        T u_sat = u_raw;
        if (u_raw > u_max_)
        {
            u_sat = u_max_;
        }
        else if (u_raw < u_min_)
        {
            u_sat = u_min_;
        }

        // Clamping: I only update, if
        // - not saturating
        // - or error pulls out of saturation
        const bool saturated_high = (u_raw > u_max_);
        const bool saturated_low = (u_raw < u_min_);

        if ((!saturated_high && !saturated_low) ||
            (saturated_high && e < 0) ||
            (saturated_low && e > 0))
        {
            i_k1_ = i_candidate;
        }
        e_k1_ = e;

        return u_sat;
    }
};

#endif