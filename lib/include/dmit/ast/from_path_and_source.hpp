#pragma once

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/storage.hpp"
#include "dmit/lex/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/src/file.hpp"

#include <filesystem>
#include <cstdint>

namespace dmit::ast
{

class FromPathAndSource
{

public:

    State make(const std::filesystem::path  & path,
               const com::TStorage<uint8_t> & source);

private:

    lex::state::Builder _lexer;
    prs::state::Builder _parser;
    ast::state::Builder _aster;

    SourceRegister _sourceRegister;
};

} // namespace dmit::ast
