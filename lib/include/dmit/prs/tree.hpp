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
        COMPARISON,
        ASSIGNMENT,
        OPERATOR,
        FUN_CALL,
        ARG_LIST,
        STATEM_WHILE,
        STATEM_RETURN,
        DECLAR_LET,
        DECLAR_FUN,
        SCOPE,
        PROGRAM,
        END_OF_TREE
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
         const uint32_t size);

    node::Kind  _kind = node::Kind::INVALID;
    uint32_t    _size;
};

struct Range : fmt::Formatable
{
    Range() = default;

    Range(const uint32_t start,
          const uint32_t stop);

    uint32_t    _start;
    uint32_t    _stop;
};

} // namespace tree

class Tree : fmt::Formatable
{

public:

    Tree();

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
            addNode(KIND, size, start, stop);
        }
        else if (_nodes.back()._size < size - 1)
        {
            addNode(KIND, size, start, stop);
        }
    }

    void resize(const std::size_t size);

    const std::vector<tree::Node>& nodes() const;

    const std::vector<tree::Range>& ranges() const;

    const tree::Range& range(const tree::Node& node) const;

private:

    void addNode(const tree::node::Kind kind,
                 const uint32_t size,
                 const uint32_t start,
                 const uint32_t stop);

    std::vector<tree::Node>  _nodes;
    std::vector<tree::Range> _ranges;
};

} // namespace state
} // namespace prs
} // namespace dmit
