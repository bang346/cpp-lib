#ifndef PID_HPP
#define PID_HPP

/// @brief Template PID-Controller
/// @tparam T value type
/// @note   default double
template <typename T = double>
class PID
{
private:
    T const b0_, b1_, b2_, TA_;
    T u_k0_, u_k1_;
    T e_k1_, e_k2_;

public:
    PID(const T Kp, const T Ki, const T Kd, const T TA)
        : b0_(Kp + Ki * TA / 2 + 2 * Kd / TA),
          b1_(Ki * TA - 4 * Kd / TA),
          b2_(-Kp + Ki * TA / 2 + 2 * Kd / TA),
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
        // PID_Part
        u_k0_ = u_k1_ + b0_ * e + b1_ * e_k1_ + b2_ * e_k2_;

        // Shift values back
        u_k1_ = u_k0_;
        e_k2_ = e_k1_;
        e_k1_ = e;

        return u_k0_;
    }
};

#endif