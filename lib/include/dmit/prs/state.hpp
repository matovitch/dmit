#pragma once

#include "dmit/prs/parser.hpp"
#include "dmit/prs/error.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/option_reference.hpp"

#include <optional>
#include <vector>

namespace dmit::prs
{

struct State : fmt::Formatable
{
    void clear();

    state::Tree            _tree;
    state::error::SetOfSet _errors;

    std::optional<state::tree::node::Kind> _errorNoteOpt;

    dmit::com::OptionRef<Stack> _stackRefOpt;
};

namespace state
{

namespace tree::writer
{

struct Open
{
    void operator()(const lex::Reader& reader, Stack& stack, State& state) const
    {
        stack._treeSize     = state._tree.size();
        stack._readerOffset = reader.offset();

        stack._parent = state._stackRefOpt;
        state._stackRefOpt = stack;
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
            state._stackRefOpt = stack._parent;
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
                                                  state._errorNoteOpt.value(),
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

namespace clean_up
{

struct Close
{
    void operator()(const std::optional<lex::Reader>& readerOpt, Stack& stack, State& state) const
    {
        if (readerOpt && readerOpt.value().isEoi())
        {
            state._errors.cleanUp();
        }
    }
};

} // namespace clean_up

namespace recover
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
        if (!readerOpt)
        {
            return;
        }

        if (state._errors.offset() <= stack._readerOffset)
        {
            state._errors.recover();
        }
    }
};

} // namespace recover

namespace note
{

template <com::TEnumIntegerType<tree::node::Kind> TREE_NODE_KIND>
struct Open
{
    void operator()(const lex::Reader& reader, Stack& stack, State& state) const
    {
        stack._errorNoteOpt = state._errorNoteOpt;
        state._errorNoteOpt = TREE_NODE_KIND;
    }
};

struct Close
{
    void operator()(const std::optional<lex::Reader>& readerOpt, Stack& stack, State& state) const
    {
        state._errorNoteOpt = stack._errorNoteOpt;
    }
};

} // namespace note

} // namespace error

class Builder
{
    using Parser = TParser<error::note::Open<tree::node::Kind::PROGRAM>, error::clean_up::Close>;

public:

    Builder();

    const State& operator()(const std::vector<lex::Token>&);

    void clearState();

private:

    parser::Pool _pool;
    Parser       _parser;
    State        _state;
};

} // namespace state
} // namespace dmit::prs
