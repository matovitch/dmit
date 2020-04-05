#pragma once

#include "dmit/com/singleton.hpp"
#include "dmit/com/type_flag.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/fmt/com/logger.hpp"
#include "dmit/fmt/src/file.hpp"

#include <sstream>
#include <string>

namespace dmit::com
{

namespace logger
{

class Sub : fmt::Formatable
{

public:

    Sub(std::ostream& os);

    template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(fmt::Formatable, Type)>
    std::ostream& operator<<(const Type& loggable)
    {
        return _oss << ::dmit::fmt::asString(loggable);
    }

    template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS_NOT(fmt::Formatable, Type)>
    std::ostream& operator<<(const Type& notLoggable)
    {
        return _oss << notLoggable;
    }

    const std::ostringstream& stream() const;

    void flush();

    void clear();

    ~Sub();

private:

    std::ostringstream _oss;
    std::ostream&      _os;
};

}

struct Logger : Singletonable
{
    Logger();

    logger::Sub _out;
    logger::Sub _err;
};

} // namespace dmit::com

#define DMIT_COM_LOG_OUT dmit::com::TSingleton<dmit::com::Logger>::instance()._out
#define DMIT_COM_LOG_ERR dmit::com::TSingleton<dmit::com::Logger>::instance()._err
