#include "test.hpp"

#include "dmit/wsm/writer.hpp"
#include "dmit/wsm/emit.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/base64.hpp"

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
    dmit::wsm::node::TPool<0x10> nodePool;

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::MODULE> moduleIdx;

    nodePool.make(moduleIdx);

    auto& module = nodePool.get(moduleIdx);

    nodePool.make(module._types, 1);

    auto& typeFunc = nodePool.get(module._types[0]);

    nodePool.make(typeFunc.   _domain);
    nodePool.make(typeFunc. _codomain);

    auto&   domain = nodePool.get(typeFunc.   _domain);
    auto& codomain = nodePool.get(typeFunc. _codomain);

    nodePool.make(  domain._valTypes, 1);
    nodePool.make(codomain._valTypes, 1);

    auto&   domainType = nodePool.get(  domain._valTypes[0]);
    auto& codomainType = nodePool.get(codomain._valTypes[0]);

    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::TYPE_I32> i32Idx1;
    dmit::wsm::node::TIndex<dmit::wsm::node::Kind::TYPE_I32> i32Idx2;

    nodePool.make(i32Idx1);
    nodePool.make(i32Idx2);

      domainType._asVariant = i32Idx1;
    codomainType._asVariant = i32Idx2;

    dmit::wsm::writer::Bematist bematist;

    dmit::wsm::emit(moduleIdx, nodePool, bematist);

    std::vector<uint8_t> writeBuffer;

    writeBuffer.resize(bematist._size);

    dmit::wsm::writer::Scribe scribe{writeBuffer.data()};

    dmit::wsm::emit(moduleIdx, nodePool, scribe);

    std::cout << base64Encode(writeBuffer) << '\n';
}
