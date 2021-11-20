#include "test.hpp"

#include "dmit/wsm/writer.hpp"
#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/base64.hpp"
#include "dmit/com/blit.hpp"

std::string base64Encode(const std::vector<uint8_t>& toEncode)
{
    std::vector<uint8_t> destBuffer;

    destBuffer.resize(dmit::com::base64::encodeBufferSize(toEncode.size()));

    uint8_t* const dest = destBuffer.data();

    dmit::com::base64::encode(toEncode.data(), toEncode.size(), dest);

    return {reinterpret_cast<char*>(dest), destBuffer.size()};
}

TEST_CASE("PLOP")
{
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
    nodePool.make(module._exports      , 0);

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

    dmit::com::blitDefault(  domainType_0 ._asVariant);
    dmit::com::blitDefault(  domainType_1 ._asVariant);
    dmit::com::blitDefault(codomainType   ._asVariant);

      domainType_0 ._asVariant = i32Idx;
      domainType_1 ._asVariant = i32Idx;
    codomainType   ._asVariant = i32Idx;

    auto& function = nodePool.get(module._funcs[0]);

    function._typeIdx = 1;

    nodePool.make(function._locals , 0);
    nodePool.make(function._body   , 3);

    auto& localsGet_0 = nodePool.get(function._body[0]);
    auto& localsGet_1 = nodePool.get(function._body[1]);
    auto& add         = nodePool.get(function._body[2]);

    dmit::com::blitDefault(localsGet_0._asVariant);
    dmit::com::blitDefault(localsGet_1._asVariant);
    dmit::com::blitDefault(        add._asVariant);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_0;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_LOCAL_GET > instLocalGet_1;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::INST_I32       > instAdd;

    nodePool.make(instLocalGet_0);
    nodePool.make(instLocalGet_1);
    nodePool.make(instAdd);

    nodePool.get(instLocalGet_0)._localIdx = 0;
    nodePool.get(instLocalGet_1)._localIdx = 1;
    nodePool.get(instAdd)._asEnum = dmit::wsm::NumericInstruction::ADD;

    localsGet_0 ._asVariant = instLocalGet_0;
    localsGet_1 ._asVariant = instLocalGet_1;
    add         ._asVariant = instAdd;

    dmit::wsm::writer::Bematist bematist;

    dmit::wsm::emit(moduleIdx, nodePool, bematist);

    std::vector<uint8_t> writeBuffer;

    writeBuffer.resize(bematist._size);

    dmit::wsm::writer::Scribe scribe{writeBuffer.data()};

    dmit::wsm::emit(moduleIdx, nodePool, scribe);

    std::cout << base64Encode(writeBuffer) << '\n';
}
