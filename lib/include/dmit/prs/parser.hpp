#pragma once

#include "dmit/prs/pipeline.hpp"
#include "dmit/prs/reader.hpp"
#include "dmit/prs/stack.hpp"
#include "dmit/prs/state.hpp"

#include <functional>
#include <optional>
#include <memory>
#include <vector>

namespace dmit
{

namespace prs
{

template <class Open, class Close>
class TParser
{

public:

    using Fn = std::function<std::optional<Reader>(Reader)>;

    TParser(std::optional<Fn>& parserFnOpt, State& state) :
        _parserFnOpt{parserFnOpt},
        _state{state}
    {}

    void operator=(const Fn& parserFn)
    {
        _parserFnOpt.get() = parserFn;
    }

    std::optional<Reader> operator()(Reader reader) const
    {
        Stack stack;

        reader.advanceToRawToken();

        Open{}(reader, stack, _state.get());
        const auto& readerOpt = (_parserFnOpt.get().value())(reader);
        Close{}(readerOpt, stack, _state.get());

        return readerOpt;
    }

private:

    std::reference_wrapper<std::optional<Fn>> _parserFnOpt;
    std::reference_wrapper<State>             _state;
};

namespace parser
{

class Pool
{
    using Fn = std::function<std::optional<Reader>(Reader)>;

public:

    template <class Open  = open  ::TPipeline<>,
              class Close = close ::TPipeline<>>
    TParser<Open, Close> make(State& state)
    {
        _fnPtrs.emplace_back(new std::optional<Fn>);

        return TParser<Open, Close>{*(_fnPtrs.back()), state};
    }

private:

    std::vector<std::unique_ptr<std::optional<Fn>>> _fnPtrs;
};

} // namespace parser
} // namespace prs
} // namespace dmit