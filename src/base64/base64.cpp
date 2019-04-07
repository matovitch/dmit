#include "base64/base64.hpp"

extern "C"
{
    #include "ketopt/ketopt.h"
}

#include <iostream>
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

void displayHelp(std::ostream& os)
{
    os << "base64\n\n";
    os << "Usage:\n";
    os << "    " << "-h --help       Show this screen\n";
    os << "    " << "-v --version    Display version\n";
    os << "    " << "-e --encode     Encode STDIN in base64\n";
    os << "    " << "-d --decode     Decode STDIN in base64\n";
}

void displayVersion(std::ostream& os)
{
    os << "base64, version 0.1\n";
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

void display(const std::vector<uint8_t>& bytes, std::ostream& os)
{
    for (const auto byte : bytes)
    {
        os << byte;
    }

    os << '\n';
}

void displayEncode(const std::vector<uint8_t>& srce, std::ostream& os)
{
    std::vector<uint8_t> dest;

    dest.resize(base64::encodeBufferSize(srce.size()));

    base64::encode(srce.data(),
                   srce.size(),
                   dest.data());

    display(dest, os);
}

void displayDecode(const std::vector<uint8_t>& srce, std::ostream& os)
{
    std::vector<uint8_t> dest;

    dest.resize(base64::decodeBufferSize(srce.data(),
                                         srce.size()));
    base64::decode(srce.data(),
                   srce.size(),
                   dest.data());

    display(dest, os);
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
        displayHelp(std::cout);
        return EXIT_SUCCESS;
    }

    if (hasVersion)
    {
        displayVersion(std::cout);
        return EXIT_SUCCESS;
    }

    if (hasEncode && hasDecode)
    {
        std::cerr << "error: encode and decode options are mutually exclusives.\n";
        displayHelp(std::cerr);
        return EXIT_FAILURE;
    }

    const auto& bytes = makeBytes(std::cin);

    if (hasEncode)
    {
        displayEncode(bytes, std::cout);
    }

    if (hasDecode)
    {
        displayDecode(bytes, std::cout);
    }

    return EXIT_SUCCESS;
}
