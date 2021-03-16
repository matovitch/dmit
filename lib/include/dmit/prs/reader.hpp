#pragma once

#include "dmit/prs/tree.hpp"

#include <cstdint>

namespace dmit::prs
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

    bool isValidNext() const;

    Reader makeSubReader() const;

    uint32_t size() const;

private:

    Reader(const state::tree::Node* const head,
           const state::tree::Node* const tail);

    const state::tree::Node* _head;
    const state::tree::Node* _tail;
};

} // namespace dmit::prs
