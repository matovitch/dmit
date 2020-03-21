#pragma once

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/enum.hpp"

#include <cstdint>
#include <vector>

namespace dmit
{

namespace prs
{

struct State;

namespace state
{

namespace tree
{

namespace node
{

struct Kind : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        INVALID,
        INTEGER,
        DECIMAL,
        IDENTIFIER,
        OPPOSE,
        INVERSE,
        PRODUCT,
        SUM,
        ASSIGNMENT,
        TYP_INFER,
        DECLAR_LET,
        DECLAR_VAR,
        LIST_ARG,
        LIST_DISP,
        STA_RETURN
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);
};

struct Arity : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        VARIADIC,
        ONE
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Arity);
};

} // namespace node

struct Node : fmt::Formatable
{
    Node() = default;

    Node(const node::Kind  kind,
         const uint32_t size,
         const uint32_t start,
         const uint32_t stop);

    node::Kind  _kind = node::Kind::INVALID;
    uint32_t    _size;
    uint32_t    _start;
    uint32_t    _stop;
};

} // namespace tree

class Tree : fmt::Formatable
{

public:

    void clear();

    uint32_t size() const;

    template <com::TEnumIntegerType<tree::node::Arity > ARITY,
              com::TEnumIntegerType<tree::node::Kind  > KIND>
    void addNode(const uint32_t size,
                 const uint32_t start,
                 const uint32_t stop)
    {
        if constexpr (ARITY != tree::node::Arity::VARIADIC)
        {
            _nodes.emplace_back(KIND, size, start, stop);
        }
        else if (_nodes.back()._size < size - 1)
        {
            _nodes.emplace_back(KIND, size, start, stop);
        }
    }

    void resize(const std::size_t size);

    const std::vector<tree::Node>& nodes() const;

private:

    std::vector<tree::Node> _nodes;
};

} // namespace state
} // namespace prs
} // namespace dmit
