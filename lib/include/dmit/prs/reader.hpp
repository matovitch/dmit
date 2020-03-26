#pragma once

#include "dmit/prs/tree.hpp"

#include <functional>
#include <optional>
#include <cstdint>

namespace dmit
{

namespace prs
{

namespace reader
{

struct Head
{
    int32_t _offset;
};

} // namespace reader

class Reader
{

public:

    Reader(const state::Tree& tree);

    void advance();

    const state::tree::Node& look() const;

    bool isValid() const;

    std::optional<Reader> makeSubReader() const;

private:

    Reader(const state::tree::Node* const head,
              const state::tree::Node* const tail);

    const state::tree::Node* _head;
    const state::tree::Node* _tail;
};

} // namespace prs
} // namespace dmit
