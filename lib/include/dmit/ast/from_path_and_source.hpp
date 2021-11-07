#pragma once

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/lex/state.hpp"
#include "dmit/prs/state.hpp"

#include <cstdint>
#include <vector>

namespace dmit::ast
{

class FromPathAndSource
{

public:

    State make(const std::vector<uint8_t>& path,
               const std::vector<uint8_t>& source);
private:

    lex::state::Builder _lexer;
    prs::state::Builder _parser;
    ast::state::Builder _aster;

    SourceRegister _sourceRegister;
};

} // namespace dmit::ast
