#include "dmit/com/base64.hpp"

#include "dmit/com/logger.hpp"

extern "C"
{
    #include "ketopt/ketopt.h"
}

#include <cstdlib>
#include <cstdint>
#include <vector>

enum : char
{
    K_OPTION_INVALID = ':',
    K_OPTION_HELP    = 'h',
    K_OPTION_VERSION = 'v',
    K_OPTION_ENCODE  = 'e',
    K_OPTION_DECODE  = 'd'
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"    , ko_no_argument, K_OPTION_HELP    },
    { "version" , ko_no_argument, K_OPTION_VERSION },
    { "encode"  , ko_no_argument, K_OPTION_ENCODE  },
    { "decode"  , ko_no_argument, K_OPTION_DECODE  },
    { nullptr   , ko_no_argument, K_OPTION_INVALID } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hved";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "base64\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h --help       Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v --version    Display version\n";
    DMIT_COM_LOG_OUT << "    " << "-e --encode     Encode STDIN in base64\n";
    DMIT_COM_LOG_OUT << "    " << "-d --decode     Decode STDIN in base64\n";
}

void displayVersion()
{
    DMIT_COM_LOG_OUT << "base64, version 0.1\n";
}

static const std::size_t K_READ_BATCH_SIZE = 0x1000;

auto makeBytes(std::istream& is)
{
    std::vector<uint8_t> bytes;

    while (!is.eof())
    {
        bytes.resize(bytes.size() + K_READ_BATCH_SIZE);

        is.read(reinterpret_cast<char*>(bytes.data() + bytes.size() - K_READ_BATCH_SIZE), K_READ_BATCH_SIZE);
    }

    bytes.resize(bytes.size() - K_READ_BATCH_SIZE + is.gcount());

    return bytes;
}

void display(const std::vector<uint8_t>& bytes)
{
    for (const auto byte : bytes)
    {
        DMIT_COM_LOG_OUT << byte;
    }
}

void displayEncode(const std::vector<uint8_t>& srce)
{
    std::vector<uint8_t> dest;

    dest.resize(dmit::com::base64::encodeBufferSize(srce.size()));

    dmit::com::base64::encode(srce.data(),
                              srce.size(),
                              dest.data());
    display(dest);

    DMIT_COM_LOG_OUT << '\n';
}

void displayDecode(const std::vector<uint8_t>& srce)
{
    std::vector<uint8_t> dest;

    dest.resize(dmit::com::base64::decodeBufferSize(srce.data(),
                                         srce.size()));
    dmit::com::base64::decode(srce.data(),
                              srce.size(),
                              dest.data());
    display(dest);
}

int main(int argc, char** argv)
{
    bool hasHelp    = false;
    bool hasVersion = false;
    bool hasEncode  = false;
    bool hasDecode  = false;

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
        hasEncode  |= (ketoptOption == K_OPTION_ENCODE  );
        hasDecode  |= (ketoptOption == K_OPTION_DECODE  );
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

    if (hasEncode && hasDecode)
    {
        DMIT_COM_LOG_ERR << "error: encode and decode options are mutually exclusives\n";
        displayHelp();
        return EXIT_FAILURE;
    }

    if (!hasEncode && !hasDecode)
    {
        DMIT_COM_LOG_ERR << "error: invalid command line, no encode or decode flag set\n";
        displayHelp();
        return EXIT_FAILURE;
    }

    const auto& bytes = makeBytes(std::cin);

    if (hasEncode) { displayEncode(bytes); }
    if (hasDecode) { displayDecode(bytes); }

    return EXIT_SUCCESS;
}
