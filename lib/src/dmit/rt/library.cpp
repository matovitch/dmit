#include "dmit/rt/library.hpp"

#include "dmit/rt/function_register.hpp"

namespace dmit::rt
{

void Library::recordsIn(FunctionRegister& functionRegister) const
{
    for (const auto& function : _functions)
    {
        function_register::recorder::StructuredArg toRecord
        {
            function->id(),
            function->me()
        };

        functionRegister.call(function_register::Recorder::ID,
                              reinterpret_cast<uint8_t*>(&toRecord));
    }
}

namespace library
{

Function& Function::me()
{
    return *this;
}

} // namespace library
} // namespace dmit::rt
