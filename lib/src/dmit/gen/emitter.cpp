#include "dmit/gen/emitter.hpp"

#include "dmit/ast/definition_role.hpp"
#include "dmit/ast/v_index.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

#include "dmit/fmt/com/unique_id.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"

#include <cstdint>
#include <vector>

namespace dmit::gen
{

namespace
{

const com::UniqueId K_TYPE_I64             {0x7d516e355461f852, 0xeb2349989392e0bb};
const com::UniqueId K_FUNC_ADD_I64_LIT_INT {0xce6a2caefab56273, 0xbedcc1c288a680af};

struct Scribe : ast::TVisitor<Scribe>
{
    DMIT_AST_VISITOR_SIMPLE();

    Scribe(ast::State::NodePool  & poolAst,
           sem::InterfaceMap     & interfaceMap,
           PoolWasm              & poolWasm) :
        TVisitor<Scribe>{poolAst},
        _interfaceMap{interfaceMap},
        _wsmPool{poolWasm}
    {
        auto& wsmModule = _wsmPool.makeGet(_wsmModuleIdx);

        _wsmPool.make(wsmModule._types        , 0);
        _wsmPool.make(wsmModule._funcs        , 0);
        _wsmPool.make(wsmModule._tables       , 0);
        _wsmPool.make(wsmModule._mems         , 0);
        _wsmPool.make(wsmModule._globalConsts , 0);
        _wsmPool.make(wsmModule._globalVars   , 0);
        _wsmPool.make(wsmModule._datas        , 0);
        _wsmPool.make(wsmModule._imports      , 0);
        _wsmPool.make(wsmModule._exports      , 0);
        _wsmPool.make(wsmModule._symbols      , 0);
        _wsmPool.make(wsmModule._relocCode);
        _wsmPool.make(wsmModule._relocData);

        dmit::com::blitDefault(wsmModule._startOpt);
        dmit::com::blitDefault(wsmModule._relocCode);
        dmit::com::blitDefault(wsmModule._relocData);
    }

    com::UniqueId id(const ast::node::VIndex& vIndex)
    {
        return isInterface(vIndex) ? ast::node::v_index::makeId(_interfaceMap._astNodePool, vIndex)
                                   : ast::node::v_index::makeId(_nodePool, vIndex);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS>){}

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        if (function._id == K_FUNC_ADD_I64_LIT_INT)
        {
            return; // TODO make the add function
        }

        auto& wsmModule = _wsmPool.get(_wsmModuleIdx);

        auto& wsmTypeFunc = _wsmPool.grow(wsmModule._types);

        auto& wsmDomain   = _wsmPool.makeGet(wsmTypeFunc.   _domain);
        auto& wsmCodomain = _wsmPool.makeGet(wsmTypeFunc. _codomain);

        // Build the domain

        _wsmPool.make(wsmDomain._valTypes, function._arguments._size);

        for (int i = 0; i < function._arguments._size; i++)
        {
            auto vIndex = get(get(get(get(function._arguments[i])._typeClaim)._type)._name)._asVIndex;

            if (id(vIndex) == K_TYPE_I64)
            {
                wsm::node::TIndex<wsm::node::Kind::TYPE_I64> wsmI64Idx;
                _wsmPool.make(wsmI64Idx);
                com::blit(wsmI64Idx, _wsmPool.get(wsmDomain._valTypes[i])._asVariant);
            }
            else
            {
                DMIT_COM_ASSERT(!"Unknown type!");
            }
        }

        // And the codomain

        _wsmPool.make(wsmCodomain._valTypes, 1);

        auto vIndex = get(get(function._returnType.value())._name)._asVIndex;

        if (id(vIndex) == K_TYPE_I64)
        {
            wsm::node::TIndex<wsm::node::Kind::TYPE_I64> wsmI64Idx;
            _wsmPool.make(wsmI64Idx);
            com::blit(wsmI64Idx, _wsmPool.get(wsmCodomain._valTypes[0])._asVariant);
        }
        else
        {
            DMIT_COM_ASSERT(!"Unknown type!");
        }

        // Now build the function

        auto& wsmFunction = _wsmPool.grow(wsmModule._funcs);

        wsmFunction._typeIdx = _idxFunc;

        _wsmPool.make(wsmFunction._locals, 0);
        _wsmPool.make(wsmFunction._body);

        // TODO make the wasm function

        auto defRole = ast::node::v_index::makeDefinitionRole(_nodePool, function._parent);

        if (defRole == ast::DefinitionRole::EXPORTED)
        {
            auto& export_ = _wsmPool.grow(wsmModule._exports);

            wsm::node::TIndex<wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

            _wsmPool.makeGet(funcRefIdx)._function = wsmModule._funcs.back();

            com::blit(funcRefIdx, export_._descriptor);

            auto functionIdAsString = fmt::asString(function._id);

            auto& name = _wsmPool.makeGet(export_._name);

            _wsmPool.make(name._bytes, functionIdAsString.size());

            for (auto i = 0; i < functionIdAsString.size(); i++)
            {
                _wsmPool.get(name._bytes[i])._value = functionIdAsString[i];
            }
        }

        _idxFunc++;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        base()(definition._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        base()(get(moduleIdx)._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    sem::InterfaceMap & _interfaceMap;
    PoolWasm          & _wsmPool;

    wsm::node::TIndex<wsm::node::Kind::MODULE> _wsmModuleIdx;

    uint32_t _idxFunc   = 0;
};

} // namespace

com::TStorage<uint8_t> make(sem::InterfaceMap& interfaceMap,
                            ast::Bundle& bundle,
                            PoolWasm& poolWasm)
{
    Scribe scribe{bundle._nodePool, interfaceMap, poolWasm};

    scribe.base()(bundle._views);

    auto emitSize = wsm::emitSizeObject(scribe._wsmModuleIdx, poolWasm);

    com::TStorage<uint8_t> storage{emitSize};

    wsm::emitObject(scribe._wsmModuleIdx, poolWasm, storage.data());

    return storage;
}

} // namespace dmit::gen
