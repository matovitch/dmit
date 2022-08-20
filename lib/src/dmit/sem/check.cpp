#include "dmit/sem/check.hpp"

#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

namespace dmit::sem
{

namespace
{

struct Checker : ast::TVisitor<Checker>
{

    Checker(ast::State::NodePool& astNodePool,
            Context& context) :
        TVisitor<Checker>{astNodePool},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        //TODO
    }

    Context& _context;
};

} // namespace

void check(ast::Bundle& bundle, Context& context)
{
    Checker checker{bundle._nodePool, context};
    checker.base()(bundle._views);
}

} // namespace dmit::sem
