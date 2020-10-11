#pragma once

#include "dmit/vm/process.hpp"
#include "dmit/vm/program.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/sha256.hpp"

#include "pool/pool.hpp"

#include <stack>

namespace dmit::rt
{

class ProcessStack
{

public:

    ProcessStack(const uint16_t callStackSize, const com::sha256::Seed& seed);

    void push(const vm::Program&, const vm::program::Counter);

    vm::Process& top();

    void pop();

    const com::UniqueId& topId() const;

    bool isEmpty() const;

private:

    const uint16_t _callStackSize;

    pool::TMake<vm::StackCall::Storage, 1> _callStackPool;

    std::stack<vm::Process   > _processes;
    std::stack<com::UniqueId > _uniqueIds;

    com::sha256::UniqueIdSequence _uniqueIdSequence;
};

} // namespace dmit::rt
