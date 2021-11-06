#include "dmit/fmt/ast/definition_role.hpp"

#include "dmit/ast/definition_role.hpp"

static const char* K_DEFINITION_STATUS_AS_CSTR[] =
{
    "EXPORTED",
    "LOCAL"
};

namespace dmit::fmt
{

std::string asString(const ast::DefinitionRole definitionStatus)
{
    std::ostringstream oss;

    oss << "\"" << K_DEFINITION_STATUS_AS_CSTR[definitionStatus._asInt] << "\"";

    return oss.str();
}

} // dmit::fmt
