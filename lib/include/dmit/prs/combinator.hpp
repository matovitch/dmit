#pragma once

#include "dmit/prs/reader.hpp"

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
    return [token](Reader reader) -> std::optional<Reader>
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
    return [](Reader reader) -> std::optional<Reader> { return reader; };
}

template <class Parser, class... Parsers>
auto seq(Parser&& parser, Parsers&&... parsers)
{
    return [parser, parsers...](Reader reader) -> std::optional<Reader>
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
    return [](Reader reader) -> std::optional<Reader>
    {
        return std::nullopt;
    };
}

template <class Parser, class... Parsers>
auto alt(Parser&& parser, Parsers&&... parsers)
{
    return [parser, parsers...](Reader reader) -> std::optional<Reader>
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
    return [parser](Reader reader) -> std::optional<Reader>
    {
        std::optional<Reader> readerOpt{reader};

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
    return [parser](Reader reader) -> std::optional<Reader>
    {
        const auto& readerOpt = parser(reader);

        return readerOpt ? readerOpt : reader;
    };
}

} // namespace combinators

} // namespace prs

} // namespace dmit