#include "test.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/storage.hpp"
#include "dmit/com/blit.hpp"

#include "wasm3/wasm3.hpp"

#include <cstdint>

TEST_CASE("wsm")
{
    // 1. Build the WASM module

    dmit::wsm::node::TPool<0xC> nodePool;

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::MODULE> moduleIdx;

    nodePool.make(moduleIdx);

    auto& module = nodePool.get(moduleIdx);

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

    nodePool.make(typeFunc.   _domain);
    nodePool.make(typeFunc. _codomain);

    auto&   domain = nodePool.get(typeFunc.   _domain);
    auto& codomain = nodePool.get(typeFunc. _codomain);

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

    nodePool.make(instLocalGet_0);
    nodePool.make(instLocalGet_1);
    nodePool.make(instAdd);

    nodePool.get(instLocalGet_0)._localIdx = 0;
    nodePool.get(instLocalGet_1)._localIdx = 1;
    nodePool.get(instAdd)._asEnum = dmit::wsm::NumericInstruction::ADD;

    dmit::com::blit(instLocalGet_0 , localsGet_0._asVariant);
    dmit::com::blit(instLocalGet_1 , localsGet_1._asVariant);
    dmit::com::blit(instAdd        ,         add._asVariant);

    auto& export_ = nodePool.get(module._exports[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

    nodePool.make(export_._name);
    nodePool.make(funcRefIdx);

    nodePool.get(funcRefIdx)._funcIdx = 1;

    auto& name = nodePool.get(export_._name);

    nodePool.make(name._bytes, 3);

    nodePool.get(name._bytes[0])._value = 'a';
    nodePool.get(name._bytes[1])._value = 'd';
    nodePool.get(name._bytes[2])._value = 'd';

    dmit::com::blit(funcRefIdx, export_._descriptor);

    // 2. Write it

    auto emitSize = dmit::wsm::emitSize(moduleIdx, nodePool);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emit(moduleIdx, nodePool, storage.data());

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
