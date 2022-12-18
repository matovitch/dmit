#pragma once

#include "dmit/ast/node.hpp"

#include "dmit/com/storage.hpp"

#include <filesystem>
#include <cstdint>
#include <vector>

namespace dmit::ast
{

class SourceRegister
{

public:

    void add(TNode<node::Kind::SOURCE>&, const std::filesystem::path&, const com::TStorage<uint8_t>&);

    ~SourceRegister();

private:

    std::vector<TNode<node::Kind::SOURCE>*> _sources;
};

} // namespace dmit::ast
