#pragma once

#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/bundle.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/storage.hpp"

#include <cstdint>

namespace dmit::gen
{

using PoolWasm = dmit::wsm::node::TPool<0xC>;

com::TStorage<uint8_t> make(ast::Bundle& bundle,
                            PoolWasm& poolWasm);
struct Emitter
{
    using ReturnType = com::TStorage<uint8_t>;

    Emitter(com::TStorage<ast::Bundle>& bundles) :
        _bundles{bundles}
    {}

    com::TStorage<uint8_t> run(const uint64_t index)
    {
        return gen::make(_bundles[index],
                         _poolWasm);
    }

    uint32_t size() const
    {
        return _bundles._size;
    }

    com::TStorage<ast::Bundle> & _bundles;

    PoolWasm _poolWasm;
};

struct EmitterNew : com::parallel_for::TJob<PoolWasm, com::TStorage<uint8_t>>
{
    EmitterNew(com::TStorage<ast::Bundle>& bundles) :
        _bundles{bundles}
    {}

    void run(PoolWasm& poolWasm, int32_t index, com::TStorage<uint8_t>* object) override
    {
        new (object) com::TStorage<uint8_t>{gen::make(_bundles[index],
                                                      poolWasm)};
    }

    int32_t size() const override
    {
        return _bundles._size;
    }

    com::TStorage<ast::Bundle> & _bundles;
};

} // namespace dmit::gen
