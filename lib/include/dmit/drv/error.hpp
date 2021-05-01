#pragma once

#include "dmit/com/logger.hpp"

#include "sqlite3/sqlite3.h"
#include "nng/nng.h"

namespace dmit::drv
{

void displaySqlite3Error(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << "error: " << functionName << " returned '" << sqlite3_errstr(errorCode) << "'\n";
}

void displayNngError(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << "error: " << functionName << " returned '" << nng_strerror(errorCode) << "'\n";
}

} // namespace dmit::drv
