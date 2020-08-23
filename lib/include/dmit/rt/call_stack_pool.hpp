#pragma once

#include "dmit/vm/stack_call.hpp"
#include "dmit/vm/program.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dmit::rt
{

class CallStackPool
{

public:

    CallStackPool(const uint16_t stackSize);

    vm::StackCall::Storage& make();

private:

    const uint16_t _stackSize;

    // TODO: This pool design is dumbissim, please improve
    std::vector<std::unique_ptr<vm::StackCall::Storage>> _stackStorages;
};

} // namespace dmit::rt
