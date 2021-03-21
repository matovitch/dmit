#include "dmit/fmt/ast/function_status.hpp"

#include "dmit/ast/function_status.hpp"

static const char* K_FUNCTION_STATUS_AS_CSTR[] =
{
    "EXPORTED",
    "LOCAL"
};

namespace dmit::fmt
{

std::string asString(const ast::FunctionStatus functionStatus)
{
    std::ostringstream oss;

    oss << "\"" << K_FUNCTION_STATUS_AS_CSTR[functionStatus._asInt] << "\"";

    return oss.str();
}

} // dmit::fmt
