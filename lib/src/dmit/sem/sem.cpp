#include "dmit/sem/sem.hpp"

#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/node.hpp"
#include "dmit/ast/pool.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"

namespace dmit::sem
{

namespace
{

struct StackIn
{
    com::UniqueId _id;
};

struct StackOut
{
    com::UniqueId _id;
};

struct PathId : ast::TVisitor<PathId, StackIn, StackOut>
{
    PathId(Context& context, com::UniqueId rootId) :
        ast::TVisitor<PathId, StackIn, StackOut>{context._ast._nodePool, rootId}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(ast::node::TIndex<KIND> nodeIndex) {}

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER> idIdx)
    {
        auto&& slice = ast::lexeme::getSlice(get(idIdx)._lexeme, _nodePool);

        _stackPtrOut->_id = _stackPtrIn->_id;

        com::murmur::combine(
            com::UniqueId{slice._head, slice.size()},
            _stackPtrOut->_id
        );
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> binopIdx)
    {
        auto& binop = get(binopIdx);

        base()(binop._lhs);

        _stackPtrIn->_id = _stackPtrOut->_id;

        base()(binop._rhs);
    }

    com::UniqueId id()
    {
        return _stackPtrOut->_id;
    }
};

struct Stack
{
    ast::node::Location _parent;
    com::UniqueId       _id;
};

struct DeclareModulesAndLocateImports : ast::TVisitor<DeclareModulesAndLocateImports, Stack>
{
    DeclareModulesAndLocateImports(Context& context) :
        ast::TVisitor<DeclareModulesAndLocateImports, Stack>{context._ast._nodePool,
                                                             context._ast._module,
                                                             com::UniqueId{}},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    com::UniqueId getModuleId(ast::TNode<ast::node::Kind::MODULE> module)
    {
        if (!module._path)
        {
            return com::UniqueId{"#root"};
        }

        PathId pathId{_context, _stackPtrIn->_id};

        pathId.base()(module._path);

        return pathId.id();
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = base().get(importIdx);

        com::blit(_stackPtrIn->_parent , import._parent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = base().get(moduleIdx);

        _stackPtrIn->_id = getModuleId(module);

        com::blit(_stackPtrIn->_id     , module._id     );
        com::blit(_stackPtrIn->_parent , module._parent );

        _stackPtrIn->_parent = moduleIdx;

        base()(module._imports);
        base()(module._modules);

        _context._factMap.emplace(module._id, moduleIdx);
    }

    Context& _context;
};

struct SolveImports : ast::TVisitor<SolveImports>
{
    SolveImports(Context& context) :
        ast::TVisitor<SolveImports>{context._ast._nodePool},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    std::optional<com::UniqueId> getId(ast::Expression path,
                                       ast::node::TIndex<ast::node::Kind::MODULE> parent)
    {
        auto& module = base().get(parent);

        PathId pathId{_context, module._id};

        pathId.base()(path);

        if (_context._factMap._asRobinMap.find(pathId.id()) !=
            _context._factMap._asRobinMap.end())
        {
            return pathId.id();
        }

        auto& moduleParent = std::get<decltype(parent)>(module._parent);

        if (parent == moduleParent)
        {
            return std::nullopt;
        }

        return getId(path, moduleParent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = base().get(importIdx);

        auto& moduleParent = std::get<ast::node::TIndex<ast::node::Kind::MODULE>>(import._parent);

        auto&& idOpt = getId(import._path, moduleParent);

        DMIT_COM_ASSERT(idOpt && "error: failed to find import!");

        import._id = idOpt.value();
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = base().get(moduleIdx);

        base()(module._imports);
        base()(module._modules);
    }

    Context& _context;
};

} // namespace

void declareModulesAndLocateImports(Context& context)
{
    DeclareModulesAndLocateImports visitor{context};

    visitor.base()(context._ast._module);
}

void solveImports(Context& context)
{
    SolveImports visitor{context};

    visitor.base()(context._ast._module);
}

} // namespace dmit::sem
