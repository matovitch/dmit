#include "dmit/ast/from_path_and_source.hpp"

#include "dmit/src/file.hpp"

#include "dmit/fmt/logger.hpp"

#include "ketopt/ketopt.h"

#include <cstring>

enum
{
    K_OPTION_INVALID     = ':'   ,
    K_OPTION_HELP        = 'h'   ,
    K_OPTION_VERSION     = 'v'   ,
    K_OPTION_FILE_PATH   = 'f'   ,
    K_OPTION_LEX         = 0x100 , // These options have no char representation
    K_OPTION_PRS                 ,
    K_OPTION_AST
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"        , ko_no_argument       , K_OPTION_HELP        },
    { "version"     , ko_no_argument       , K_OPTION_VERSION     },
    { "file-path"   , ko_required_argument , K_OPTION_FILE_PATH   },
    { "lex"         , ko_no_argument       , K_OPTION_LEX         },
    { "prs"         , ko_no_argument       , K_OPTION_PRS         },
    { "ast"         , ko_no_argument       , K_OPTION_AST         },
    { nullptr       , ko_no_argument       , K_OPTION_INVALID     } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hvsu:f:";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "dmit_test\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h, --help                  Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v, --version               Display version\n";
    DMIT_COM_LOG_OUT << "    " << "    --lex                   Lex\n";
    DMIT_COM_LOG_OUT << "    " << "    --prs                   Parse\n";
    DMIT_COM_LOG_OUT << "    " << "    --ast                   Build the full AST\n";
    DMIT_COM_LOG_OUT << "    " << "-f, --file-path  FILE_PATH  Path of file to process\n";
}

void displayVersion()
{
    DMIT_COM_LOG_OUT << "dmit_test, version 0.1\n";
}

void displayFileError(const dmit::src::file::Error& fileError, const char* fileName)
{
    if (fileError == dmit::src::file::Error::FILE_NOT_FOUND)
    {
        DMIT_COM_LOG_ERR << "error: cannot find file '" << fileName << "'\n";
    }

    if (fileError == dmit::src::file::Error::FILE_NOT_REGULAR)
    {
        DMIT_COM_LOG_ERR << "error: '" << fileName << "' is not a regular file\n";
    }

    if (fileError == dmit::src::file::Error::FILE_OPEN_FAIL)
    {
        DMIT_COM_LOG_ERR << "error: cannot open '" << fileName << "'\n";
    }

    if (fileError == dmit::src::file::Error::FILE_READ_FAIL)
    {
        DMIT_COM_LOG_ERR << "error: failed to read file '" << fileName << "'\n";
    }
}

int main(int argc, char** argv)
{
    // Decode the arguments

    bool hasHelp    = false;
    bool hasVersion = false;
    bool hasLex     = false;
    bool hasPrs     = false;
    bool hasAst     = false;

    const char* filePath = nullptr;

    ketopt_t ketoptStatus = KETOPT_INIT;
    int      ketoptOption;

    while ((ketoptOption = ketopt(&ketoptStatus,
                                  argc,
                                  argv,
                                  0,
                                  K_OPTIONS_SHORT,
                                  K_OPTIONS_LONG)) != -1)
    {
        hasHelp    |= (ketoptOption == K_OPTION_HELP    );
        hasVersion |= (ketoptOption == K_OPTION_VERSION );
        hasLex     |= (ketoptOption == K_OPTION_LEX     );
        hasPrs     |= (ketoptOption == K_OPTION_PRS     );
        hasAst     |= (ketoptOption == K_OPTION_AST     );

        if (ketoptOption == K_OPTION_FILE_PATH ) { filePath = ketoptStatus.arg; }
    }

    // Display help/version if needed

    if (hasHelp)
    {
        displayHelp();
        return EXIT_SUCCESS;
    }

    if (hasVersion)
    {
        displayVersion();
        return EXIT_SUCCESS;
    }

    if (!filePath)
    {
        DMIT_COM_LOG_ERR << "You need to provide a file.\n";
        displayHelp();
        return EXIT_FAILURE;
    }

    const auto& fileErrOpt = dmit::src::file::make(filePath);

    if (fileErrOpt.hasError())
    {
        displayFileError(fileErrOpt.error(), filePath);
        return EXIT_FAILURE;
    }

    const auto& file = fileErrOpt.value();

    if (hasLex)
    {
        dmit::lex::state::Builder lexer;

        const auto& lex = lexer(file.content().data(),
                                file.content().size());

        DMIT_COM_LOG_OUT << lex << '\n';
    }

    if (hasPrs)
    {
        dmit::lex::state::Builder lexer;
        dmit::prs::state::Builder parser;

        const auto& lex = lexer(file.content().data(),
                                file.content().size());

        const auto& prs = parser(lex._tokens);

        DMIT_COM_LOG_OUT << prs << '\n';
    }

    if (hasAst)
    {
        dmit::ast::FromPathAndSource astFromPathAndSource;

        const auto& toParse = file.content();

        std::vector<uint8_t> filePathAsVector{reinterpret_cast<const uint8_t*>(filePath),
                                              reinterpret_cast<const uint8_t*>(filePath) + sizeof(filePath)};

        DMIT_COM_LOG_OUT << astFromPathAndSource.make(filePathAsVector, toParse) << '\n';
    }

    return EXIT_SUCCESS;
}
