#pragma once

#include "dmit/com/critical_section.hpp"
#include "dmit/com/singleton.hpp"

#include <iostream>
#include <atomic>
#include <thread>

namespace dmit::com
{

struct ThreadStream : Singletonable
{
    ThreadStream() : _os{std::cout} {}

    std::ostream& _os;
    std::atomic<bool> _flag;
};

template <class Type>
ThreadStream& operator<<(ThreadStream& ts, const Type& type)
{
    ts._os << type;
    return ts;
}

} // namespace dmit::com

#define DMIT_COM_THREAD_STREAM(x)                                                                \
{                                                                                                \
    DMIT_COM_CRITICAL_SECTION(dmit::com::TSingleton<dmit::com::ThreadStream>::instance()._flag); \
                              dmit::com::TSingleton<dmit::com::ThreadStream>::instance() << ' '  \
                              << std::this_thread::get_id() << ' ' << x << '\n';                 \
}
