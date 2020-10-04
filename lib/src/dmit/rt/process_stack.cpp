#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/process.hpp"
#include "dmit/vm/program.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/sha256.hpp"

namespace dmit::rt
{

ProcessStack::ProcessStack(const uint16_t callStackSize, const com::sha256::Seed& seed) :
    _callStackPool{callStackSize},
    _uniqueIdSequence{seed}
{}

void ProcessStack::push(const vm::Program&     program,
                        const vm::program::Counter programCounter)
{
    _processes.emplace(_callStackPool.make(), program, programCounter);
    _uniqueIds.emplace(_uniqueIdSequence.nextId());
}

vm::Process& ProcessStack::top()
{
    DMIT_COM_ASSERT(!_processes.empty());

    return _processes.top();
}

void ProcessStack::pop()
{
    _processes.pop();
    _uniqueIds.pop();
}

const com::UniqueId& ProcessStack::topId() const
{
    DMIT_COM_ASSERT(!_uniqueIds.empty());

    return _uniqueIds.top();
}

bool ProcessStack::isEmpty() const
{
    return _processes.empty();
}

} // namespace dmit::rt
