#pragma once

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/enum.hpp"

#include <cstdint>
#include <vector>

namespace dmit::prs
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
        DCL_VARIABLE,
        EXP_ASSIGN,
        EXP_BINOP,
        EXP_OPERATOR,
        EXP_OPPOSE,
        FUN_ARGUMENTS,
        FUN_CALL,
        FUN_DEFINITION,
        FUN_RETURN,
        LIT_DECIMAL,
        LIT_IDENTIFIER,
        LIT_INTEGER,
        PROGRAM,
        SCOPE,
        STM_RETURN,
        STM_WHILE,
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

    Node(const node::Kind kind,
         const uint32_t size,
         const uint32_t childCount,
         const uint32_t start,
         const uint32_t stop);

    node::Kind  _kind = node::Kind::END_OF_TREE;
    uint32_t    _size;
    uint32_t    _childCount;
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
                 const uint32_t childCount,
                 const uint32_t start,
                 const uint32_t stop)
    {
        if constexpr (ARITY != tree::node::Arity::VARIADIC)
        {
            addNode(KIND, size, childCount, start, stop);
        }
        // Here comparison with underflow is used for scope recovery
        else if (_nodes.back()._size < size - 1)
        {
            addNode(KIND, size, childCount, start, stop);
        }
    }

    void resize(const std::size_t size);

    const std::vector<tree::Node>& nodes() const;

private:

    void addNode(const tree::node::Kind kind,
                 const uint32_t size,
                 const uint32_t childCount,
                 const uint32_t start,
                 const uint32_t stop);

    std::vector<tree::Node>  _nodes;
};

} // namespace state
} // namespace dmit::prs
