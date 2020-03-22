#pragma once

#include "dmit/prs/parser.hpp"
#include "dmit/prs/error.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/lex/token.hpp"

#include <optional>
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

namespace tree
{

namespace writer
{

struct Open
{
    void operator()(const Reader& reader, Stack& stack, State& state) const
    {
        stack._treeSize     = state._tree.size();
        stack._readerOffset = reader.offset();
    }
};

template <com::TEnumIntegerType<state::tree::node::Arity > NODE_ARITY,
          com::TEnumIntegerType<state::tree::node::Kind  > NODE_KIND>
struct Close
{
    void operator()(const std::optional<Reader>& readerOpt, const Stack& stack, State& state) const
    {
        if (!readerOpt)
        {
            state._tree.resize(stack._treeSize);
            return;
        }

        const auto size = state._tree.size() - stack._treeSize;

        state._tree.addNode<NODE_ARITY,
                            NODE_KIND>(size,
                                       stack._readerOffset,
                                       readerOpt.value().offset());
    }
};

} // namespace writer
} // namespace tree

namespace error
{

namespace token_check
{

template <com::TEnumIntegerType<lex::Token> EXPECTED_TOKEN>
struct Open
{
    void operator()(const Reader& reader, Stack& stack, State& state) const
    {
        if (reader.offset() <= state._errorSet.offset())
        {
            state._errorSet.push(EXPECTED_TOKEN, reader.look(), reader.offset());
        }
    }
};

struct Close
{
    void operator()(const std::optional<Reader>& readerOpt, const Stack& stack, State& state) const
    {
        if (readerOpt)
        {
            state._errorSet.pop();
        }
    }
};

} // namespace token_check

namespace clear
{

struct Close
{
    void operator()(const std::optional<Reader>& readerOpt, const Stack& stack, State& state) const
    {
        if (readerOpt && readerOpt.value().isEoi())
        {
            state._errorSet.clear();
        }
    }
};

} // namespace clear
} // namespace error

class Builder
{
    using Parser = TParser<open::TPipeline<>, error::clear::Close>;

public:

    Builder();

    const State& operator()(const std::vector<lex::Token>&, std::optional<Parser> = std::nullopt);

    void clearState();

private:

    parser::Pool     _pool;
    State            _state;

public:

    Parser _parserProgram;
    Parser _parserFunction;
    Parser _parserStatement;
    Parser _parserDeclaration;
    Parser _parserAssignment;
    Parser _parserExpression;
};

} // namespace state
} // namespace prs
} // namespace dmit
