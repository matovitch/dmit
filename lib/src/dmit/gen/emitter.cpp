#include "dmit/gen/emitter.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/bundle.hpp"

#include "dmit/com/storage.hpp"

#include <cstdint>

namespace dmit::gen
{

com::TStorage<uint8_t> make(sem::InterfaceMap& interfaceMap,
                            ast::Bundle& bundle,
                            PoolWasm& poolWasm)
{
    auto nbDefinition = bundle.nbDefinition();

    uint32_t nbType   = 0;
    uint32_t nbFunc   = 0;
    uint32_t nbExport = 0;

    wsm::node::TIndex<wsm::node::Kind::MODULE> moduleIdx;

    poolWasm.make(moduleIdx);

    auto& module = poolWasm.get(moduleIdx);

    poolWasm.make(module._types        , nbDefinition);
    poolWasm.make(module._funcs        , nbDefinition);
    poolWasm.make(module._tables       , 0);
    poolWasm.make(module._mems         , 0);
    poolWasm.make(module._globalConsts , 0);
    poolWasm.make(module._globalVars   , 0);
    poolWasm.make(module._elems        , 0);
    poolWasm.make(module._datas        , 0);
    poolWasm.make(module._imports      , 0);
    poolWasm.make(module._exports      , nbDefinition);

    poolWasm.trim(module._types   , nbType   );
    poolWasm.trim(module._funcs   , nbFunc   );
    poolWasm.trim(module._exports , nbExport );

    dmit::com::blitDefault(module._startOpt);

    auto emitSize = dmit::wsm::emitSize(moduleIdx, poolWasm);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emit(moduleIdx, poolWasm, storage.data());

    return storage;
}

} // namespace dmit::gen
