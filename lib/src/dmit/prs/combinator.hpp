#pragma once

#include "dmit/lex/reader.hpp"
#include "dmit/lex/token.hpp"

#include "dmit/com/enum.hpp"

#include <optional>

namespace dmit
{

namespace prs
{

namespace combinator
{

auto tok(const com::TEnumIntegerType<lex::Token> token)
{
    return [token](lex::Reader reader) -> std::optional<lex::Reader>
    {
        if (reader.look() != token)
        {
            return std::nullopt;
        }

        reader.advance();

        return reader;
    };
}

auto seq()
{
    return [](lex::Reader reader) -> std::optional<lex::Reader> { return reader; };
}

template <class Parser, class... Parsers>
auto seq(Parser&& parser, Parsers&&... parsers)
{
    return [parser, parsers...](lex::Reader reader) -> std::optional<lex::Reader>
    {
        const auto& readerOpt = parser(reader);

        if (!readerOpt)
        {
            return std::nullopt;
        }

        return seq(parsers...)(readerOpt.value());
    };
}

auto alt()
{
    return [](lex::Reader reader) -> std::optional<lex::Reader>
    {
        return std::nullopt;
    };
}

template <class Parser, class... Parsers>
auto alt(Parser&& parser, Parsers&&... parsers)
{
    return [parser, parsers...](lex::Reader reader) -> std::optional<lex::Reader>
    {
        const auto& readerOpt = parser(reader);

        if (!readerOpt)
        {
            return alt(parsers...)(reader);
        }

        return readerOpt;
    };
}

template <class Parser>
auto rep(Parser&& parser)
{
    return [parser](lex::Reader reader) -> std::optional<lex::Reader>
    {
        std::optional<lex::Reader> readerOpt{reader};

        while ((readerOpt = parser(readerOpt.value())))
        {
            reader = readerOpt.value();
        }

        return reader;
    };
}

template <class Parser>
auto opt(Parser&& parser)
{
    return [parser](lex::Reader reader) -> std::optional<lex::Reader>
    {
        const auto& readerOpt = parser(reader);

        return readerOpt ? readerOpt : reader;
    };
}

} // namespace combinator

#define USING_COMBINATORS \
  using combinator::tok;  \
  using combinator::seq;  \
  using combinator::rep;  \
  using combinator::alt;  \
  using combinator::opt;

} // namespace prs
} // namespace dmit
