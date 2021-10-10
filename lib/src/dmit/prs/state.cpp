#include "dmit/prs/combinator.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/lex/reader.hpp"
#include "dmit/lex/token.hpp"

#include <vector>

namespace dmit::prs
{

void State::clear()
{
    _tree   .clear();
    _errors .clear();
}

namespace state
{

namespace
{

auto makeParser(parser::Pool& pool, State& state)
{
    return pool.make(state);
}

template <com::TEnumIntegerType<lex::Token> TOKEN>
auto makeParserToken(parser::Pool& pool, State& state)
{
    return pool.make
    <
        error::token_check::Open<TOKEN>,
        error::token_check::Close
    >
    (state);
}

auto makeParserRecoverable(parser::Pool& pool, State& state)
{
    return pool.make
    <
        error::recoverable::Open,
        error::recoverable::Close
    >
    (state);
}

template <com::TEnumIntegerType<lex::Token       > TOKEN,
          com::TEnumIntegerType<tree::node::Kind > TREE_NODE_KIND>
auto makeParserTokenUnary(parser::Pool& pool, State& state)
{
    return pool.make
    <
        open::TPipeline
        <
            tree::writer::Open<TREE_NODE_KIND>,
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
auto makeParserUnary(parser::Pool& pool, State& state)
{
    return pool.make
    <
        tree::writer::Open<TREE_NODE_KIND>,
        tree::writer::Close
        <
            tree::node::Arity::ONE,
            TREE_NODE_KIND
        >
    >(state);
}

template <com::TEnumIntegerType<tree::node::Kind > TREE_NODE_KIND>
auto makeParserVariadic(parser::Pool& pool, State& state)
{
    return pool.make
    <
        tree::writer::Open<TREE_NODE_KIND>,
        tree::writer::Close
        <
            tree::node::Arity::VARIADIC,
            TREE_NODE_KIND
        >
    >(state);
}

} // namespace

#define TOKEN_TREE_NODE_KIND_PAIR(x) lex::Token::x, tree::node::Kind::LIT_##x

Builder::Builder() :
    _parser{_pool.make(_state)}
{
    auto integer       = makeParserTokenUnary <TOKEN_TREE_NODE_KIND_PAIR(INTEGER    )> (_pool, _state);
    auto decimal       = makeParserTokenUnary <TOKEN_TREE_NODE_KIND_PAIR(DECIMAL    )> (_pool, _state);
    auto identifier    = makeParserTokenUnary <TOKEN_TREE_NODE_KIND_PAIR(IDENTIFIER )> (_pool, _state);
    auto plus          = makeParserToken      <lex::Token::PLUS                      > (_pool, _state);
    auto minus         = makeParserToken      <lex::Token::MINUS                     > (_pool, _state);
    auto star          = makeParserToken      <lex::Token::STAR                      > (_pool, _state);
    auto slash         = makeParserToken      <lex::Token::SLASH                     > (_pool, _state);
    auto percent       = makeParserToken      <lex::Token::PERCENT                   > (_pool, _state);
    auto parLeft       = makeParserToken      <lex::Token::PAR_LEFT                  > (_pool, _state);
    auto parRight      = makeParserToken      <lex::Token::PAR_RIGHT                 > (_pool, _state);
    auto braLeft       = makeParserToken      <lex::Token::BRA_LEFT                  > (_pool, _state);
    auto braRight      = makeParserToken      <lex::Token::BRA_RIGHT                 > (_pool, _state);
    auto ketLeft       = makeParserToken      <lex::Token::KET_LEFT                  > (_pool, _state);
    auto ketRight      = makeParserToken      <lex::Token::KET_RIGHT                 > (_pool, _state);
    auto ketLeftEqual  = makeParserToken      <lex::Token::KET_LEFT_EQUAL            > (_pool, _state);
    auto ketRightEqual = makeParserToken      <lex::Token::KET_RIGHT_EQUAL           > (_pool, _state);
    auto dot           = makeParserToken      <lex::Token::DOT                       > (_pool, _state);
    auto comma         = makeParserToken      <lex::Token::COMMA                     > (_pool, _state);
    auto colon         = makeParserToken      <lex::Token::COLON                     > (_pool, _state);
    auto semiColon     = makeParserToken      <lex::Token::SEMI_COLON                > (_pool, _state);
    auto equal         = makeParserToken      <lex::Token::EQUAL                     > (_pool, _state);
    auto plusEqual     = makeParserToken      <lex::Token::PLUS_EQUAL                > (_pool, _state);
    auto minusEqual    = makeParserToken      <lex::Token::MINUS_EQUAL               > (_pool, _state);
    auto starEqual     = makeParserToken      <lex::Token::STAR_EQUAL                > (_pool, _state);
    auto slashEqual    = makeParserToken      <lex::Token::SLASH_EQUAL               > (_pool, _state);
    auto percentEqual  = makeParserToken      <lex::Token::PERCENT_EQUAL             > (_pool, _state);
    auto keyIf         = makeParserToken      <lex::Token::IF                        > (_pool, _state);
    auto keyElse       = makeParserToken      <lex::Token::ELSE                      > (_pool, _state);
    auto keyLet        = makeParserToken      <lex::Token::LET                       > (_pool, _state);
    auto keyFunc       = makeParserToken      <lex::Token::FUNC                      > (_pool, _state);
    auto keyClass      = makeParserToken      <lex::Token::CLASS                     > (_pool, _state);
    auto keyWhile      = makeParserToken      <lex::Token::WHILE                     > (_pool, _state);
    auto keyExport     = makeParserToken      <lex::Token::EXPORT                    > (_pool, _state);
    auto keyImport     = makeParserToken      <lex::Token::IMPORT                    > (_pool, _state);
    auto keyModule     = makeParserToken      <lex::Token::MODULE                    > (_pool, _state);
    auto keyReturn     = makeParserToken      <lex::Token::RETURN                    > (_pool, _state);
    auto minusKetRight = makeParserToken      <lex::Token::MINUS_KET_RIGHT           > (_pool, _state);
    auto opSum         = makeParserUnary      <tree::node::Kind::EXP_OPERATOR        > (_pool, _state);
    auto opProduct     = makeParserUnary      <tree::node::Kind::EXP_OPERATOR        > (_pool, _state);
    auto opComparison  = makeParserUnary      <tree::node::Kind::EXP_OPERATOR        > (_pool, _state);
    auto opAssign      = makeParserUnary      <tree::node::Kind::EXP_OPERATOR        > (_pool, _state);
    auto opPath        = makeParserUnary      <tree::node::Kind::EXP_OPERATOR        > (_pool, _state);
    auto negAtom       = makeParserUnary      <tree::node::Kind::EXP_OPPOSE          > (_pool, _state);
    auto funCall       = makeParserUnary      <tree::node::Kind::FUN_CALL            > (_pool, _state);
    auto funArguments  = makeParserUnary      <tree::node::Kind::FUN_ARGUMENTS       > (_pool, _state);
    auto stmReturn     = makeParserUnary      <tree::node::Kind::STM_RETURN          > (_pool, _state);
    auto dclVariable   = makeParserUnary      <tree::node::Kind::DCL_VARIABLE        > (_pool, _state);
    auto rawExport     = makeParserUnary      <tree::node::Kind::DCL_EXPORT          > (_pool, _state);
    auto rawImport     = makeParserUnary      <tree::node::Kind::DCL_IMPORT          > (_pool, _state);
    auto clsMembers    = makeParserUnary      <tree::node::Kind::CLS_MEMBERS         > (_pool, _state);
    auto rawScope      = makeParserUnary      <tree::node::Kind::SCOPE               > (_pool, _state);
    auto rawClass      = makeParserUnary      <tree::node::Kind::DEF_CLASS           > (_pool, _state);
    auto rawFunction   = makeParserUnary      <tree::node::Kind::DEF_FUNCTION        > (_pool, _state);
    auto rawDefinition = makeParserUnary      <tree::node::Kind::DEFINITION          > (_pool, _state);
    auto rawModule     = makeParserUnary      <tree::node::Kind::MODULE              > (_pool, _state);
    auto rootModule    = makeParserUnary      <tree::node::Kind::MODULE              > (_pool, _state);
    auto scope         = makeParserVariadic   <tree::node::Kind::SCOPE               > (_pool, _state);
    auto product       = makeParserVariadic   <tree::node::Kind::EXP_BINOP           > (_pool, _state);
    auto sum           = makeParserVariadic   <tree::node::Kind::EXP_BINOP           > (_pool, _state);
    auto comparison    = makeParserVariadic   <tree::node::Kind::EXP_BINOP           > (_pool, _state);
    auto path          = makeParserVariadic   <tree::node::Kind::EXP_BINOP           > (_pool, _state);
    auto assignment    = makeParserVariadic   <tree::node::Kind::EXP_ASSIGN          > (_pool, _state);
    auto rcvScopeElem  = makeParserRecoverable                                         (_pool, _state);
    auto rcvScope      = makeParserRecoverable                                         (_pool, _state);
    auto rcvClass      = makeParserRecoverable                                         (_pool, _state);
    auto rcvFunction   = makeParserRecoverable                                         (_pool, _state);
    auto rcvDefinition = makeParserRecoverable                                         (_pool, _state);
    auto rcvImport     = makeParserRecoverable                                         (_pool, _state);
    auto rcvModule     = makeParserRecoverable                                         (_pool, _state);
    auto rawScopeElem  = makeParser                                                    (_pool, _state);
    auto skpScopeElem  = makeParser                                                    (_pool, _state);
    auto skpScope      = makeParser                                                    (_pool, _state);
    auto skpClass      = makeParser                                                    (_pool, _state);
    auto skpFunction   = makeParser                                                    (_pool, _state);
    auto skpDefinition = makeParser                                                    (_pool, _state);
    auto skpImport     = makeParser                                                    (_pool, _state);
    auto skpModule     = makeParser                                                    (_pool, _state);
    auto expression    = makeParser                                                    (_pool, _state);
    auto atom          = makeParser                                                    (_pool, _state);
    auto typeClaim     = makeParser                                                    (_pool, _state);
    auto posAtom       = makeParser                                                    (_pool, _state);

    USING_COMBINATORS;

    integer       = tok<lex::Token::INTEGER         >();
    decimal       = tok<lex::Token::DECIMAL         >();
    identifier    = tok<lex::Token::IDENTIFIER      >();
    plus          = tok<lex::Token::PLUS            >();
    minus         = tok<lex::Token::MINUS           >();
    star          = tok<lex::Token::STAR            >();
    slash         = tok<lex::Token::SLASH           >();
    percent       = tok<lex::Token::PERCENT         >();
    parLeft       = tok<lex::Token::PAR_LEFT        >();
    braLeft       = tok<lex::Token::BRA_LEFT        >();
    parRight      = tok<lex::Token::PAR_RIGHT       >();
    braRight      = tok<lex::Token::BRA_RIGHT       >();
    ketLeft       = tok<lex::Token::KET_LEFT        >();
    ketRight      = tok<lex::Token::KET_RIGHT       >();
    ketLeftEqual  = tok<lex::Token::KET_LEFT_EQUAL  >();
    ketRightEqual = tok<lex::Token::KET_RIGHT_EQUAL >();
    dot           = tok<lex::Token::DOT             >();
    comma         = tok<lex::Token::COMMA           >();
    colon         = tok<lex::Token::COLON           >();
    semiColon     = tok<lex::Token::SEMI_COLON      >();
    equal         = tok<lex::Token::EQUAL           >();
    plusEqual     = tok<lex::Token::PLUS_EQUAL      >();
    minusEqual    = tok<lex::Token::MINUS_EQUAL     >();
    starEqual     = tok<lex::Token::STAR_EQUAL      >();
    slashEqual    = tok<lex::Token::SLASH_EQUAL     >();
    percentEqual  = tok<lex::Token::PERCENT_EQUAL   >();
    keyIf         = tok<lex::Token::IF              >();
    keyElse       = tok<lex::Token::ELSE            >();
    keyLet        = tok<lex::Token::LET             >();
    keyFunc       = tok<lex::Token::FUNC            >();
    keyClass      = tok<lex::Token::CLASS           >();
    keyWhile      = tok<lex::Token::WHILE           >();
    keyExport     = tok<lex::Token::EXPORT          >();
    keyImport     = tok<lex::Token::IMPORT          >();
    keyModule     = tok<lex::Token::MODULE          >();
    keyReturn     = tok<lex::Token::RETURN          >();
    minusKetRight = tok<lex::Token::MINUS_KET_RIGHT >();

    // Expression

    opPath = dot;

    path = seq(identifier, rep(seq(opPath, path)));

    posAtom = seq(opt(plus), alt(path, integer, decimal, seq(parLeft, expression, parRight)));

    negAtom = seq(minus, posAtom);

    atom = alt(posAtom, negAtom);

    opProduct = alt(star, slash, percent);

    product = seq(atom, rep(seq(opProduct, atom)));

    opSum = alt(plus, minus);

    sum = seq(product, rep(seq(opSum, product)));

    opComparison = alt(ketLeft, ketRight, ketLeftEqual, ketRightEqual);

    comparison = seq(sum, opt(seq(opComparison, sum)));

    opAssign = alt(equal, plusEqual, minusEqual, starEqual, slashEqual, percentEqual);

    assignment = seq(comparison, opt(seq(opAssign, assignment)));

    funCall = seq(identifier, parLeft, opt(seq(expression, rep(seq(comma, expression)))), parRight);

    expression = alt(funCall, assignment);

    // Return statement

    stmReturn = seq(keyReturn, expression);

    // Let declaration

    typeClaim = seq(identifier, colon, identifier);

    dclVariable = seq(keyLet, typeClaim);

    // Scope Element

    rawScopeElem = seq(alt(dclVariable,
                           stmReturn,
                           expression, seq()), semiColon);

    skpScopeElem = seq(skp(alt(tok<lex::Token::SEMI_COLON >(),
                               tok<lex::Token::BRA_LEFT   >(),
                               tok<lex::Token::BRA_RIGHT  >(),
                               tok<lex::Token::FUNC       >(),
                               tok<lex::Token::CLASS      >(),
                               tok<lex::Token::MODULE     >(),
                               tok<lex::Token::EXPORT     >(),
                               tok<lex::Token::IMPORT     >())), opt(tok<lex::Token::SEMI_COLON>()));

    rcvScopeElem = alt(rawScopeElem,
                       skpScopeElem);

    // Scope

    rawScope = seq(braLeft, rep(alt(rcvScopeElem, rawScope)), braRight);

    skpScope = skp(alt(tok<lex::Token::FUNC   >(),
                       tok<lex::Token::CLASS  >(),
                       tok<lex::Token::MODULE >(),
                       tok<lex::Token::EXPORT >(),
                       tok<lex::Token::IMPORT >()));

    rcvScope = alt(rawScope,
                   skpScope);

    scope = rcvScope; // Only here to create a fake scope if we could not parse one

    // Function declaration

    funArguments = seq(parLeft, opt(seq(typeClaim, rep(seq(comma, typeClaim)))), parRight);

    rawFunction = seq(keyFunc, identifier, funArguments, opt(seq(minusKetRight, identifier)), scope);

    skpFunction = seq(opt(tok<lex::Token::FUNC>()), skp(alt(tok<lex::Token::FUNC>(),
                                                            tok<lex::Token::CLASS>(),
                                                            tok<lex::Token::MODULE>(),
                                                            tok<lex::Token::EXPORT>(),
                                                            tok<lex::Token::IMPORT>(),
                                                            tok<lex::Token::BRA_RIGHT>())));
    rcvFunction = alt(rawFunction,
                      skpFunction);

    // Class declaration

    clsMembers = seq(braLeft, rep(seq(typeClaim, semiColon)), braRight);

    rawClass = seq(keyClass, identifier, clsMembers);

    skpClass = seq(opt(tok<lex::Token::CLASS>()), skp(alt(tok<lex::Token::FUNC>(),
                                                          tok<lex::Token::CLASS>(),
                                                          tok<lex::Token::MODULE>(),
                                                          tok<lex::Token::EXPORT>(),
                                                          tok<lex::Token::IMPORT>(),
                                                          tok<lex::Token::BRA_RIGHT>())));
    rcvClass = alt(rawClass,
                   skpClass);

    rawExport = keyExport;

    rawDefinition = seq(opt(rawExport), alt(rcvFunction, rcvClass));

    skpDefinition = seq(opt(tok<lex::Token::EXPORT>()), skp(alt(tok<lex::Token::FUNC>(),
                                                                tok<lex::Token::CLASS>(),
                                                                tok<lex::Token::MODULE>(),
                                                                tok<lex::Token::EXPORT>(),
                                                                tok<lex::Token::IMPORT>(),
                                                                tok<lex::Token::BRA_RIGHT>())));
    rcvDefinition = alt(rawDefinition, skpDefinition);

    // Import

    rawImport = seq(keyImport, path, semiColon);

    skpImport = seq(opt(tok<lex::Token::IMPORT>()),
                    skp(alt(tok<lex::Token::SEMI_COLON >(),
                            tok<lex::Token::FUNC       >(),
                            tok<lex::Token::CLASS      >(),
                            tok<lex::Token::MODULE     >(),
                            tok<lex::Token::EXPORT     >(),
                            tok<lex::Token::IMPORT     >(),
                            tok<lex::Token::BRA_RIGHT  >())),
                    opt(tok<lex::Token::SEMI_COLON>()));

    rcvImport = alt(rawImport,
                    skpImport);
    // Module

    rawModule = seq(keyModule, path, alt(seq(semiColon , rep(alt(rcvDefinition, rcvImport, rawModule))),
                                         seq(braLeft   , rep(alt(rcvDefinition, rcvImport, rawModule)), braRight)));

    skpModule = seq(opt(tok<lex::Token::MODULE>()), skp(alt(tok<lex::Token::FUNC>(),
                                                            tok<lex::Token::CLASS>(),
                                                            tok<lex::Token::MODULE>(),
                                                            tok<lex::Token::EXPORT>(),
                                                            tok<lex::Token::IMPORT>())));
    rcvModule = alt(rawModule,
                    skpModule);
    // Full parser

    rootModule = rep(alt(rcvDefinition, rcvImport, rcvModule));

    _parser = rootModule;
}

const State& Builder::operator()(const std::vector<lex::Token>& tokens)
{
    lex::Reader reader(tokens);

    const auto& readerOpt = _parser(reader);

    if (readerOpt && readerOpt.value().isEoi())
    {
        _state._errors.cleanUp();
    }

    return _state;
}

void Builder::clearState()
{
    _state.clear();
}

} // namespace state
} // namespace dmit::prs
