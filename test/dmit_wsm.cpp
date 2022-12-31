#include "test.hpp"

#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"
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
    nodePool.make(module._datas        , 0);
    nodePool.make(module._imports      , 0);
    nodePool.make(module._exports      , 1);
    nodePool.make(module._symbols      , 0);

    dmit::com::blitDefault(module._startOpt);
    dmit::com::blitDefault(module._relocSizeCode);
    dmit::com::blitDefault(module._relocSizeData);

    auto& typeFunc = nodePool.get(module._types[0]);

    typeFunc._id = 0;

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

    function._id = 0;
    function._type = module._types[0];

    nodePool.make(function._locals);
    nodePool.make(function._body);

    auto& local_1 = nodePool.grow(function._locals);
    auto& local_2 = nodePool.grow(function._locals);

    local_1._id = 0;
    local_2._id = 1;

    auto& localsGet_0 = nodePool.grow(function._body);
    auto& localsGet_1 = nodePool.grow(function._body);
    auto& add         = nodePool.grow(function._body);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_0;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_1;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_I32       > instAdd;

    nodePool.makeGet(instLocalGet_0)._local = nodePool.get(nodePool.back(function._locals))._next;
    nodePool.makeGet(instLocalGet_1)._local =              nodePool.back(function._locals);
    nodePool.makeGet(instAdd)._asEnum = dmit::wsm::NumericInstruction::ADD;

    nodePool.make(function._locals);

    dmit::com::blit(instLocalGet_0 , localsGet_0._asVariant);
    dmit::com::blit(instLocalGet_1 , localsGet_1._asVariant);
    dmit::com::blit(instAdd        ,         add._asVariant);

    auto& export_ = nodePool.get(module._exports[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

    nodePool.makeGet(funcRefIdx)._function = module._funcs[0];

    auto& name = nodePool.makeGet(export_._name);

    nodePool.make(name._bytes, 3);

    nodePool.get(name._bytes[0])._value = 'a';
    nodePool.get(name._bytes[1])._value = 'd';
    nodePool.get(name._bytes[2])._value = 'd';

    dmit::com::blit(funcRefIdx, export_._descriptor);

    // 2. Write it

    auto emitSize = dmit::wsm::emitSizeNoObject(moduleIdx, nodePool);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emitNoObject(moduleIdx, nodePool, storage.data());

    // 3. Run it

    wasm3::Environment env;

    wasm3::Runtime runtime = env.makeRuntime(0x100 /*stackSize*/);

    wasm3::Result result = m3Err_none;

    wasm3::Module m3module;
    result = wasm3::parseModule(env, m3module, storage.data(), emitSize);
    DMIT_COM_ASSERT(!result && "Could not parse wasm module");

    result = wasm3::loadModule(runtime, m3module);
    DMIT_COM_ASSERT(!result && "Could not load wasm module");

    wasm3::Function m3add;
    result = wasm3::findFunction(m3add, runtime, "add");
    DMIT_COM_ASSERT(!result && "Could not find add function");

    result = wasm3::call(m3add, 2, 3);
    DMIT_COM_ASSERT(!result && "Call to add failed");

    int32_t value = 0;
    result = wasm3::getResults(m3add, &value);
    DMIT_COM_ASSERT(!result && "Could not get result from addition");

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
    nodePool.make(module._datas        , 0);
    nodePool.make(module._imports      , 0);
    nodePool.make(module._exports      , 1);
    nodePool.make(module._symbols      , 0);

    dmit::com::blitDefault(module._startOpt);
    dmit::com::blitDefault(module._relocSizeCode);
    dmit::com::blitDefault(module._relocSizeData);


    auto& typeFunc = nodePool.get(module._types[0]);

    typeFunc._id = 0;

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

    function._id = 0;
    function._type = module._types[0];

    nodePool.make(function._locals);
    nodePool.make(function._body);

    auto& local_1 = nodePool.grow(function._locals);
    auto& local_2 = nodePool.grow(function._locals);

    local_1._id = 0;
    local_2._id = 1;

    dmit::com::blit(i32Idx, nodePool.makeGet(local_2._type)._asVariant);

    auto& localGet_0 = nodePool.grow(function._body);
    auto& i32const_1 = nodePool.grow(function._body);
    auto& i32add     = nodePool.grow(function._body);
    auto& localSet_1 = nodePool.grow(function._body);
    auto& localGet_1 = nodePool.grow(function._body);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_0;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_CONST_I32 > instConst;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_I32       > instAdd;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_SET > instLocalSet_1;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_1;

    nodePool.makeGet(instLocalGet_0)._local = nodePool.get(nodePool.back(function._locals))._next;
    nodePool.makeGet(instConst)._value = 1;
    nodePool.makeGet(nodePool.get(instConst)._relocation)._type = dmit::wsm::RelocationType::NONE;
    nodePool.makeGet(instAdd)._asEnum = dmit::wsm::NumericInstruction::ADD;
    nodePool.makeGet(instLocalSet_1)._local = nodePool.back(function._locals);
    nodePool.makeGet(instLocalGet_1)._local = nodePool.back(function._locals);

    nodePool.get(nodePool.back(function._locals))._next = function._locals._begin;
    function._localsSize = 1;

    dmit::com::blit(instLocalGet_0 , localGet_0._asVariant);
    dmit::com::blit(instLocalGet_1 , localGet_1._asVariant);
    dmit::com::blit(instLocalSet_1 , localSet_1._asVariant);
    dmit::com::blit(instConst      , i32const_1._asVariant);
    dmit::com::blit(instAdd        ,     i32add._asVariant);

    auto& export_ = nodePool.get(module._exports[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_REF_FUNC> funcRefIdx;

    nodePool.makeGet(funcRefIdx)._function = module._funcs[0];

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

    auto emitSize = dmit::wsm::emitSizeNoObject(moduleIdx, nodePool);

    dmit::com::TStorage<uint8_t> storage{emitSize};

    dmit::wsm::emitNoObject(moduleIdx, nodePool, storage.data());

    // 3. Run it

    wasm3::Environment env;

    wasm3::Runtime runtime = env.makeRuntime(0x100);

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
