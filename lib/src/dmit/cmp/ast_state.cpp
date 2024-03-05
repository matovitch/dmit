#include "dmit/cmp/ast/state.hpp"

#include "dmit/cmp/com/tree_pool.hpp"
#include "dmit/cmp/lex/token.hpp"
#include "dmit/cmp/cmp.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/storage.hpp"

#include <optional>
#include <cstdint>

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const ast::State& state)
{
    // 0. Write the node pool

    if (!write<ast::node::Kind, ast::TNode, 0xC>(context, state._nodePool))
    {
        return false;
    }

    // 1. Write the module and source index

    if (!write<ast::node::Kind, ast::node::Kind::MODULE>(context, state._module))
    {
        return false;
    }

    if (!write<ast::node::Kind, ast::node::Kind::SOURCE>(context, state._source))
    {
        return false;
    }

    // 2. (Re)Write the source node

    auto& source = state._nodePool.get(state._source);

    const auto& srcPathAsString = source._srcPath.string();

    if (!writeStr(context, srcPathAsString.data(),
                           srcPathAsString.size()))
    {
        return false;
    }

    writeBin(context, source._srcContent.value().data(),
                      source._srcContent.value()._size);

    if (!writeArray32(context, source._srcOffsets.size()))
    {
        return false;
    }

    for (auto srcOffset : source._srcOffsets)
    {
        if (!writeU32(context, srcOffset))
        {
            return false;
        }
    }

    if (!writeArray32(context, source._lexOffsets.size()))
    {
        return false;
    }

    for (auto lexOffset : source._lexOffsets)
    {
        if (!writeU32(context, lexOffset))
        {
            return false;
        }
    }

    write(source._lexTokens.begin(),
          source._lexTokens.end(), context);

    return true;
}

std::optional<ast::State> readAstState(cmp_ctx_t* context, ast::SourceRegister& sourceRegister)
{
    auto nodePool = readComTreePool<ast::node::Kind, ast::TNode, 0xC>(context);

    if (!nodePool)
    {
        return std::nullopt;
    }

    auto module = readComTreeIndex<ast::node::Kind, ast::node::Kind::MODULE>(context);

    if (!module)
    {
        return std::nullopt;
    }

    auto sourceIdx = readComTreeIndex<ast::node::Kind, ast::node::Kind::SOURCE>(context);

    if (!sourceIdx)
    {
        return std::nullopt;
    }

    auto& source = nodePool.value().get(sourceIdx.value());

    uint32_t srcPathSize;

    if (!readStrSize(context, &srcPathSize))
    {
        return std::nullopt;
    }

    std::string srcPathAsString;

    srcPathAsString.resize(srcPathSize);

    if (!readBytes(context, srcPathAsString.data(), srcPathSize))
    {
        return std::nullopt;
    }

    uint32_t srcContentSize;

    if (!readBinSize(context, &srcContentSize))
    {
        return std::nullopt;
    }

    com::TStorage<uint8_t> srcContent{srcContentSize};

    if (!readBytes(context, srcContent.data(), srcContentSize))
    {
        return std::nullopt;
    }

    sourceRegister.add(source, srcPathAsString, srcContent);

    uint32_t              srcOffsetsSize;
    std::vector<uint32_t> srcOffsets;

    if (!readArray(context, &srcOffsetsSize))
    {
        return std::nullopt;
    }

    srcOffsets.resize(srcOffsetsSize);

    for (uint32_t i = 0; i < srcOffsetsSize; i++)
    {
        if (!readU32(context, srcOffsets.data() + i))
        {
            return std::nullopt;
        }
    }

    uint32_t              lexOffsetsSize;
    std::vector<uint32_t> lexOffsets;

    if (!readArray(context, &lexOffsetsSize))
    {
        return std::nullopt;
    }

    lexOffsets.resize(lexOffsetsSize);

    for (uint32_t i = 0; i < lexOffsetsSize; i++)
    {
        if (!readU32(context, lexOffsets.data() + i))
        {
            return std::nullopt;
        }
    }

    source._srcOffsets.swap(srcOffsets);
    source._lexOffsets.swap(lexOffsets);

    return std::nullopt;
}

} // namespace dmit::cmp