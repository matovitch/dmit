#include "dmit/sem/context.hpp"

#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"

namespace dmit::sem
{

Context::Context(ast::State::NodePool& astNodePool) :
    _astNodePool{astNodePool},
    _scheduler{_schedulerTaskGraphPoolSet}
{}

src::Slice Context::getSlice(const ast::node::TIndex<ast::node::Kind::LEXEME> lexeme)
{
    return ast::lexeme::getSlice(lexeme, _astNodePool);
}

} // namespace dmit::sem
