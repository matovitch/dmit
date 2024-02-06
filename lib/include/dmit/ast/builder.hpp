#pragma once

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/src/file.hpp"

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>

namespace dmit::ast
{

struct Builder : com::parallel_for::TJob<FromPathAndSource, State>
{
    Builder(const std::vector<std::filesystem::path>  & paths,
            const std::vector<com::TStorage<uint8_t>> & contents) :
        _paths{paths},
        _contents{contents}
    {
        TNode<node::Kind::LIT_INTEGER>::_status = node::Status::ASTED;
    }

    void run(FromPathAndSource& fromPathAndSource, int32_t index, State* state) override
    {
        new (state) State{fromPathAndSource.make(_paths[index], _contents[index])};
    }

    int32_t size() const override
    {
        return _paths.size();
    }

    const std::vector<std::filesystem::path>  & _paths;
    const std::vector<com::TStorage<uint8_t>> & _contents;
};

} // namespace dmit::ast
