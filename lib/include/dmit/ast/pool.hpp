#pragma once

#include "dmit/ast/storage.hpp"
#include "dmit/ast/node.hpp"

namespace dmit
{

namespace ast
{

namespace node
{

namespace pool
{

template <com::TEnumIntegerType<Kind> KIND, uint8_t LOG2_SIZE>
class TSub
{

public:

    const TNode<KIND>& get(const TIndex<KIND> index) const
    {
        return reinterpret_cast<const TNode<KIND>&>(_storage.get(index._value));
    }

    TNode<KIND>& get(const TIndex<KIND> index)
    {
        return reinterpret_cast<TNode<KIND>&>(_storage.get(index._value));
    }

    TIndex<KIND> make()
    {
        return TIndex<KIND>{_storage.make()};
    }

    void make(const uint32_t size, TRange<KIND>& range)
    {
        range._size = size;
        range._index._value = _storage.make(size);
    }

private:

    storage::TMake<TNode<KIND>, LOG2_SIZE> _storage;
};

} // namespace pool

template <uint8_t LOG2_SIZE>
struct TPool
{
    template <com::TEnumIntegerType<Kind> KIND>
    const TNode<KIND>& get(const TIndex<KIND> index) const;

    template <com::TEnumIntegerType<Kind> KIND>
    TNode<KIND>& get(const TIndex<KIND> index);

    template <com::TEnumIntegerType<Kind> KIND>
    void make(TIndex<KIND>& index);

    template <com::TEnumIntegerType<Kind> KIND>
    void make(const uint32_t size, TRange<KIND>& range);

    template <> TNode<Kind::FUNCTION         >& get<Kind::FUNCTION         >(const TIndex<Kind::FUNCTION         > index) { return _function          .get(index); }
    template <> TNode<Kind::LEXEME           >& get<Kind::LEXEME           >(const TIndex<Kind::LEXEME           > index) { return _lexeme            .get(index); }
    template <> TNode<Kind::ARGUMENTS        >& get<Kind::ARGUMENTS        >(const TIndex<Kind::ARGUMENTS        > index) { return _arguments         .get(index); }
    template <> TNode<Kind::SCOPE            >& get<Kind::SCOPE            >(const TIndex<Kind::SCOPE            > index) { return _scope             .get(index); }
    template <> TNode<Kind::TYPING_STATEMENT >& get<Kind::TYPING_STATEMENT >(const TIndex<Kind::TYPING_STATEMENT > index) { return _typingStatement   .get(index); }
    template <> TNode<Kind::RETURN_TYPE      >& get<Kind::RETURN_TYPE      >(const TIndex<Kind::RETURN_TYPE      > index) { return _returnType        .get(index); }

    template <> const TNode<Kind::FUNCTION         >& get<Kind::FUNCTION         >(const TIndex<Kind::FUNCTION         > index) const { return _function          .get(index); }
    template <> const TNode<Kind::LEXEME           >& get<Kind::LEXEME           >(const TIndex<Kind::LEXEME           > index) const { return _lexeme            .get(index); }
    template <> const TNode<Kind::ARGUMENTS        >& get<Kind::ARGUMENTS        >(const TIndex<Kind::ARGUMENTS        > index) const { return _arguments         .get(index); }
    template <> const TNode<Kind::SCOPE            >& get<Kind::SCOPE            >(const TIndex<Kind::SCOPE            > index) const { return _scope             .get(index); }
    template <> const TNode<Kind::TYPING_STATEMENT >& get<Kind::TYPING_STATEMENT >(const TIndex<Kind::TYPING_STATEMENT > index) const { return _typingStatement   .get(index); }
    template <> const TNode<Kind::RETURN_TYPE      >& get<Kind::RETURN_TYPE      >(const TIndex<Kind::RETURN_TYPE      > index) const { return _returnType        .get(index); }

    template <> void make<Kind::FUNCTION         >(TIndex<Kind::FUNCTION         >& index) { index = _function          .make();}
    template <> void make<Kind::LEXEME           >(TIndex<Kind::LEXEME           >& index) { index = _lexeme            .make();}
    template <> void make<Kind::ARGUMENTS        >(TIndex<Kind::ARGUMENTS        >& index) { index = _arguments         .make();}
    template <> void make<Kind::SCOPE            >(TIndex<Kind::SCOPE            >& index) { index = _scope             .make();}
    template <> void make<Kind::TYPING_STATEMENT >(TIndex<Kind::TYPING_STATEMENT >& index) { index = _typingStatement   .make();}
    template <> void make<Kind::RETURN_TYPE      >(TIndex<Kind::RETURN_TYPE      >& index) { index = _returnType        .make();}

    template <> void make<Kind::FUNCTION         >(const uint32_t size, TRange<Kind::FUNCTION         >& range) { _function          .make(size, range);}
    template <> void make<Kind::LEXEME           >(const uint32_t size, TRange<Kind::LEXEME           >& range) { _lexeme            .make(size, range);}
    template <> void make<Kind::ARGUMENTS        >(const uint32_t size, TRange<Kind::ARGUMENTS        >& range) { _arguments         .make(size, range);}
    template <> void make<Kind::SCOPE            >(const uint32_t size, TRange<Kind::SCOPE            >& range) { _scope             .make(size, range);}
    template <> void make<Kind::TYPING_STATEMENT >(const uint32_t size, TRange<Kind::TYPING_STATEMENT >& range) { _typingStatement   .make(size, range);}
    template <> void make<Kind::RETURN_TYPE      >(const uint32_t size, TRange<Kind::RETURN_TYPE      >& range) { _returnType        .make(size, range);}


    pool::TSub<Kind::FUNCTION          , LOG2_SIZE> _function;
    pool::TSub<Kind::LEXEME            , LOG2_SIZE> _lexeme;
    pool::TSub<Kind::ARGUMENTS         , LOG2_SIZE> _arguments;
    pool::TSub<Kind::SCOPE             , LOG2_SIZE> _scope;
    pool::TSub<Kind::TYPING_STATEMENT  , LOG2_SIZE> _typingStatement;
    pool::TSub<Kind::RETURN_TYPE       , LOG2_SIZE> _returnType;
};

} // namespace node
} // namespace ast
} // namespace dmit

