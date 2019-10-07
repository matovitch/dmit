#include "dmit/prs/state.hpp"
#include "dmit/prs/error.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/lex/state.hpp"
#include "dmit/lex/token.hpp"

#include "dmit/fmt/prs/error.hpp"
#include "dmit/fmt/prs/tree.hpp"

#include "doctest/doctest_fwd.h"

#include <iostream>
#include <vector>

class Parser
{

public:

    const dmit::prs::State& operator()(const std::string& toParse)
    {
        _parser .clearState();
        _lexer  .clearState();

        const auto& tokens = _lexer(reinterpret_cast<const uint8_t*>(toParse.data()),
                                                                     toParse.size())._tokens;
        return _parser(tokens);
    } 

private:

    dmit::prs::state::Builder _parser;
    dmit::lex::state::Builder _lexer;
};

TEST_CASE("dmit::prs::State parse(const std::string& toParse)")
{
    Parser parser;

    std::cout << "[{\"tree\":"    << parser("3*(1-9)")._tree
              << ",\"errorSet\":" << parser("3*(1-9)")._errorSet;

    std::cout << "},{\"tree\":"   << parser("a=5+3")._tree;
    std::cout << ",\"errorSet\":" << parser("x=6+2")._errorSet << "}]"; 
}
