#pragma once

#include "dmit/fmt/sem/interface_map.hpp"
#include "dmit/fmt/ast/bundle.hpp"
#include "dmit/fmt/ast/state.hpp"
#include "dmit/fmt/lex/state.hpp"
#include "dmit/fmt/prs/state.hpp"
#include "dmit/fmt/src/file.hpp"

#include "dmit/com/logger.hpp"

namespace dmit::fmt
{

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(fmt::Formatable, Type)>
com::logger::Sub& operator<<(com::logger::Sub& logger, const Type& loggable)
{
    logger << ::dmit::fmt::asString(loggable);

    return logger;
}

} // namespace dmit::fmt
