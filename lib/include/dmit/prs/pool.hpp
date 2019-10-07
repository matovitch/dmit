#pragma once

#include "dmit/prs/subscriber.hpp"
#include "dmit/prs/parser.hpp"

#include <optional>
#include <memory>
#include <vector>

namespace dmit
{

namespace prs
{

struct State;

namespace pool
{

class Parser
{

public:

    prs::Parser make(State&);

private:

    std::vector<std::unique_ptr<std::optional<prs::Parser::Fn>>> _parsers;
};

class Subscriber
{

public:

    template <class Type, class... Args>
    void bind(prs::Parser& parser, Args&&... args)
    {
        _subscribers.emplace_back(new Type{args...});

        parser.bindSubscriber(_subscribers.back().get());
    }

private:

    std::vector<std::unique_ptr<prs::Subscriber>> _subscribers;

};

} // namespace pool

} // namespace prs

} // namespace dmit