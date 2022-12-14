#pragma once

#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/bundle.hpp"

#include "dmit/com/storage.hpp"

#include <cstdint>
#include <vector>

namespace dmit::gen
{

using PoolWasm = dmit::wsm::node::TPool<0xC>;

com::TStorage<uint8_t> make(ast::Bundle& bundle,
                            PoolWasm& poolWasm);
struct Emitter
{
    using ReturnType = com::TStorage<uint8_t>;

    Emitter(std::vector<ast::Bundle>& bundles) :
        _bundles{bundles}
    {}

    com::TStorage<uint8_t> run(const uint64_t index)
    {
        return gen::make(_bundles[index],
                         _poolWasm);
    }

    uint32_t size() const
    {
        return _bundles.size();
    }

    std::vector<ast::Bundle> & _bundles;

    PoolWasm _poolWasm;
};

} // namespace dmit::gen
