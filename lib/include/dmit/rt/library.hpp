#pragma once

#include "dmit/rt/callable.hpp"

#include "dmit/com/unique_id.hpp"

#include <memory>
#include <vector>

namespace dmit::rt
{

namespace library
{

struct Function : Callable
{
    virtual const com::UniqueId& id() const = 0;

    Function& me();
};

namespace function
{

using Pool = std::vector<std::unique_ptr<Function>>;

} // namespace function
} // namespace library

class Library
{

public:

    const library::function::Pool& functions() const;

    template <class FunctionType, class... Args>
    void addFunction(Args&&... args)
    {
        _functions.emplace_back(std::make_unique<FunctionType>(args...));
    }

private:

    library::function::Pool _functions;
};

} // namespace dmit::rt
