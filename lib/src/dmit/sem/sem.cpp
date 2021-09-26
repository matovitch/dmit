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

    ast::TVisitor<DeclareModulesAndLocateImports, Stack>& base()
    {
        return static_cast<ast::TVisitor<DeclareModulesAndLocateImports, Stack>&>(*this);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopConclusion(ast::node::TRange<KIND>& range) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopPreamble(ast::node::TRange<KIND>&) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationConclusion(ast::node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationPreamble(ast::node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void emptyOption() {}

    com::UniqueId getModuleId(ast::TNode<ast::node::Kind::MODULE> module)
    {
        if (!module._name)
        {
            return com::UniqueId{"#root"};
        }

        auto& name = base().get(module._name.value());

        auto&& slice = ast::lexeme::getSlice(name._lexeme, _nodePool);

        return com::UniqueId{slice._head, slice.size()};
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = base().get(importIdx);

        com::blit(_stackPtr->_parent , import._parent );
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = base().get(moduleIdx);

        com::murmur::combine(getModuleId(module), _stackPtr->_id);

        com::blit(_stackPtr->_id     , module._id     );
        com::blit(_stackPtr->_parent , module._parent );

        _stackPtr->_parent = moduleIdx;

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

    ast::TVisitor<SolveImports>& base()
    {
        return static_cast<ast::TVisitor<SolveImports>&>(*this);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopConclusion(ast::node::TRange<KIND>& range) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopPreamble(ast::node::TRange<KIND>&) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationConclusion(ast::node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationPreamble(ast::node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void emptyOption() {}

    std::optional<com::UniqueId> getId(com::UniqueId name,
                                       ast::node::TIndex<ast::node::Kind::MODULE> parent)
    {
        auto& module = base().get(parent);

        com::UniqueId moduleIdCopy = module._id;

        com::murmur::combine(name, moduleIdCopy);

        if (_context._factMap.find(moduleIdCopy) != _context._factMap.end())
        {
            return moduleIdCopy;
        }

        auto& moduleParent = std::get<decltype(parent)>(module._parent);

        if (parent == moduleParent)
        {
            return std::nullopt;
        }

        return getId(name, moduleParent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = base().get(importIdx);

        auto& moduleParent = std::get<ast::node::TIndex<ast::node::Kind::MODULE>>(import._parent);

        auto& moduleName = base().get(import._moduleName);

        auto&& slice = ast::lexeme::getSlice(moduleName._lexeme, _nodePool);

        auto&& idOpt = getId(com::UniqueId{slice._head, slice.size()}, moduleParent);

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
