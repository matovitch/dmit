#pragma once

#include "dmit/prs/subscriber.hpp"
#include "dmit/prs/reader.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/enum.hpp"

#include <optional>
#include <cstdint>
#include <vector>

namespace dmit
{

namespace prs
{

struct Stack;
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
        UNWRAP,
        ONE
    };
};

} // namespace node

struct Node : fmt::Formatable
{
    Node() = default;

    Node(const node::Kind,
         const uint32_t,
         const uint32_t,
         const uint32_t);

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

    void addNode(const tree::node::Arity arity,
                 const tree::node::Kind,
                 const uint32_t,
                 const uint32_t,
                 const uint32_t);

    void resize(const std::size_t);

    const std::vector<tree::Node>& nodes() const;

private:

    std::vector<tree::Node> _nodes;
};

} // namespace state

namespace subscriber
{

namespace tree
{

class Writer : public Subscriber
{
    using NodeArity = state::tree::node::Arity;
    using NodeKind  = state::tree::node::Kind;
    
public:

    Writer(const com::TEnumIntegerType<NodeKind>,
           const com::TEnumIntegerType<NodeArity>);

    void onStart(const Reader&, Stack&, State&) const override;

    void onEnd(const std::optional<Reader>&, const Stack&, State&) const override;

private:

    const NodeKind  _nodeKind;
    const NodeArity _nodeArity;
};

} // namespace tree

} // namespace subscriber

} // namepsace prs

} // namespace dmit
