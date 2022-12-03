#include "test.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/storage.hpp"
#include "dmit/com/blit.hpp"

#include "wasm3/wasm3.hpp"

#include <cstdint>

TEST_CASE("wsm_add")
{
    // 1. Build the WASM module

    dmit::wsm::node::TPool<0xC> nodePool;

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::MODULE> moduleIdx;

    auto& module = nodePool.makeGet(moduleIdx);

    nodePool.make(module._types        , 1);
    nodePool.make(module._funcs        , 1);
    nodePool.make(module._tables       , 0);
    nodePool.make(module._mems         , 0);
    nodePool.make(module._globalConsts , 0);
    nodePool.make(module._globalVars   , 0);
    nodePool.make(module._elems        , 0);
    nodePool.make(module._datas        , 0);
    nodePool.make(module._imports      , 0);
    nodePool.make(module._exports      , 1);

    dmit::com::blitDefault(module._startOpt);

    auto& typeFunc = nodePool.get(module._types[0]);

    auto&   domain = nodePool.makeGet(typeFunc.   _domain);
    auto& codomain = nodePool.makeGet(typeFunc. _codomain);

    nodePool.make(  domain._valTypes, 2);
    nodePool.make(codomain._valTypes, 1);

    auto&   domainType_0 = nodePool.get(  domain._valTypes[0]);
    auto&   domainType_1 = nodePool.get(  domain._valTypes[1]);
    auto& codomainType   = nodePool.get(codomain._valTypes[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::TYPE_I32> i32Idx;

    nodePool.make(i32Idx);

    dmit::com::blit(i32Idx,   domainType_0 ._asVariant);
    dmit::com::blit(i32Idx,   domainType_1 ._asVariant);
    dmit::com::blit(i32Idx, codomainType   ._asVariant);

    auto& function = nodePool.get(module._funcs[0]);

    function._typeIdx = 1;

    nodePool.make(function._locals , 0);
    nodePool.make(function._body   , 3);

    auto& localsGet_0 = nodePool.get(function._body[0]);
    auto& localsGet_1 = nodePool.get(function._body[1]);
    auto& add         = nodePool.get(function._body[2]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_0;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_1;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_I32       > instAdd;

    nodePool.makeGet(instLocalGet_0)._localIdx = 0;
    nodePool.makeGet(instLocalGet_1)._localIdx = 1;
    nodePool.makeGet(instAdd)._asEnum = dmit::wsm::NumericInstruction::ADD;

    dmit::com::blit(instLocalGet_0 , localsGet_0._asVariant);
    dmit::com::blit(instLocalGet_1 , localsGet_1._asVariant);
    dmit::com::blit(instAdd        ,         add._asVariant);

    auto& export_ = nodePool.get(module._exports[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

    nodePool.makeGet(funcRefIdx)._funcIdx = 1;

    auto& name = nodePool.makeGet(export_._name);

    nodePool.make(name._bytes, 3);

    nodePool.get(name._bytes[0])._value = 'a';
    nodePool.get(name._bytes[1])._value = 'd';
    nodePool.get(name._bytes[2])._value = 'd';

    dmit::com::blit(funcRefIdx, export_._descriptor);

    // 2. Write it

    auto emitSize = dmit::wsm::emitSize(moduleIdx, nodePool, false /*isObject*/);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emit(moduleIdx, nodePool, storage.data(), false /*isObject*/);

    // 3. Run it

    wasm3::Environment env;

    wasm3::Runtime runtime = env.makeRuntime(0x100 /*stackSize*/);

    wasm3::Result result = m3Err_none;

    wasm3::Module m3module;
    result = wasm3::parseModule(env, m3module, storage.data(), emitSize);
    CHECK(!result);

    result = wasm3::loadModule(runtime, m3module);
    CHECK(!result);

    wasm3::Function m3add;
    result = wasm3::findFunction(m3add, runtime, "add");
    CHECK(!result);

    result = wasm3::call(m3add, 2, 3);
    CHECK(!result);

    int32_t value = 0;
    result = wasm3::getResults(m3add, &value);
    CHECK(!result);

    CHECK(value == 5);
}

TEST_CASE("wsm_increment")
{
    // 1. Build the WASM module

    dmit::wsm::node::TPool<0xC> nodePool;

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::MODULE> moduleIdx;

    auto& module = nodePool.makeGet(moduleIdx);

    nodePool.make(module._types        , 1);
    nodePool.make(module._funcs        , 1);
    nodePool.make(module._tables       , 0);
    nodePool.make(module._mems         , 0);
    nodePool.make(module._globalConsts , 0);
    nodePool.make(module._globalVars   , 0);
    nodePool.make(module._elems        , 0);
    nodePool.make(module._datas        , 0);
    nodePool.make(module._imports      , 0);
    nodePool.make(module._exports      , 1);

    dmit::com::blitDefault(module._startOpt);

    auto& typeFunc = nodePool.get(module._types[0]);

    auto&   domain = nodePool.makeGet(typeFunc.   _domain);
    auto& codomain = nodePool.makeGet(typeFunc. _codomain);

    nodePool.make(  domain._valTypes, 1);
    nodePool.make(codomain._valTypes, 1);

    auto&   domainType = nodePool.get(  domain._valTypes[0]);
    auto& codomainType = nodePool.get(codomain._valTypes[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::TYPE_I32> i32Idx;

    nodePool.make(i32Idx);

    dmit::com::blit(i32Idx,   domainType ._asVariant);
    dmit::com::blit(i32Idx, codomainType ._asVariant);

    auto& function = nodePool.get(module._funcs[0]);

    function._typeIdx = 1;

    nodePool.make(function._locals , 1);
    nodePool.make(function._body   , 5);

    auto& local_1 = nodePool.get(function._locals[0]);
    dmit::com::blit(i32Idx, local_1._asVariant);

    auto& localGet_0 = nodePool.get(function._body[0]);
    auto& i32const_1 = nodePool.get(function._body[1]);
    auto& i32add     = nodePool.get(function._body[2]);
    auto& localSet_1 = nodePool.get(function._body[3]);
    auto& localGet_1 = nodePool.get(function._body[4]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_0;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_CONST_I32 > instConst;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_I32       > instAdd;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_SET > instLocalSet_1;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_1;

    nodePool.makeGet(instLocalGet_0)._localIdx = 0;
    nodePool.makeGet(instConst)._value = 1;
    nodePool.makeGet(instAdd)._asEnum = dmit::wsm::NumericInstruction::ADD;
    nodePool.makeGet(instLocalSet_1)._localIdx = 1;
    nodePool.makeGet(instLocalGet_1)._localIdx = 1;

    dmit::com::blit(instLocalGet_0 , localGet_0._asVariant);
    dmit::com::blit(instLocalGet_1 , localGet_1._asVariant);
    dmit::com::blit(instLocalSet_1 , localSet_1._asVariant);
    dmit::com::blit(instConst      , i32const_1._asVariant);
    dmit::com::blit(instAdd        ,     i32add._asVariant);

    auto& export_ = nodePool.get(module._exports[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

    nodePool.makeGet(funcRefIdx)._funcIdx = 1;

    auto& name = nodePool.makeGet(export_._name);

    nodePool.make(name._bytes, 9);

    nodePool.get(name._bytes[0])._value = 'i';
    nodePool.get(name._bytes[1])._value = 'n';
    nodePool.get(name._bytes[2])._value = 'c';
    nodePool.get(name._bytes[3])._value = 'r';
    nodePool.get(name._bytes[4])._value = 'e';
    nodePool.get(name._bytes[5])._value = 'm';
    nodePool.get(name._bytes[6])._value = 'e';
    nodePool.get(name._bytes[7])._value = 'n';
    nodePool.get(name._bytes[8])._value = 't';

    dmit::com::blit(funcRefIdx, export_._descriptor);

    // 2. Write it

    auto emitSize = dmit::wsm::emitSize(moduleIdx, nodePool, false /*isObject*/);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emit(moduleIdx, nodePool, storage.data(), false /*isObject*/);

    // 3. Run it

    wasm3::Environment env;

    wasm3::Runtime runtime = env.makeRuntime(0x100 /*stackSize*/);

    wasm3::Result result = m3Err_none;

    wasm3::Module m3module;
    result = wasm3::parseModule(env, m3module, storage.data(), emitSize);
    CHECK(!result);

    result = wasm3::loadModule(runtime, m3module);
    CHECK(!result);

    wasm3::Function m3add;
    result = wasm3::findFunction(m3add, runtime, "increment");
    CHECK(!result);

    result = wasm3::call(m3add, 2);
    CHECK(!result);

    int32_t value = 0;
    result = wasm3::getResults(m3add, &value);
    CHECK(!result);

    CHECK(value == 3);
}
