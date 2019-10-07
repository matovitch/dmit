#include "dmit/src/file.hpp"

#include "dmit/com/error_option.hpp"
#include "dmit/com/logger.hpp"

extern "C"
{
    #include "ketopt/ketopt.h"
}

#include <iostream>
#include <cstdlib>
#include <cstdint>

enum : char
{
    K_OPTION_INVALID = ':',
    K_OPTION_HELP    = 'h',
    K_OPTION_VERSION = 'v',
    K_OPTION_FILE    = 'f',
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"    , ko_no_argument       , K_OPTION_HELP    },
    { "version" , ko_no_argument       , K_OPTION_VERSION },
    { "file"    , ko_required_argument , K_OPTION_FILE    },
    { nullptr   , ko_no_argument       , K_OPTION_INVALID } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hvf:";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "file_loader\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h, --help         Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v, --version      Display version\n";
    DMIT_COM_LOG_OUT << "    " << "-f, --file=FILE    Attempt to load file at FILE location\n";
}

void displayVersion()
{
    DMIT_COM_LOG_OUT << "file_loader, version 0.1\n";
}

void displayError(const dmit::src::file::Error& fileError, const char* fileName)
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
    bool        hasHelp    = false;
    bool        hasVersion = false;
    const char* fileName   = nullptr;

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

        if (ketoptOption == K_OPTION_FILE)
        {
            fileName = ketoptStatus.arg;
        }
    }

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

    if (!fileName)
    {
        DMIT_COM_LOG_ERR << "error: unrecognized command line\n";
        displayHelp();
        return EXIT_FAILURE;
    }

    const auto& fileErrOpt = dmit::src::file::make(fileName);

    if (fileErrOpt.hasError())
    {
        displayError(fileErrOpt.error(), fileName);
        return EXIT_FAILURE;
    }

    DMIT_COM_LOG_OUT << fileErrOpt.value() << '\n';

    return EXIT_SUCCESS;
}
