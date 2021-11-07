#pragma once

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include <cstdint>
#include <vector>

namespace dmit::ast
{

struct Builder
{
    using ReturnType = ast::State;

    Builder(const std::vector<std::vector<uint8_t>>& paths,
               const std::vector<std::vector<uint8_t>>& sources) :
        _paths{paths},
        _sources{sources}
    {}

    ast::State run(const uint64_t index)
    {
        return _astFromPathAndSource.make(_paths   [index],
                                          _sources [index]);
    }

    uint32_t size() const
    {
        return _paths.size();
    }

    const std::vector<std::vector<uint8_t>>& _paths   ;
    const std::vector<std::vector<uint8_t>>& _sources ;

    ast::FromPathAndSource _astFromPathAndSource;
};

} // namespace dmit::ast
