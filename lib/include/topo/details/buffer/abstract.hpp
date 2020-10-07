#pragma once

namespace topo_details
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

} // namespace topo_details
