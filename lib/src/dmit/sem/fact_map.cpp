#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/murmur.hpp"

#include <cstdint>

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

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        auto&& slice = getSlice(identifierIdx);

        _stackPtrOut->_id = _stackPtrIn->_id;

        com::murmur::combine(slice.makeUniqueId(), _stackPtrOut->_id);
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
    ast::node::Index _parent;
    com::UniqueId    _id;
};

struct FindModulesAndBindImports : ast::TVisitor<FindModulesAndBindImports, Stack>
{
    FindModulesAndBindImports(ast::State& ast, FactMap& factMap) :
        ast::TVisitor<FindModulesAndBindImports, Stack>{ast._nodePool,
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

        module._status = ast::node::Status::IDENTIFIED;
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

    std::optional<com::UniqueId> getId(ast::node::VIndex path,
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

        auto moduleParent = ast::node::as<ast::node::Kind::MODULE>(module._parent);

        if (parent == moduleParent)
        {
            return std::nullopt;
        }

        return getId(path, moduleParent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = base().get(importIdx);

        auto moduleParent = ast::node::as<ast::node::Kind::MODULE>(import._parent);

        auto&& idOpt = getId(import._path, moduleParent);

        DMIT_COM_ASSERT(idOpt && "error: failed to find import!");

        import._id = idOpt.value();

        import._status = ast::node::Status::IDENTIFIED;
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

namespace fact_map
{

com::UniqueId next(com::UniqueId key)
{
    com::murmur::combine(key, key);

    return key;
}

} // namespace fact_map

void FactMap::emplace(com::UniqueId key,
                      ast::State::NodePool& astNodePool,
                      ast::node::Index index)
{
    auto fit = _asRobinMap.find(key);

    if (fit == _asRobinMap.end())
    {
        _asRobinMap.emplace(key, Fact{&astNodePool, index, 1});
        return;
    }

    fit->second._count++;

    emplace(fact_map::next(key), astNodePool, index);
}

void FactMap::findModulesAndBindImports(ast::State& ast)
{
    FindModulesAndBindImports visitor{ast, *this};

    visitor.base()(ast._module);
}

void FactMap::solveImports(ast::State& ast)
{
    SolveImports visitor{ast, *this};

    visitor.base()(ast._module);
}

void FactMap::solveImports(com::TStorage<ast::State>& asts)
{
    for (auto& ast : asts)
    {
        findModulesAndBindImports(ast);
    }

    for (auto& ast : asts)
    {
        solveImports(ast);
    }
}

} // namespace dmit::sem
