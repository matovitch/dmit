#include "dmit/gen/emitter.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

#include "dmit/com/storage.hpp"

#include <cstdint>

namespace dmit::gen
{

namespace
{

struct StackIn
{
    wsm::node::Index _cursor;
};

struct StackOut
{
    uint32_t _indexType   = 0;
    uint32_t _indexFunc   = 0;
    uint32_t _indexExport = 0;
};

struct Wasmer : ast::TVisitor<Wasmer, StackIn, StackOut>
{
    DMIT_AST_VISITOR_SIMPLE();

    Wasmer(ast::State::NodePool & poolAst,
           PoolWasm             & poolWasm,
           sem::InterfaceMap    & interfaceMap,
           wsm::node::TIndex<wsm::node::Kind::MODULE> moduleIdx) :
        TVisitor<Wasmer, StackIn, StackOut>{poolAst},
        _poolWasm{poolWasm},
        _interfaceMap{interfaceMap}
    {
        _stackPtrIn->_cursor = moduleIdx;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        //base()(get(viewIdx)._modules);
    }

    ~Wasmer()
    {
        auto& module = _poolWasm.get(wsm::node::as<wsm::node::Kind::MODULE>(_stackPtrIn->_cursor));

        _poolWasm.trim(module._types   , _stackPtrOut->_indexType   );
        _poolWasm.trim(module._funcs   , _stackPtrOut->_indexFunc   );
        _poolWasm.trim(module._exports , _stackPtrOut->_indexExport );

        _poolWasm.make(module._tables       , 0);
        _poolWasm.make(module._mems         , 0);
        _poolWasm.make(module._globalConsts , 0);
        _poolWasm.make(module._globalVars   , 0);
        _poolWasm.make(module._elems        , 0);
        _poolWasm.make(module._datas        , 0);
        _poolWasm.make(module._imports      , 0);

        dmit::com::blitDefault(module._startOpt);
    }

    PoolWasm& _poolWasm;
    sem::InterfaceMap& _interfaceMap;
};

} // namespace

com::TStorage<uint8_t> make(sem::InterfaceMap& interfaceMap,
                            ast::Bundle& bundle,
                            PoolWasm& poolWasm)
{
    auto nbDefinition = bundle.nbDefinition();

    wsm::node::TIndex<wsm::node::Kind::MODULE> moduleIdx;

    poolWasm.make(moduleIdx);

    auto& module = poolWasm.get(moduleIdx);

    poolWasm.make(module._types        , nbDefinition);
    poolWasm.make(module._funcs        , nbDefinition);
    poolWasm.make(module._exports      , nbDefinition);

    {
        Wasmer wasmer{bundle._nodePool, poolWasm, interfaceMap, moduleIdx};
        wasmer.base()(bundle._views);
    }

    auto emitSize = dmit::wsm::emitSize(moduleIdx, poolWasm);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emit(moduleIdx, poolWasm, storage.data());

    return storage;
}

} // namespace dmit::gen
