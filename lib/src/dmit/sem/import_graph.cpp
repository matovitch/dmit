#include "dmit/sem/import_graph.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"

namespace dmit::sem
{

namespace
{

namespace import_graph
{

struct RegisterAst : ast::TVisitor<RegisterAst>
{

    RegisterAst(ast::State::NodePool& astNodePool, ImportGraph& importGraph) :
        ast::TVisitor<RegisterAst>{astNodePool},
        _importGraph{importGraph}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = base().get(importIdx);

        auto& moduleParent = base().get(
            ast::node::as<ast::node::Kind::MODULE>(import._parent)
        );

        _importGraph.makeDependency(moduleParent._id, import._id);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = base().get(moduleIdx);

        _importGraph.getOrMakeModule(module._id);

        base()(module._imports);
        base()(module._modules);
    }

    ImportGraph& _importGraph;
};

} // namespace import_graph

} // namespace

ImportGraph::ImportGraph() :
    _topoGraph{_topoPoolSet}
{}

ImportGraph::Module ImportGraph::getOrMakeModule(const com::UniqueId& moduleId)
{
    auto fit = _moduleMap.find(moduleId);

    if (fit != _moduleMap.end())
    {
        return fit->second;
    }

    auto module = _topoGraph.makeNode(moduleId);

    _moduleMap.emplace(moduleId, module);

    return module;
}

void ImportGraph::makeDependency(const com::UniqueId& lhsModuleId,
                                 const com::UniqueId& rhsModuleId)
{
    auto lhsModule = getOrMakeModule(lhsModuleId);
    auto rhsModule = getOrMakeModule(rhsModuleId);

    _topoGraph.attach(lhsModule, rhsModule);
}

void ImportGraph::makeBundles(std::vector<com::UniqueId> & moduleOrder,
                              std::vector<uint32_t     > & moduleBundles)
{
    if (_moduleMap.empty())
    {
        moduleBundles.push_back(0);
        return;
    }

    moduleOrder   .reserve(_moduleMap.size());
    moduleBundles .reserve(_moduleMap.size() + 1);

    int hyperNodeStackSize = 0;

    do
    {
        if (hyperNodeStackSize == 0)
        {
            moduleBundles.push_back(moduleOrder.size());
        }

        while (_topoGraph.isCyclic())
        {
            _topoGraph.solveCycle();
        }

        const auto top = _topoGraph.top();
                         _topoGraph.pop(top);

        if (top->isHyperOpen())
        {
            hyperNodeStackSize++;
            continue;
        }

        if (top->isHyperClose())
        {
            hyperNodeStackSize--;
            continue;
        }

        moduleOrder.push_back(top->_value);
    }
    while (!_topoGraph.empty() ||
            _topoGraph.isCyclic());

    moduleBundles.push_back(moduleOrder.size());
}

void ImportGraph::registerAst(ast::State& ast)
{
    import_graph::RegisterAst visitor{ast._nodePool, *this};

    visitor.base()(ast._module);
}

void ImportGraph::registerAsts(std::vector<ast::State>& asts)
{
    for (auto& ast : asts)
    {
        registerAst(ast);
    }
}

} // namespace dmit::sem
