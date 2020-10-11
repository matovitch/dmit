#pragma once

#include "dmit/prs/pipeline.hpp"
#include "dmit/prs/stack.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/lex/reader.hpp"

#include "pool/pool.hpp"

#include <functional>
#include <optional>
#include <memory>
#include <vector>

namespace dmit::prs
{

template <class Open, class Close>
class TParser
{

public:

    using Fn = std::function<std::optional<lex::Reader>(lex::Reader)>;

    TParser(std::optional<Fn>& parserFnOpt, State& state) :
        _parserFnOpt{parserFnOpt},
        _state{state}
    {}

    void operator=(const Fn& parserFn)
    {
        _parserFnOpt = parserFn;
    }

    std::optional<lex::Reader> operator()(lex::Reader reader) const
    {
        Stack stack;

        Open{}(reader, stack, _state);
        auto&& readerOpt = (_parserFnOpt.value())(reader);

        if (readerOpt)
        {
            readerOpt.value().advanceToRawToken();
        }

        Close{}(readerOpt, stack, _state);

        return readerOpt;
    }

private:

    std::optional<Fn> & _parserFnOpt;
    State             & _state;
};

namespace parser
{

class Pool
{
    using Fn = std::function<std::optional<lex::Reader>(lex::Reader)>;

public:

    template <class Open  = open  ::TPipeline<>,
              class Close = close ::TPipeline<>>
    TParser<Open, Close> make(State& state)
    {
        auto& fnPtr = _fnPtrs.make();

        return TParser<Open, Close>{fnPtr, state};
    }

private:

    pool::TMake<std::optional<Fn>, 0x100> _fnPtrs;
};

} // namespace parser
} // namespace dmit::prs
