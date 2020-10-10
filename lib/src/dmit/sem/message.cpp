#include "dmit/sem/message.hpp"

namespace dmit::sem::message
{

void Abstract::send()
{
    _refCount++;
}

void Abstract::retrieve()
{
    _refCount--;
}

bool Abstract::isValid() const
{
    return _refCount != 0;
}

} // namespace dmit::sem::message
