#include "dmit/prs/parser.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/stack.hpp"
#include "dmit/prs/state.hpp"

#include <functional>
#include <optional>
#include <vector>

namespace dmit
{

namespace prs
{

Parser::Parser(std::optional<Fn>& parserFnOpt, State& state) :
    _parserFnOpt{parserFnOpt},
    _state{state}
{}

void Parser::operator=(const Parser::Fn& parserFn)
{
    _parserFnOpt = parserFn;
}

std::optional<Reader> Parser::operator()(Reader reader) const
{
    Stack stack;

    reader.advanceToRawToken();

    notifyStart(reader, stack);
    const auto& readerOpt = (_parserFnOpt.value())(reader);
    notifyEnd(readerOpt, stack);

    return readerOpt;
}

void Parser::bindSubscriber(Subscriber* const subscriber)
{
    _subscribers.push_back(subscriber);
}

void Parser::notifyStart(const Reader& reader, Stack& stack) const
{
    for (auto& suscriber : _subscribers)
    {
        suscriber->onStart(reader, stack, _state);
    }
}

void Parser::notifyEnd(const std::optional<Reader>& readerOpt, const Stack& stack) const
{
    for (auto& suscriber : _subscribers)
    {
        suscriber->onEnd(readerOpt, stack, _state);
    }
}

} // namespace prs

} // namespace dmit
