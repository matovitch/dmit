#pragma once

#include "dmit/com/singleton.hpp"
#include "dmit/com/type_flag.hpp"

#include <sstream>
#include <ostream>

namespace dmit
{

namespace fmt { struct Formatable; } // To avoid depending upon dmit::fmt

namespace com
{

namespace logger
{

class Sub
{

public:

    Sub(std::ostream& os);

    template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS_NOT(fmt::Formatable, Type)>
    Sub& operator<<(const Type& notLoggable)
    {
        _oss << notLoggable;

        return *this;
    }

    const std::ostringstream& stream() const;

    void flush();

    void clear();

    ~Sub();

private:

    std::ostringstream _oss;
    std::ostream&      _os;
};

} // namespace logger

struct Logger : Singletonable
{
    Logger();

    logger::Sub _out;
    logger::Sub _err;
};

} // namespace com
} // namespace dmit

#define DMIT_COM_LOG_OUT dmit::com::TSingleton<dmit::com::Logger>::instance()._out
#define DMIT_COM_LOG_ERR dmit::com::TSingleton<dmit::com::Logger>::instance()._err
