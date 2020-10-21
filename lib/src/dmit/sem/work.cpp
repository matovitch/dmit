#include "dmit/sem/work.hpp"

namespace dmit::sem
{

void Work::run()
{
    _function();
    _pool.recycle(*this);
}

namespace work
{

void Pool::recycle(Work& work)
{
    _pool.recycle(work);
}

} // namespace work
} // namespace dmit::sem
