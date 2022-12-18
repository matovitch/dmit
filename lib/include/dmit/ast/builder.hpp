#pragma once

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/src/file.hpp"

#include "dmit/com/constant_reference.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace dmit::ast
{

struct Builder
{
    using ReturnType = ast::State;

    Builder(const std::vector<src::File>& files) : _files{files} {}

    ast::State run(const uint64_t index)
    {
        return _astFromPathAndSource.make(_files[index]);
    }

    uint32_t size() const
    {
        return _files.size();
    }

    const std::vector<src::File>& _files;

    ast::FromPathAndSource _astFromPathAndSource;
};

} // namespace dmit::ast
