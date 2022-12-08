#include "dmit/gen/emitter.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

#include "dmit/fmt/com/unique_id.hpp"

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

namespace scribe
{

struct Stack
{
    bool _isExport = false;
};

} // namespace scribe

struct Scribe : ast::TVisitor<Scribe, scribe::Stack>
{
    DMIT_AST_VISITOR_SIMPLE();

    Scribe(ast::State::NodePool  & poolAst,
           sem::InterfaceMap     & interfaceMap,
           PoolWasm              & poolWasm,
           std::vector<uint32_t> & measures) :
        TVisitor<Scribe, scribe::Stack>{poolAst},
        _interfaceMap{interfaceMap},
        _wsmPool{poolWasm},
        _measures{measures}
    {
        auto& wsmModule = _wsmPool.makeGet(_wsmModuleIdx);

        _wsmPool.make(wsmModule._types        , 0);
        _wsmPool.make(wsmModule._funcs        , 0);
        _wsmPool.make(wsmModule._tables       , 0);
        _wsmPool.make(wsmModule._mems         , 0);
        _wsmPool.make(wsmModule._globalConsts , 0);
        _wsmPool.make(wsmModule._globalVars   , 0);
        _wsmPool.make(wsmModule._elems        , 0);
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

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS>){}

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        if (function._id == K_FUNC_ADD_I64_LIT_INT)
        {
            return; // TODO make the add function
        }

        auto& wsmModule = _wsmPool.get(_wsmModuleIdx);

        if (!_idxFunc)
        {
            auto nbFunction = _measures[_measuresIdx++];
            _wsmPool.make(wsmModule._types, nbFunction);
            _wsmPool.make(wsmModule._funcs, nbFunction);
        }

        if (!_idxExport)
        {
            auto nbExport = _measures[_measuresIdx++];
            _wsmPool.make(wsmModule._exports, nbExport);
        }

        auto& wsmTypeFunc = _wsmPool.get(wsmModule._types[_idxFunc]);

        auto& wsmDomain   = _wsmPool.makeGet(wsmTypeFunc.   _domain);
        auto& wsmCodomain = _wsmPool.makeGet(wsmTypeFunc. _codomain);

        // Build the domain

        _wsmPool.make(wsmDomain._valTypes, function._arguments._size);

        for (int i = 0; i < function._arguments._size; i++)
        {
            auto vIndex = get(get(get(get(function._arguments[i])._typeClaim)._type)._name)._asVIndex;

            auto id = isInterface(vIndex) ? ast::node::v_index::makeId(_interfaceMap._astNodePool, vIndex)
                                          : ast::node::v_index::makeId(_nodePool, vIndex);
            if (id == K_TYPE_I64)
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

        auto id = isInterface(vIndex) ? ast::node::v_index::makeId(_interfaceMap._astNodePool, vIndex)
                                      : ast::node::v_index::makeId(_nodePool, vIndex);
        if (id == K_TYPE_I64)
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

        auto& wsmFunction = _wsmPool.get(wsmModule._funcs[_idxFunc++]);

        wsmFunction._typeIdx = _idxFunc;

        _wsmPool.make(wsmFunction._locals, 0);
        _wsmPool.make(wsmFunction._body);

        // TODO make the wasm function

        if (_stackPtrIn->_isExport)
        {
            auto& export_ = _wsmPool.get(wsmModule._exports[_idxExport++]);

            wsm::node::TIndex<wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

            _wsmPool.makeGet(funcRefIdx)._funcIdx = (_idxFunc << 1) - 1;

            com::blit(funcRefIdx, export_._descriptor);

            auto functionIdAsString = fmt::asString(function._id);

            auto& name = _wsmPool.makeGet(export_._name);

            _wsmPool.make(name._bytes, functionIdAsString.size());

            for (auto i = 0; i < functionIdAsString.size(); i++)
            {
                _wsmPool.get(name._bytes[i])._value = functionIdAsString[i];
            }
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        _stackPtrIn->_isExport = (definition._role == ast::DefinitionRole::EXPORTED);

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
    uint32_t _idxExport = 0;

    uint32_t              _measuresIdx = 0;
    std::vector<uint32_t> _measures;
};

namespace bematist
{

struct Stack
{
    bool     _isExport   = false;
    uint32_t _nbFunction = 0;
    uint32_t _nbExport   = 0;
};

} // namespace bematist

struct Bematist : ast::TVisitor<Bematist, bematist::Stack,
                                          bematist::Stack>
{
    Bematist(ast::State::NodePool & poolAst,
             sem::InterfaceMap    & interfaceMap) :
        TVisitor<Bematist, bematist::Stack,
                           bematist::Stack>{poolAst},
        _interfaceMap{interfaceMap}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS>)
    {
        *_stackPtrOut = *_stackPtrIn;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        *_stackPtrOut = *_stackPtrIn;

        _stackPtrOut->_nbFunction += 1;
        _stackPtrOut->_nbExport   += _stackPtrOut->_isExport;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        _stackPtrIn->_isExport = (definition._role == ast::DefinitionRole::EXPORTED);

        base()(definition._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto indexFunction = _measures.size();
                             _measures.push_back(0);
        auto indexExport   = _measures.size();
                             _measures.push_back(0);

        _stackPtrIn->_nbFunction = 0;
        _stackPtrIn->_nbExport   = 0;

        base()(get(moduleIdx)._definitions);

        _measures[indexFunction ] = _stackPtrOut->_nbFunction;
        _measures[indexExport   ] = _stackPtrOut->_nbExport;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationConclusion(ast::node::TIndex<KIND>)
    {
        *_stackPtrIn = *_stackPtrOut;
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationPreamble(ast::node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopConclusion(ast::node::TRange<KIND>& range) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopPreamble(ast::node::TRange<KIND>&) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopConclusion(ast::node::TList<KIND>& range) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopPreamble(ast::node::TList<KIND>&) {}

    template <class Type>
    void emptyOption() {}

    sem::InterfaceMap&    _interfaceMap;
    std::vector<uint32_t> _measures;
};


} // namespace

com::TStorage<uint8_t> make(sem::InterfaceMap& interfaceMap,
                            ast::Bundle& bundle,
                            PoolWasm& poolWasm)
{
    Bematist bematist{bundle._nodePool, interfaceMap};

    bematist.base()(bundle._views);

    Scribe scribe{bundle._nodePool, interfaceMap, poolWasm, bematist._measures};

    scribe.base()(bundle._views);

    auto emitSize = wsm::emitSizeObject(scribe._wsmModuleIdx, poolWasm);

    com::TStorage<uint8_t> storage{emitSize};

    wsm::emitObject(scribe._wsmModuleIdx, poolWasm, storage.data());

    return storage;
}

} // namespace dmit::gen
