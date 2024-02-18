#include "dmit/ast/node.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/blit.hpp"

#include <utility>

namespace dmit::sem
{

struct StackDummy{};

template <class Derived, class StackIn  = StackDummy,
                         class StackOut = StackDummy>
struct TVisitor : ast::TVisitor<Derived, StackIn, StackOut>
{
    template <class... Args>
    TVisitor(Context& context, ast::State::NodePool& astNodePool, Args&&... args) :
        ast::TVisitor<Derived, StackIn, StackOut>{astNodePool, std::forward<Args>(args)...},
        _context{context}
    {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    ast::node::VIndexOrLock& vIndexOrLock(ast::node::TIndex<KIND> astNodeIdx)
    {
        auto& astNode = ast::TVisitor<Derived, StackIn, StackOut>::get(astNodeIdx);

        if (astNode._status == ast::node::Status::ASTED)
        {
            com::blit(_context.makeLock(), astNode._asVIndexOrLock);
            astNode._status = ast::node::Status::LOCKED;
        }

        return astNode._asVIndexOrLock;
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    ast::node::VIndex vIndex(ast::node::TIndex<KIND> astNodeIdx)
    {
        return _context.vIndex(vIndexOrLock(astNodeIdx));
    }

    void notifyEvent(const com::UniqueId& id, const ast::node::VIndex& vIndex)
    {
        return _context.notifyEvent(id, vIndex);
    }

    Context& _context;
};


} // namespace dmit::sem
