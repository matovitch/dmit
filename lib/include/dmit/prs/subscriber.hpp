#pragma once

#include "dmit/prs/reader.hpp"

#include <optional>

namespace dmit
{

namespace prs
{

struct Stack;
struct State;

struct Subscriber
{
    virtual void onStart (const               Reader  &,       Stack&, State&) const = 0;
    virtual void onEnd   (const std::optional<Reader> &, const Stack&, State&) const = 0;

    virtual ~Subscriber() {}
};

} // namespace prs

} // namespace dmit
