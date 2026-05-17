#include <gtest/gtest.h>
#include <cmath>
#include <iostream>

#include "prittyprinting.hpp"


template<int Port, int Pin>
struct CsPin {
    static void low()  { std::cout << GREEN << "LOW\n";}
    static void high() { std::cout << GREEN << "HIGH\n";}
};

class DRV8353 {
public:
    template<typename Cs>
    DRV8353(Cs)
    {
        setCsLow = &Cs::low;
        setCsHigh = &Cs::high;
    }
    void writeReg(uint16_t data) {
        setCsLow();
        setCsHigh();
    }
private:
    void (*setCsLow)();   // Zeiger auf Funktion Cs::low
    void (*setCsHigh)();  // Zeiger auf Funktion Cs::high
};



TEST(SPI, Example)
{
    std::cout << GREEN << "START" << std::endl;
using MyCs = CsPin<5, 4>;

DRV8353 drv(MyCs{});
drv.writeReg(0x00);


}
