#pragma once

namespace pool_details
{

namespace buffer
{

template <class Type>
struct TAbstract
{
    virtual Type* allocate() = 0;

    virtual ~TAbstract() {}
};

} // namespace buffer

} // namespace pool_details
