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
    auto parenL     = _poolParser.make(_state);
    auto parenR     = _poolParser.make(_state);
    auto dot        = _poolParser.make(_state);
    auto equal      = _poolParser.make(_state);
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

    auto& variable   = identifier;
    auto& expression = sum;

    _poolSubscriber.bind<subscriber::tree::Writer>(term       , state::tree::node::Kind::TERM       , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(negAtom    , state::tree::node::Kind::OPPOSE     , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(divitive   , state::tree::node::Kind::INVERSE    , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(product    , state::tree::node::Kind::PRODUCT    , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(negative   , state::tree::node::Kind::OPPOSE     , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(sum        , state::tree::node::Kind::SUM        , state::tree::node::Arity::VARIADIC );
    _poolSubscriber.bind<subscriber::tree::Writer>(variable   , state::tree::node::Kind::VARIABLE   , state::tree::node::Arity::ONE      );
    _poolSubscriber.bind<subscriber::tree::Writer>(assignment , state::tree::node::Kind::ASSIGNMENT , state::tree::node::Arity::VARIADIC );

    _poolSubscriber.bind<subscriber::error::TokChecker>(integer    , lex::Token::INTEGER    );
    _poolSubscriber.bind<subscriber::error::TokChecker>(decimal    , lex::Token::DECIMAL    );
    _poolSubscriber.bind<subscriber::error::TokChecker>(identifier , lex::Token::IDENTIFIER );
    _poolSubscriber.bind<subscriber::error::TokChecker>(plus       , lex::Token::PLUS       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(minus      , lex::Token::MINUS      );
    _poolSubscriber.bind<subscriber::error::TokChecker>(star       , lex::Token::STAR       );
    _poolSubscriber.bind<subscriber::error::TokChecker>(slash      , lex::Token::SLASH      );
    _poolSubscriber.bind<subscriber::error::TokChecker>(parenL     , lex::Token::PAREN_L    );
    _poolSubscriber.bind<subscriber::error::TokChecker>(parenR     , lex::Token::PAREN_R    );
    _poolSubscriber.bind<subscriber::error::TokChecker>(dot        , lex::Token::DOT        );
    _poolSubscriber.bind<subscriber::error::TokChecker>(equal      , lex::Token::EQUAL      );

    _poolSubscriber.bind<subscriber::error::Clear>(_parser);

    integer    = tok(lex::Token::INTEGER   );
    decimal    = tok(lex::Token::DECIMAL   );
    identifier = tok(lex::Token::IDENTIFIER);
    plus       = tok(lex::Token::PLUS      );
    minus      = tok(lex::Token::MINUS     );
    star       = tok(lex::Token::STAR      );
    slash      = tok(lex::Token::SLASH     );
    parenL     = tok(lex::Token::PAREN_L   );
    parenR     = tok(lex::Token::PAREN_R   );
    dot        = tok(lex::Token::DOT       );
    equal      = tok(lex::Token::EQUAL     );

    term = alt(integer, decimal, identifier);

    posAtom = seq(alt(term, seq(parenL, sum, parenR)));

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
    // alias expression = sum
    // alias variable   = identifier

    assignment = seq(variable, equal, expression);

    _parser = alt(assignment, expression);
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
