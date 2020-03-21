#include "dmit/prs/combinator.hpp"
#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/lex/token.hpp"

#include <vector>

namespace dmit
{

namespace prs
{

void State::clear()
{
    _tree     .clear();
    _errorSet .clear();
}

namespace state
{

namespace
{

auto makeParser(parser::Pool& parserPool, State& state)
{
    return parserPool.make(state);
}

template <com::TEnumIntegerType<lex::Token> TOKEN>
auto makeParserToken(parser::Pool& parserPool, State& state)
{
    return parserPool.make
    <
        error::token_check::Open<TOKEN>,
        error::token_check::Close
    >
    (state);
}

template <com::TEnumIntegerType<lex::Token       > TOKEN,
          com::TEnumIntegerType<tree::node::Kind > TREE_NODE_KIND>
auto makeParserTokenUnary(parser::Pool& parserPool, State& state)
{
    return parserPool.make
    <
        open::TPipeline
        <
            tree::writer::Open,
            error::token_check::Open<TOKEN>
        >,
        close::TPipeline
        <
            tree::writer::Close
            <
                tree::node::Arity::ONE,
                TREE_NODE_KIND
            >,
            error::token_check::Close
        >
    >
    (state);
}

template <com::TEnumIntegerType<tree::node::Kind > TREE_NODE_KIND>
auto makeParserUnary(parser::Pool& parserPool, State& state)
{
    return parserPool.make
    <
        tree::writer::Open,
        tree::writer::Close
        <
            tree::node::Arity::ONE,
            TREE_NODE_KIND
        >
    >(state);
}

template <com::TEnumIntegerType<tree::node::Kind > TREE_NODE_KIND>
auto makeParserVariadic(parser::Pool& parserPool, State& state)
{
    return parserPool.make
    <
        tree::writer::Open,
        tree::writer::Close
        <
            tree::node::Arity::VARIADIC,
            TREE_NODE_KIND
        >
    >(state);
}

} // namespace

#define TOKEN_TREE_NODE_KIND_PAIR(x) lex::Token::x, tree::node::Kind::x

Builder::Builder() :
    _parser{_parserPool.make<open::TPipeline<>, error::clear::Close>(_state)}
{
    auto integer    = makeParserTokenUnary<TOKEN_TREE_NODE_KIND_PAIR(INTEGER    )> (_parserPool, _state);
    auto decimal    = makeParserTokenUnary<TOKEN_TREE_NODE_KIND_PAIR(DECIMAL    )> (_parserPool, _state);
    auto identifier = makeParserTokenUnary<TOKEN_TREE_NODE_KIND_PAIR(IDENTIFIER )> (_parserPool, _state);
    auto plus       = makeParserToken<lex::Token::PLUS                           > (_parserPool, _state);
    auto minus      = makeParserToken<lex::Token::MINUS                          > (_parserPool, _state);
    auto star       = makeParserToken<lex::Token::STAR                           > (_parserPool, _state);
    auto slash      = makeParserToken<lex::Token::SLASH                          > (_parserPool, _state);
    auto parLeft    = makeParserToken<lex::Token::PAR_LEFT                       > (_parserPool, _state);
    auto parRight   = makeParserToken<lex::Token::PAR_RIGHT                      > (_parserPool, _state);
    auto dot        = makeParserToken<lex::Token::DOT                            > (_parserPool, _state);
    auto comma      = makeParserToken<lex::Token::COMMA                          > (_parserPool, _state);
    auto colon      = makeParserToken<lex::Token::COLON                          > (_parserPool, _state);
    auto semiColon  = makeParserToken<lex::Token::SEMI_COLON                     > (_parserPool, _state);
    auto equal      = makeParserToken<lex::Token::EQUAL                          > (_parserPool, _state);
    auto keyIf      = makeParserToken<lex::Token::IF                             > (_parserPool, _state);
    auto keyElse    = makeParserToken<lex::Token::ELSE                           > (_parserPool, _state);
    auto keyLet     = makeParserToken<lex::Token::LET                            > (_parserPool, _state);
    auto keyVar     = makeParserToken<lex::Token::VAR                            > (_parserPool, _state);
    auto keyFunc    = makeParserToken<lex::Token::FUNC                           > (_parserPool, _state);
    auto keyWhile   = makeParserToken<lex::Token::WHILE                          > (_parserPool, _state);
    auto keyReturn  = makeParserToken<lex::Token::RETURN                         > (_parserPool, _state);
    auto negAtom    = makeParserUnary    <tree::node::Kind::OPPOSE               > (_parserPool, _state);
    auto divitive   = makeParserUnary    <tree::node::Kind::INVERSE              > (_parserPool, _state);
    auto product    = makeParserVariadic <tree::node::Kind::PRODUCT              > (_parserPool, _state);
    auto negative   = makeParserUnary    <tree::node::Kind::OPPOSE               > (_parserPool, _state);
    auto sum        = makeParserVariadic <tree::node::Kind::SUM                  > (_parserPool, _state);
    auto assignment = makeParserVariadic <tree::node::Kind::ASSIGNMENT           > (_parserPool, _state);
    auto typInfer   = makeParserUnary    <tree::node::Kind::TYP_INFER            > (_parserPool, _state);
    auto declarLet  = makeParserVariadic <tree::node::Kind::DECLAR_LET           > (_parserPool, _state);
    auto listDisp   = makeParserVariadic <tree::node::Kind::LIST_DISP            > (_parserPool, _state);
    auto declarVar  = makeParserUnary    <tree::node::Kind::DECLAR_VAR           > (_parserPool, _state);
    auto listArg    = makeParserUnary    <tree::node::Kind::LIST_ARG             > (_parserPool, _state);
    auto staReturn  = makeParserUnary    <tree::node::Kind::STA_RETURN           > (_parserPool, _state);
    auto term       = makeParser                                                   (_parserPool, _state);
    auto atom       = makeParser                                                   (_parserPool, _state);
    auto mulitive   = makeParser                                                   (_parserPool, _state);
    auto additive   = makeParser                                                   (_parserPool, _state);
    auto posAtom    = makeParser                                                   (_parserPool, _state);
    auto typAnnot   = makeParser                                                   (_parserPool, _state);
    auto disp       = makeParser                                                   (_parserPool, _state);
    auto arg        = makeParser                                                   (_parserPool, _state);
    auto args       = makeParser                                                   (_parserPool, _state);

    USING_COMBINATORS;

    integer    = tok(lex::Token::INTEGER    );
    decimal    = tok(lex::Token::DECIMAL    );
    identifier = tok(lex::Token::IDENTIFIER );
    plus       = tok(lex::Token::PLUS       );
    minus      = tok(lex::Token::MINUS      );
    star       = tok(lex::Token::STAR       );
    slash      = tok(lex::Token::SLASH      );
    parLeft    = tok(lex::Token::PAR_LEFT   );
    parRight   = tok(lex::Token::PAR_RIGHT  );
    dot        = tok(lex::Token::DOT        );
    comma      = tok(lex::Token::COMMA      );
    colon      = tok(lex::Token::COLON      );
    semiColon  = tok(lex::Token::SEMI_COLON );
    equal      = tok(lex::Token::EQUAL      );
    keyIf      = tok(lex::Token::IF         );
    keyElse    = tok(lex::Token::ELSE       );
    keyLet     = tok(lex::Token::LET        );
    keyVar     = tok(lex::Token::VAR        );
    keyFunc    = tok(lex::Token::FUNC       );
    keyWhile   = tok(lex::Token::WHILE      );
    keyReturn  = tok(lex::Token::RETURN     );

    // Argument list

    arg = alt(sum, integer, decimal);

    args = seq(arg, rep(seq(comma, arg)));

    listArg = seq(parLeft, opt(args), parRight);

    // Dispatch list

    disp = seq(identifier, opt(listArg));

    listDisp = seq(disp, rep(seq(dot, disp)));

    // Expression

    term = alt(listDisp, integer, decimal);

    posAtom = seq(alt(term, seq(parLeft, sum, parRight)));

    negAtom = seq(minus, posAtom);

    atom = alt(posAtom, negAtom);

    mulitive = seq(star  , atom);
    divitive = seq(slash , atom);

    product = seq(atom, rep(alt(mulitive,
                                divitive)));

    additive = seq(plus  , product);
    negative = seq(minus , product);

    sum = seq(product, rep(alt(additive,
                               negative)));
    // Assigment

    assignment = seq(identifier, equal, sum);

    // Var declaration

    typAnnot = seq(colon, identifier);
    typInfer = seq();

    declarVar = seq(keyVar, identifier, alt(typAnnot,
                                            typInfer), opt(seq(equal, sum)));

    // Let declaration

    declarLet = seq(keyLet, identifier, alt(typAnnot,
                                            typInfer), equal, sum);

    // Return statement

    staReturn = seq(keyReturn, sum);

    // Full parser

    _parser = alt(declarLet, declarVar, staReturn, assignment, sum);
}

const State& Builder::operator()(const std::vector<lex::Token>& tokens)
{
    Reader reader(tokens);

    _parser(reader);

    return _state;
}

void Builder::clearState()
{
    _state.clear();
}

} // namespace state
} // namespace prs
} // namespace dmit
