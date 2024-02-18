#include "dmit/com/clock.hpp"

#include <iostream>
#include <chrono>

namespace
{
    void displayDuration(const std::chrono::nanoseconds duration)
    {
        std::cout << std::setw(3) << std::chrono::duration_cast<std::chrono::seconds     >(duration).count()        << "s "
                  << std::setw(3) << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000 << "m "
                  << std::setw(3) << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000 << "u ";
    }
}

namespace dmit::com
{

void Clock::display()
{
    auto diff = std::chrono::steady_clock::now() - _reference;

    _total += diff;

    if (_order > 9999)
    {
        std::cout << "0000CLK0 Too many events!\n";
        std::cout << "9999CLK9 Too many events!\n";
    }

    displayHeader();
    displayDuration(diff);

    std::cout << std::right << std::setw(55 - 5 * _LEVEL) << _name << '\n';
}

Clock::~Clock()
{
    display();

    if (_isFull)
    {
        displayHeader();

        for (int i = 0; i < 14; i++)
        {
            std::cout << '-';
        }

        std::cout << '\n';

        displayHeader();
        displayDuration(_total);

        std::cout << '\n';

        if (!_LEVEL)
        {
            displayHeader();
            std::cout << "======================================================================\n";
        }
    }

    _ORDER--;
    _LEVEL--;
}

int Clock::_LEVEL = -1;
int Clock::_ORDER = 0;

} // namespace dmit::com
