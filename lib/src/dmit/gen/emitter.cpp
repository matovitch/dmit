#include "dmit/gen/emitter.hpp"

#include "dmit/ast/definition_role.hpp"
#include "dmit/ast/v_index.hpp"
#include "dmit/ast/lexeme.hpp"
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

#include <charconv>
#include <cstdint>
#include <vector>
#include <tuple>

namespace dmit::gen
{

namespace
{

const com::UniqueId K_TYPE_I64     {0x7d516e355461f852, 0xeb2349989392e0bb};
const com::UniqueId K_TYPE_INT     {0x705a28814eebca10, 0xb928e2c4dc06b2ae};
const com::UniqueId K_FUNC_ADD_I64 {0x96b32a82826cbe73, 0xcc4f3b004472d28c};

struct Stack
{
    wsm::node::TIndex<wsm::node::Kind::FUNCTION> _wsmFuncIdx;
};

struct Scribe : ast::TVisitor<Scribe, Stack>
{
    DMIT_AST_VISITOR_SIMPLE();

    Scribe(ast::Bundle & bundle,
           PoolWasm    & poolWasm) :
        TVisitor<Scribe, Stack>{bundle._nodePool},
        _interfacePoolOpt{bundle._interfacePoolOpt},
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

        com::blitDefault(wsmModule._startOpt);
    }

    com::UniqueId id(const ast::node::VIndex& vIndex)
    {
        return isInterface(vIndex) ? ast::node::v_index::makeId(_interfacePoolOpt.value().get(), vIndex)
                                   : ast::node::v_index::makeId(_nodePool, vIndex);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    ast::TNode<KIND>& get(const ast::node::TIndex<KIND> nodeIndex)
    {
        return nodeIndex._isInterface ? _interfacePoolOpt.value().get().get(nodeIndex)
                                      : _nodePool.get(nodeIndex);
    }

    template <class Type>
    std::optional<Type> intFromStringView(std::string_view intAsStringView)
    {
        Type value;

        return (std::from_chars(intAsStringView.begin(),
                                intAsStringView.end(), value).ec == std::errc{}) ? std::optional<int>{value}
                                                                                 : std::nullopt;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER> litIntegerIdx)
    {
        auto slice = ast::lexeme::getSlice(get(litIntegerIdx)._lexeme, _nodePool);

        DMIT_COM_ASSERT(get(get(litIntegerIdx)._expectedType)._id == K_TYPE_I64);

        auto sliceAsIntOpt = intFromStringView<int64_t>(slice.makeStringView());

        DMIT_COM_ASSERT(sliceAsIntOpt);

        wsm::node::TIndex<wsm::node::Kind::INST_CONST_I64> instConst;
        _wsmPool.makeGet(instConst)._value = sliceAsIntOpt.value();

        auto& inst = _wsmPool.grow(_wsmPool.get(_stackPtrIn->_wsmFuncIdx)._body);

        com::blit(instConst , inst._asVariant);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE> dclVariableIdx)
    {
        auto& dclVariable = get(dclVariableIdx);

        auto& wsmFunction = _wsmPool.get(_stackPtrIn->_wsmFuncIdx);

        auto type = id(get(get(get(dclVariable._typeClaim)._type)._name)._asVIndex);

        if (type == K_TYPE_I64 || type == K_TYPE_INT)
        {
            auto& local = _wsmPool.grow(wsmFunction._locals);
            dclVariable._asWsm = _wsmPool.back(wsmFunction._locals);
            local._id = wsmFunction._localsSize++;
            wsm::node::TIndex<wsm::node::Kind::TYPE_I64> i64Idx;
            com::blit(i64Idx, _wsmPool.get(local._type)._asVariant);
        }
        else
        {
            DMIT_COM_ASSERT(!"Unknown type!");
        }

    }

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        auto vIndex = get(identifierIdx)._asVIndex;

        auto wsm = isInterface(vIndex) ? ast::node::v_index::makeWsm(_interfacePoolOpt.value().get(), vIndex)
                                       : ast::node::v_index::makeWsm(_nodePool, vIndex);
        DMIT_COM_ASSERT(wsm);

        auto& inst = _wsmPool.grow(_wsmPool.get(_stackPtrIn->_wsmFuncIdx)._body);

        wsm::node::TIndex<wsm::node::Kind::INST_LOCAL_GET> instLocalGet;
        _wsmPool.makeGet(instLocalGet)._local = std::get<wsm::node::TIndex<wsm::node::Kind::LOCAL>>(wsm.value());
        com::blit(instLocalGet , inst._asVariant);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PATTERN> patternIdx)
    {
        auto identifierIdx = get(patternIdx)._variable;

        auto vIndex = get(identifierIdx)._asVIndex;

        auto wsm = isInterface(vIndex) ? ast::node::v_index::makeWsm(_interfacePoolOpt.value().get(), vIndex)
                                       : ast::node::v_index::makeWsm(_nodePool, vIndex);
        DMIT_COM_ASSERT(wsm);

        auto& inst = _wsmPool.grow(_wsmPool.get(_stackPtrIn->_wsmFuncIdx)._body);

        wsm::node::TIndex<wsm::node::Kind::INST_LOCAL_SET> instLocalSet;
        _wsmPool.makeGet(instLocalSet)._local = std::get<wsm::node::TIndex<wsm::node::Kind::LOCAL>>(wsm.value());
        com::blit(instLocalSet , inst._asVariant);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> expBinopIdx)
    {
        auto& expBinop = get(expBinopIdx);

        if (expBinop._status == ast::node::Status::BOUND)
        {
            if (expBinop._asFunction._isInterface)
            {
                base()(expBinop._lhs);
                base()(expBinop._rhs);

                auto& wsmModule = _wsmPool.get(_wsmModuleIdx);

                auto& function = get(expBinop._asFunction);

                wsm::node::VIndex symbolAsVIndex;

                if (function._asWsm)
                {
                    symbolAsVIndex = function._asWsm.value();
                }
                else
                {
                    // Import type

                    auto& wsmTypeFunc = _wsmPool.grow(wsmModule._types);

                    wsmTypeFunc._id = wsmModule._types._size - 1; // FIXME, type ids should be set later

                    auto& wsmDomain   = _wsmPool.makeGet(wsmTypeFunc.   _domain);
                    auto& wsmCodomain = _wsmPool.makeGet(wsmTypeFunc. _codomain);

                    _wsmPool.make(wsmDomain._valTypes, function._arguments._size);

                    for (int i = 0; i < function._arguments._size; i++)
                    {
                        auto type = id(get(get(get(get(function._arguments[i])._typeClaim)._type)._name)._asVIndex);

                        if (type == K_TYPE_I64)
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

                    // Import
                    auto& import = _wsmPool.grow(wsmModule._imports);

                    import._id = wsmModule._imports._size - 1;

                    auto& nameModule = _wsmPool.makeGet(import._module);
                    auto& nameImport = _wsmPool.makeGet(import._name);

                    _wsmPool.make(nameModule._bytes, 3);

                    _wsmPool.get(nameModule._bytes[0])._value = 'e';
                    _wsmPool.get(nameModule._bytes[1])._value = 'n';
                    _wsmPool.get(nameModule._bytes[2])._value = 'v';

                    auto functionIdAsString = fmt::asString(id(expBinop._asFunction));

                    _wsmPool.make(nameImport._bytes, functionIdAsString.size());

                    for (auto i = 0; i < functionIdAsString.size(); i++)
                    {
                        _wsmPool.get(nameImport._bytes[i])._value = functionIdAsString[i];
                    }

                    com::blit(wsmModule._types.back(), import._descriptor);

                    // Symbol

                    auto& symbol = _wsmPool.grow(wsmModule._symbols);

                    symbol._kind = wsm::SymbolKind::FUNCTION;
                    symbol._flags = wsm::SymbolFlag::UNDEFINED;

                    wsm::node::TIndex<wsm::node::Kind::SYMBOL_OBJECT> symbolImportIdx;

                    auto& symbolImport = _wsmPool.makeGet(symbolImportIdx);

                    symbolImport._index = wsmModule._imports.back();

                    com::blitDefault(symbolImport._name);

                    com::blit(symbolImportIdx, symbol._asVariant);
                    com::blit(symbolImport._index, function._asWsm);

                    symbolAsVIndex = symbolImport._index;
                }

                // Relocation

                auto& relocation = _wsmPool.grow(wsmModule._relocCode);

                relocation._type = wsm::RelocationType::FUNCTION_INDEX_LEB;
                relocation._index = symbolAsVIndex;

                wsmModule._relocSizeCode++;

                // Call

                auto& inst = _wsmPool.grow(_wsmPool.get(_stackPtrIn->_wsmFuncIdx)._body);

                wsm::node::TIndex<wsm::node::Kind::INST_CALL> instCall;

                auto& call_ = _wsmPool.makeGet(instCall);

                call_._function  = symbolAsVIndex;
                call_._relocation = _wsmPool.back(wsmModule._relocCode);

                com::blit(instCall, inst._asVariant);
            }
            else
            {
                // TODO much later
            }
        }
        else
        {
            base()(expBinop._rhs);
            base()(expBinop._lhs);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXPRESSION> expressionIdx)
    {
        base()(get(expressionIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::STM_RETURN> stmReturnIdx)
    {
        base()(get(stmReturnIdx)._expression);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT> scopeVariantIdx)
    {
        base()(get(scopeVariantIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        base()(get(scopeIdx)._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS>){}

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        if (function._asWsm)
        {
            return;
        }

        auto& wsmModule = _wsmPool.get(_wsmModuleIdx);

        auto& wsmTypeFunc = _wsmPool.grow(wsmModule._types);

        wsmTypeFunc._id = wsmModule._types._size - 1; // FIXME, type ids should be set later

        auto& wsmDomain   = _wsmPool.makeGet(wsmTypeFunc.   _domain);
        auto& wsmCodomain = _wsmPool.makeGet(wsmTypeFunc. _codomain);

        // Build the domain

        _wsmPool.make(wsmDomain._valTypes, function._arguments._size);

        for (int i = 0; i < function._arguments._size; i++)
        {
            auto type = id(get(get(get(get(function._arguments[i])._typeClaim)._type)._name)._asVIndex);

            if (type == K_TYPE_I64)
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
        _stackPtrIn->_wsmFuncIdx = wsmModule._funcs.back();
        function._asWsm = _stackPtrIn->_wsmFuncIdx;

        wsmFunction._type = wsmModule._types.back();

        _wsmPool.make(wsmFunction._locals);
        _wsmPool.make(wsmFunction._body);

        base()(function._arguments);

        if (function._id == K_FUNC_ADD_I64)
        {
            auto& localsGet_0     = _wsmPool.grow(wsmFunction._body);
            auto& localsGet_1     = _wsmPool.grow(wsmFunction._body);
            auto& add             = _wsmPool.grow(wsmFunction._body);

            wsm::node::TIndex<wsm::node::Kind::INST_LOCAL_GET > instLocalGet_0;
            wsm::node::TIndex<wsm::node::Kind::INST_LOCAL_GET > instLocalGet_1;
            wsm::node::TIndex<wsm::node::Kind::INST_I64       > instAdd;

            _wsmPool.makeGet(instLocalGet_0)._local =              _wsmPool.back(wsmFunction._locals);
            _wsmPool.makeGet(instLocalGet_1)._local = _wsmPool.get(_wsmPool.back(wsmFunction._locals))._next;
            _wsmPool.makeGet(instAdd)._asEnum = wsm::NumericInstruction::ADD;

            com::blit(instLocalGet_0 , localsGet_0._asVariant);
            com::blit(instLocalGet_1 , localsGet_1._asVariant);
            com::blit(instAdd        ,         add._asVariant);

            _wsmPool.make(wsmFunction._locals);
        }
        else
        {
            _wsmPool.make(wsmFunction._locals);
            base()(function._body);
        }

        wsmFunction._localsSize -= function._arguments._size;

        // Define export

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

            // Symbol

            auto& symbol = _wsmPool.grow(wsmModule._symbols);

            symbol._kind = wsm::SymbolKind::FUNCTION;
            symbol._flags = wsm::SymbolFlag::VISIBILITY_HIDDEN;

            wsm::node::TIndex<wsm::node::Kind::SYMBOL_OBJECT> symbolFunctionIdx;

            auto& symbolFunction = _wsmPool.makeGet(symbolFunctionIdx);

            symbolFunction._index = _stackPtrIn->_wsmFuncIdx;

            com::blit(export_._name, symbolFunction._name);

            com::blit(symbolFunctionIdx, symbol._asVariant);
        }
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

    com::TOptionRef<ast::State::NodePool> _interfacePoolOpt;
    PoolWasm&                             _wsmPool;

    wsm::node::TIndex<wsm::node::Kind::MODULE> _wsmModuleIdx;
};

template <class Derived>
using TWsmVisitor = typename com::tree::TTMetaVisitor<wsm::node::Kind,
                                                      wsm::TNode,
                                                      PoolWasm>::template TVisitor<Derived>;
struct Bematist : TWsmVisitor<Bematist>
{
    DMIT_COM_TREE_VISITOR_SIMPLE(wsm::node, Kind);

    Bematist(PoolWasm& wsmPool) : TWsmVisitor<Bematist>{wsmPool} {}

    void operator()(wsm::node::TIndex<wsm::node::Kind::FUNCTION> wsmFunctionIdx)
    {
        get(wsmFunctionIdx)._id = _counter++;
    }

    void operator()(wsm::node::TIndex<wsm::node::Kind::IMPORT> wsmImportIdx)
    {
        get(wsmImportIdx)._id = _counter++;
    }

    void operator()(wsm::node::TIndex<wsm::node::Kind::MODULE> wsmModuleIdx)
    {
        auto& wsmModule = get(wsmModuleIdx);

        base()(wsmModule._imports);
        base()(wsmModule._funcs);
    }

    uint32_t _counter = 0;
};

} // namespace

com::TStorage<uint8_t> make(ast::Bundle& bundle,
                            PoolWasm& poolWasm)
{
    Scribe scribe{bundle, poolWasm};

    scribe.base()(bundle._views);

    Bematist bematist{poolWasm};

    bematist.base()(scribe._wsmModuleIdx);

    auto emitSize = wsm::emitSizeObject(scribe._wsmModuleIdx, poolWasm);

    com::TStorage<uint8_t> storage{emitSize};

    wsm::emitObject(scribe._wsmModuleIdx, poolWasm, storage.data());

    return storage;
}

} // namespace dmit::gen
