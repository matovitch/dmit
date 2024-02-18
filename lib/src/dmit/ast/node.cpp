#include "dmit/ast/node.hpp"

namespace dmit::ast
{

node::VIndexOrLock TNode<node::Kind::LIT_INTEGER>::_asVIndexOrLock;
node::Status TNode<node::Kind::LIT_INTEGER>::_status = node::Status::ASTED;

} // namespace dmit::ast
