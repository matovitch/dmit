#pragma once

#include "dmit/prs/subscriber.hpp"
#include "dmit/prs/reader.hpp"

#include <functional>
#include <optional>
#include <memory>
#include <vector>

namespace dmit
{

namespace prs
{

struct Stack;
struct State;

class Parser
{

public:

    using Fn = std::function<std::optional<Reader>(Reader)>;

    Parser(std::optional<Fn>&, State&);

    void operator=(const Fn&);

    std::optional<Reader> operator()(Reader) const;

    void bindSubscriber(Subscriber* const);

protected:

    void notifyStart(const Reader&, Stack&) const;

    void notifyEnd(const std::optional<Reader>&, const Stack&) const;

    std::optional<Fn>& _parserFnOpt;

    State& _state;

    std::vector<Subscriber*> _subscribers;
};

} // namespace prs

} // namespace dmit
