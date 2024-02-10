#include <iostream>
#include <iomanip>
#include <chrono>

namespace dmit::com
{

namespace
{
    void displayDuration(const std::chrono::nanoseconds duration)
    {
        std::cout << std::setw(3) << std::chrono::duration_cast<std::chrono::seconds     >(duration).count()        << "s "
                  << std::setw(3) << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000 << "m "
                  << std::setw(3) << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000 << "u ";
    }
}

struct Clock
{
    Clock(const std::string_view name, bool isFull = false) :
        _reference{std::chrono::steady_clock::now()},
        _name{name},
        _order{_ORDER++},
        _total{0},
        _isFull{isFull}
    {
        _LEVEL++;
    }

    void displayHeader()
    {
        std::cout << std::setfill('0') << std::setw(4) << _order << std::setfill(' ') << "CLK" << _LEVEL << ' ';

        for (int i = 0; i < _LEVEL; i++)
        {
            std::cout << "     ";
        }

        _order = _ORDER++;
    }

    void display()
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

        std::cout << std::right << std::setw(50 - 5 * _LEVEL) << _name << '\n';
    }

    void operator()(const std::string_view name)
    {
        display();
        _name = name;
        _reference = std::chrono::steady_clock::now();
    }

    ~Clock()
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
        }

        _LEVEL--;
    }

    std::chrono::time_point<std::chrono::steady_clock> _reference;
    std::string_view _name;
    int _order;
    std::chrono::nanoseconds _total;
    bool _isFull;
    static int _LEVEL;
    static int _ORDER;
};

} // namespace dmit::com

#define DMIT_COM_CLK_MAKE_FULL(name) dmit::com::Clock dmitComClock{#name, true /*print total*/}
#define DMIT_COM_CLK_MAKE(name)      dmit::com::Clock dmitComClock{#name}
#define DMIT_COM_CLK(name)                            dmitComClock(#name)
