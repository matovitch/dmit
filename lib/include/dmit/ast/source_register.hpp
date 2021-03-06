#pragma once

#include "dmit/ast/node.hpp"

#include <vector>

namespace dmit::ast
{

class SourceRegister
{

public:

    void add(TNode<node::Kind::SOURCE>& source);

    ~SourceRegister();

private:

    std::vector<TNode<node::Kind::SOURCE>*> _sources;
};

} // namespace dmit::ast
