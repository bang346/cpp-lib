#ifndef CNTR_SYSTEM_HPP
#define CNTR_SYSTEM_HPP

template <typename U = double>
class cntr_system
{
private:
    const U b0_, b1_, a1_;
    U u_k1_;
    U y_k1_;

public:
    cntr_system::cntr_system(const U K, const U T, const U TA)
        : b0_(K / (1 + (2 * T) / TA)),
          b1_(b0_),
          a1_((TA - 2 * T) / (TA + 2 * T)),
          u_k1_(0),
          y_k1_(0)
    {
    }

    ~cntr_system() = default;

    U update(const U u_k0)
    {

        U y_k0_ = -a1_ * y_k1_ + b0_ * u_k0 + b1_ * u_k1_;

        u_k1_ = u_k0;
        y_k1_ = y_k0_;
        return y_k0_;
    }
};

#endif