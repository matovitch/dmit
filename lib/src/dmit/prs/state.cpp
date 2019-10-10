#include "dmit/prs/state.hpp"

#include "dmit/prs/combinator.hpp"
#include "dmit/prs/parser.hpp"
#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

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

using namespace combinator;

Builder::Builder() :
    _parser{_poolParser.make(_state)}
{
    auto integer    = _poolParser.make(_state);
    auto decimal    = _poolParser.make(_state);
    auto identifier = _poolParser.make(_state);
    auto plus       = _poolParser.make(_state);
    auto minus      = _poolParser.make(_state);
    auto star       = _poolParser.make(_state);
    auto slash      = _poolParser.make(_state);
    auto parLeft    = _poolParser.make(_state);
    auto parRight   = _poolParser.make(_state);
    auto dot        = _poolParser.make(_state);
    auto comma      = _poolParser.make(_state);
    auto colon      = _poolParser.make(_state);
    auto semiColon  = _poolParser.make(_state);
    auto equal      = _poolParser.make(_state);
    auto keyIf      = _poolParser.make(_state);
    auto keyElse    = _poolParser.make(_state);
    auto keyLet     = _poolParser.make(_state);
    auto keyVar     = _poolParser.make(_state);
    auto keyFunc    = _poolParser.make(_state);
    auto keyWhile   = _poolParser.make(_state);
    auto keyReturn  = _poolParser.make(_state);
    auto term       = _poolParser.make(_state);
    auto posAtom    = _poolParser.make(_state);
    auto negAtom    = _poolParser.make(_state);
    auto atom       = _poolParser.make(_state);
    auto mulitive   = _poolParser.make(_state);
    auto divitive   = _poolParser.make(_state);
    auto product    = _poolParser.make(_state);
    auto additive   = _poolParser.make(_state);
    auto negative   = _poolParser.make(_state);
    auto sum        = _poolParser.make(_state);
    auto assignment = _poolParser.make(_state);
    auto typInfer   = _poolParser.make(_state);
    auto typAnnot   = _poolParser.make(_state);
    auto declarLet  = _poolParser.make(_state);
    auto declarVar  = _poolParser.make(_state);
    auto disp       = _poolParser.make(_state);
    auto listDisp   = _poolParser.make(_state);
    auto arg        = _poolParser.make(_state);
    auto args       = _poolParser.make(_state);
    auto listArg    = _poolParser.make(_state);
    auto staReturn  = _poolParser.make(_state);

    _poolSubscriber.bind<subscriber::tree::Writer>(integer    , state::tree::node::Kind::INTEGER    , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(decimal    , state::tree::node::Kind::DECIMAL    , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(negAtom    , state::tree::node::Kind::OPPOSE     , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(divitive   , state::tree::node::Kind::INVERSE    , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(product    , state::tree::node::Kind::PRODUCT    , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(negative   , state::tree::node::Kind::OPPOSE     , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(sum        , state::tree::node::Kind::SUM        , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(identifier , state::tree::node::Kind::IDENTIFIER , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(assignment , state::tree::node::Kind::ASSIGNMENT , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(typInfer   , state::tree::node::Kind::TYP_INFER  , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(declarLet  , state::tree::node::Kind::DECLAR_LET , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(declarVar  , state::tree::node::Kind::DECLAR_VAR , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(listArg    , state::tree::node::Kind::LIST_ARG   , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(listDisp   , state::tree::node::Kind::LIST_DISP  , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(staReturn  , state::tree::node::Kind::STA_RETURN , state::tree::node::Arity::ONE      );

    _poolSubscriber.bind<subscriber::error::TokChecker>(integer    , lex::Token::INTEGER     );
    _poolSubscriber.bind<subscriber::error::TokChecker>(decimal    , lex::Token::DECIMAL     );
    _poolSubscriber.bind<subscriber::error::TokChecker>(identifier , lex::Token::IDENTIFIER  );
    _poolSubscriber.bind<subscriber::error::TokChecker>(plus       , lex::Token::PLUS        );
    _poolSubscriber.bind<subscriber::error::TokChecker>(minus      , lex::Token::MINUS       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(star       , lex::Token::STAR        );
    _poolSubscriber.bind<subscriber::error::TokChecker>(slash      , lex::Token::SLASH       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(parLeft    , lex::Token::PAR_LEFT    );
    _poolSubscriber.bind<subscriber::error::TokChecker>(parRight   , lex::Token::PAR_RIGHT   );
    _poolSubscriber.bind<subscriber::error::TokChecker>(dot        , lex::Token::DOT         );
    _poolSubscriber.bind<subscriber::error::TokChecker>(comma      , lex::Token::COMMA       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(colon      , lex::Token::COLON       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(semiColon  , lex::Token::SEMI_COLON  );
    _poolSubscriber.bind<subscriber::error::TokChecker>(equal      , lex::Token::EQUAL       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyIf      , lex::Token::IF          );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyElse    , lex::Token::ELSE        );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyLet     , lex::Token::LET         );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyVar     , lex::Token::VAR         );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyFunc    , lex::Token::FUNC        );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyWhile   , lex::Token::WHILE       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(keyReturn  , lex::Token::RETURN      );

    _poolSubscriber.bind<subscriber::error::Clear>(_parser);

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
