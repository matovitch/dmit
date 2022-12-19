#pragma once

#include "dmit/prs/pipeline.hpp"
#include "dmit/prs/parser.hpp"
#include "dmit/prs/error.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/reference.hpp"

#include <optional>
#include <vector>

namespace dmit::prs
{

struct State : fmt::Formatable
{
    void clear();

    state::Tree            _tree;
    state::error::SetOfSet _errors;

    std::optional<state::tree::node::Kind> _treeNodeKindOpt;
    dmit::com::TOptionRef<Stack>           _stackRefOpt;
};

namespace state
{

namespace tree::writer
{

template <com::TEnumIntegerType<state::tree::node::Kind> NODE_KIND>
struct Open
{
    void operator()(const lex::Reader& reader, Stack& stack, State& state) const
    {
        stack._treeSize     = state._tree.size();
        stack._readerOffset = reader.offset();

        stack._parent      = state._stackRefOpt;
        state._stackRefOpt = stack;

        stack._treeNodeKindOpt = state._treeNodeKindOpt;
        state._treeNodeKindOpt = NODE_KIND;
    }
};

template <com::TEnumIntegerType<state::tree::node::Arity > NODE_ARITY,
          com::TEnumIntegerType<state::tree::node::Kind  > NODE_KIND>
struct Close
{
    void operator()(const std::optional<lex::Reader>& readerOpt, Stack& stack, State& state) const
    {
        if (!readerOpt)
        {
            state._tree.resize(stack._treeSize);
            goto REVERT_STACK_AND_RETURN;
        }

        state._tree.addNode<NODE_ARITY,
                            NODE_KIND>(state._tree.size() - stack._treeSize,
                                       stack._childCount,
                                       stack._readerOffset,
                                       readerOpt.value().offset());
        if (stack._parent)
        {
            stack._parent.value().get()._childCount++;
        }

        REVERT_STACK_AND_RETURN:
            state._stackRefOpt     = stack._parent;
            state._treeNodeKindOpt = stack._treeNodeKindOpt;
    }
};

} // namespace tree::writer

namespace error
{

namespace token_check
{

template <com::TEnumIntegerType<lex::Token> EXPECTED_TOKEN>
struct Open
{
    void operator()(const lex::Reader& reader, Stack& stack, State& state) const
    {
        stack._isErrorPushed = state._errors.push(EXPECTED_TOKEN,
                                                  reader.look(),
                                                  state._treeNodeKindOpt.value(),
                                                  reader.offset());
    }
};

struct Close
{
    void operator()(const std::optional<lex::Reader>& readerOpt, Stack& stack, State& state) const
    {
        if (readerOpt && stack._isErrorPushed)
        {
            state._errors.cleanUp();
        }
    }
};

} // namespace token_check

namespace recoverable
{

struct Open
{
    void operator()(const lex::Reader& reader, Stack& stack, State& state) const
    {
        stack._readerOffset = reader.offset();
    }
};

struct Close
{
    void operator()(const std::optional<lex::Reader>& readerOpt, Stack& stack, State& state) const
    {
        if (readerOpt && state._errors.offset() <= stack._readerOffset)
        {
            state._errors.recover();
        }
    }
};

} // namespace recoverable
} // namespace error

class Builder
{
    using Parser = TParser< open::TPipeline<>,
                           close::TPipeline<>>;
public:

    Builder();

    const State& operator()(const std::vector<lex::Token>&);

    void clearState();

private:

    parser::Pool _pool;
    State        _state;
    Parser       _parser;
};

} // namespace state
} // namespace dmit::prs
