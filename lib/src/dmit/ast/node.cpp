#include "dmit/ast/node.hpp"

namespace dmit::ast
{

node::VIndex TNode<node::Kind::LIT_INTEGER>::_asVIndex;
node::Status TNode<node::Kind::LIT_INTEGER>::_status = node::Status::ASTED;

} // namespace dmit::ast
