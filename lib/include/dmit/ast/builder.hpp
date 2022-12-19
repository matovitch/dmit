#pragma once

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/storage.hpp"
#include "dmit/src/file.hpp"

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>

namespace dmit::ast
{

struct Builder
{
    using ReturnType = ast::State;

    Builder(const std::vector<std::filesystem::path>  & paths,
            const std::vector<com::TStorage<uint8_t>> & contents) :
        _paths{paths},
        _contents{contents}
    {}

    ast::State run(const uint64_t index)
    {
        return _astFromPathAndSource.make(_paths[index], _contents[index]);
    }

    uint32_t size() const
    {
        return _paths.size();
    }

    const std::vector<std::filesystem::path>  & _paths;
    const std::vector<com::TStorage<uint8_t>> & _contents;

    ast::FromPathAndSource _astFromPathAndSource;
};

} // namespace dmit::ast
