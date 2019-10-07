#pragma once

#include "dmit/prs/parser.hpp"
#include "dmit/prs/error.hpp"
#include "dmit/prs/tree.hpp"
#include "dmit/prs/pool.hpp"

#include "dmit/lex/token.hpp"

#include <vector>

namespace dmit
{

namespace prs
{

struct State
{

    void clear();

    state::Tree       _tree;
    state::error::Set _errorSet;
};

namespace state
{

class Builder
{

public:

    Builder();

    const State& operator()(const std::vector<lex::Token>&);

    void clearState();

private:

    pool::Subscriber _poolSubscriber;
    pool::Parser     _poolParser;
    Parser           _parser;
    State            _state;
};

}

} // namespace prs

} // namespace dmit
