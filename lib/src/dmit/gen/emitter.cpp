#include "dmit/gen/emitter.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::gen
{

namespace
{

const com::UniqueId K_TYPE_I64{0x7d516e355461f852, 0xeb2349989392e0bb};

const com::UniqueId K_FUNC_ADD_I64_LIT_INT{0xce6a2caefab56273, 0xbedcc1c288a680af};

struct StackIn
{
    wsm::node::Index _cursor;
};

struct Wasmer : ast::TVisitor<Wasmer, StackIn>
{
    DMIT_AST_VISITOR_SIMPLE();

    Wasmer(ast::State::NodePool & poolAst,
           PoolWasm             & poolWasm,
           sem::InterfaceMap    & interfaceMap,
           wsm::node::TIndex<wsm::node::Kind::MODULE> moduleIdx) :
        TVisitor<Wasmer, StackIn>{poolAst},
        _poolWasm{poolWasm},
        _interfaceMap{interfaceMap}
    {
        _stackPtrIn->_cursor = moduleIdx;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS>){}

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        if (function._id == K_FUNC_ADD_I64_LIT_INT)
        {
            return;
        }

        auto& module = _poolWasm.get(wsm::node::as<wsm::node::Kind::MODULE>(_stackPtrIn->_cursor));

        auto& typeFunc = _poolWasm.get(module._types[_indexType]);

        auto&   domain = _poolWasm.makeGet(typeFunc.   _domain);
        auto& codomain = _poolWasm.makeGet(typeFunc. _codomain);

        // Build the domain

        _poolWasm.make(domain._valTypes, function._arguments._size);

        for (int i = 0; i < function._arguments._size; i++)
        {
            auto vIndex = get(get(get(get(function._arguments[i])._typeClaim)._type)._name)._asVIndex;

            auto id = isInterface(vIndex) ? ast::node::v_index::makeId(_interfaceMap._astNodePool, vIndex)
                                          : ast::node::v_index::makeId(_nodePool, vIndex);

            if (id == K_TYPE_I64)
            {
                wsm::node::TIndex<wsm::node::Kind::TYPE_I64> i64Idx;
                _poolWasm.make(i64Idx);
                com::blit(i64Idx, _poolWasm.get(domain._valTypes[i])._asVariant);
            }
            else
            {
                DMIT_COM_ASSERT(!"Unknown type!");
            }
        }

        // And the codomain

        _poolWasm.make(codomain._valTypes, 1);

        auto vIndex = get(get(function._returnType.value())._name)._asVIndex;

        auto id = isInterface(vIndex) ? ast::node::v_index::makeId(_interfaceMap._astNodePool, vIndex)
                                      : ast::node::v_index::makeId(_nodePool, vIndex);

        if (id == K_TYPE_I64)
        {
            wsm::node::TIndex<wsm::node::Kind::TYPE_I64> i64Idx;
            _poolWasm.make(i64Idx);
            com::blit(i64Idx, _poolWasm.get(codomain._valTypes[0])._asVariant);
        }
        else
        {
            DMIT_COM_ASSERT(!"Unknown type!");
        }

        _indexType++;

        // Now build the function

        auto& wasmFunc = _poolWasm.get(module._funcs[_indexFunc]);

        wasmFunc._typeIdx = _indexType;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        base()(get(definitionIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        base()(get(moduleIdx)._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    ~Wasmer()
    {
        auto& module = _poolWasm.get(wsm::node::as<wsm::node::Kind::MODULE>(_stackPtrIn->_cursor));

        _poolWasm.trim(module._types   , _indexType   );
        _poolWasm.trim(module._funcs   , _indexFunc   );
        _poolWasm.trim(module._exports , _indexExport );

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

    uint32_t _indexType   = 0;
    uint32_t _indexFunc   = 0;
    uint32_t _indexExport = 0;
};

} // namespace

com::TStorage<uint8_t> make(sem::InterfaceMap& interfaceMap,
                            ast::Bundle& bundle,
                            PoolWasm& poolWasm)
{
    auto nbDefinition = bundle.nbDefinition();

    wsm::node::TIndex<wsm::node::Kind::MODULE> moduleIdx;

    auto& module = poolWasm.makeGet(moduleIdx);

    poolWasm.make(module._types   , nbDefinition);
    poolWasm.make(module._funcs   , nbDefinition);
    poolWasm.make(module._exports , nbDefinition);

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
