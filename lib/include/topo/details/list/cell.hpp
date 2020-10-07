#pragma once

namespace topo_details
{

namespace list
{

template <class CellType>
struct TCell
{
    using Type = CellType;

    template <class... Args>
    TCell(TCell* tail, Args&&... args) :
        value{args...},
        _next{tail}
    {
        if (_next)
        {
            _next->_prev = this;
        }
    }

    Type value;

    TCell* _next = nullptr;
    TCell* _prev = nullptr;
};

} // namespace list

} // namespace topo_details
