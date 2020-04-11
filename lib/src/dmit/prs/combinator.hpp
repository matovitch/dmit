#pragma once

#include "dmit/lex/reader.hpp"
#include "dmit/lex/token.hpp"


#include "dmit/fmt/lex/token.hpp"
#include "dmit/com/logger.hpp"
#include "dmit/com/enum.hpp"

#include <optional>

namespace dmit::prs
{

namespace combinator
{

template <com::TEnumIntegerType<lex::Token> TOKEN>
auto tok()
{
    return [](lex::Reader reader) -> std::optional<lex::Reader>
    {
        if (reader.look() != TOKEN)
        {
            return std::nullopt;
        }

        reader.advance();

        return reader;
    };
}

auto err()
{
    return [](lex::Reader reader) -> std::optional<lex::Reader> { return std::nullopt; };
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

template <class Parser>
auto skp(Parser&& parser)
{
    return [parser](lex::Reader reader) -> std::optional<lex::Reader>
    {
        reader.advance();
        reader.advanceToRawToken();

        while (!reader.isEoi())
        {
            const auto readerOpt = parser(reader);

            if (readerOpt)
            {
                return reader;
            }

            reader.advance();
            reader.advanceToRawToken();
        }

        return reader;
    };
}

template <class Parser>
auto til(Parser&& parser)
{
    return [parser](lex::Reader reader) -> std::optional<lex::Reader>
    {
        if (parser(reader))
        {
            return std::nullopt;
        }

        return skp(parser)(reader);
    };
}

template <class Parser>
auto up2(Parser&& parser)
{
    return [parser](lex::Reader reader) -> std::optional<lex::Reader>
    {
        if (reader.isEoi())
        {
            return std::nullopt;
        }

        return skp(parser)(reader);
    };
}

auto msg(const char* const message)
{
    return [message](lex::Reader reader) -> std::optional<lex::Reader>
    {
        DMIT_COM_LOG_OUT << message << ':' << reader.look() << ':' << reader.offset() << '\n';
        return std::nullopt;
    };
}

} // namespace combinator

#define USING_COMBINATORS \
  using combinator::tok;  \
  using combinator::err;  \
  using combinator::seq;  \
  using combinator::rep;  \
  using combinator::alt;  \
  using combinator::opt;  \
  using combinator::til;  \
  using combinator::up2;  \
  using combinator::msg;  \

} // namespace dmit::prs
