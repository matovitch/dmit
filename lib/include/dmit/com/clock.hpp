#include <iostream>
#include <iomanip>
#include <chrono>

namespace dmit::com
{

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

    void display();

    void operator()(const std::string_view name)
    {
        display();
        _name = name;
        _reference = std::chrono::steady_clock::now();
    }

    ~Clock();

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
