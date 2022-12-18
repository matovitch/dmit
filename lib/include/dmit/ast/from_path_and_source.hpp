#pragma once

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/lex/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/src/file.hpp"

#include "dmit/com/constant_reference.hpp"

#include <cstdint>
#include <vector>

namespace dmit::ast
{

class FromPathAndSource
{

public:

    State make(const src::File& source);

private:

    lex::state::Builder _lexer;
    prs::state::Builder _parser;
    ast::state::Builder _aster;

    SourceRegister _sourceRegister;
};

} // namespace dmit::ast
