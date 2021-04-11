#pragma once

#include <atomic>

namespace dmit::com
{

class Mutex
{

public:

    void   lock();
    void unlock();

private:

    std::atomic<bool> flag = false;
};

} // namespace dmit::com
