#pragma once

#include "dmit/wsm/leb128.hpp"

namespace dmit::wsm
{

template <class Writer>
struct TFixUpSize
{
    TFixUpSize(Writer& writer) : _writer{writer}
    {
        _fork = writer.fork();
        Leb128</*IS_OBJECT=*/true> size{0u};
        writer.write(size);
    }

    ~TFixUpSize()
    {
        Leb128</*IS_OBJECT=*/true> size{_writer.diff(_fork) - K_LEB128_OBJ_SIZE};
        _fork.write(size);
    }

    Writer& _writer;
    Writer  _fork;
};

} // namespace dmit::wsm
