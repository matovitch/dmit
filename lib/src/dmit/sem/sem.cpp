#include "dmit/sem/sem.hpp"

#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

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
    PathId(ast::State::NodePool& astNodePool, com::UniqueId rootId) :
        ast::TVisitor<PathId, StackIn, StackOut>{astNodePool, rootId}
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
    DeclareModulesAndLocateImports(ast::State& ast, FactMap& factMap) :
        ast::TVisitor<DeclareModulesAndLocateImports, Stack>{ast._nodePool,
                                                             ast._module,
                                                             com::UniqueId{}},
        _factMap{factMap}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    com::UniqueId getModuleId(ast::TNode<ast::node::Kind::MODULE> module)
    {
        if (!module._path)
        {
            return com::UniqueId{"#root"};
        }

        PathId pathId{_nodePool, _stackPtrIn->_id};

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

        _factMap.emplace(module._id, _nodePool, moduleIdx);
    }

    FactMap& _factMap;
};

struct SolveImports : ast::TVisitor<SolveImports>
{
    SolveImports(ast::State& ast, FactMap& factMap) :
        ast::TVisitor<SolveImports>{ast._nodePool},
        _factMap{factMap}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    std::optional<com::UniqueId> getId(ast::Expression path,
                                       ast::node::TIndex<ast::node::Kind::MODULE> parent)
    {
        auto& module = base().get(parent);

        PathId pathId{_nodePool, module._id};

        pathId.base()(path);

        if (_factMap._asRobinMap.find(pathId.id()) !=
            _factMap._asRobinMap.end())
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

    FactMap& _factMap;
};

} // namespace

void declareModulesAndLocateImports(ast::State& ast, FactMap& factMap)
{
    DeclareModulesAndLocateImports visitor{ast, factMap};

    visitor.base()(ast._module);
}

void solveImports(ast::State& ast, FactMap& factMap)
{
    SolveImports visitor{ast, factMap};

    visitor.base()(ast._module);
}

} // namespace dmit::sem
