#include "dmit/sem/analyze.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/assert.hpp"
#include "dmit/com/murmur.hpp"

#include <iostream> // TODO remove

namespace dmit::sem
{

namespace
{

struct VisitorScopeId
{
    VisitorScopeId(Context& context) : _context{context} {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    com::UniqueId operator()(ast::node::TIndex<KIND> index)
    {
        DMIT_COM_ASSERT(!"[SEM] VisitorScopeId is not applicable for this node");
        return com::UniqueId{};
    }

    template <>
    com::UniqueId operator()<ast::node::Kind::PROGRAM>(ast::node::TIndex<ast::node::Kind::PROGRAM> program)
    {
        return com::UniqueId{"#Rootscope"};
    }

    template <>
    com::UniqueId operator()<ast::node::Kind::FUN_DEFINITION>(ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> functionIdx)
    {
        const auto& function = _context.get(functionIdx);

        // 1. Compute the local function scope id
        const auto& lexeme         = _context.get(function._name)._lexeme;
        const auto& slice          = _context.getSlice(lexeme);
        const auto functionScopeId = com::UniqueId{slice._head, slice.size()};

        // 2. Get the parent scope id
        const auto parentScope = _context.get(function._name)._parentScope;
        DMIT_COM_ASSERT(parentScope);

        auto parentScopeId = (*this)(parentScope.value());

        // 3. Combine the two to compute the global function scope id
        com::murmur::combine(functionScopeId, parentScopeId);
        return parentScopeId;
    }

    template <>
    com::UniqueId operator()<ast::node::Kind::TYPE_CLAIM>(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        const auto& typeClaim = _context.get(typeClaimIdx);

        // 1. Compute the local function scope id
        const auto& lexeme          = _context.get(typeClaim._variable)._lexeme;
        const auto& slice           = _context.getSlice(lexeme);
        const auto typeClaimScopeId = com::UniqueId{slice._head, slice.size()};

        // 2. Get the parent scope id
        const auto parentScope = _context.get(typeClaim._variable)._parentScope;
        DMIT_COM_ASSERT(parentScope);

        auto parentScopeId = (*this)(parentScope.value());

        // 3. Combine the two to compute the global function scope id
        com::murmur::combine(typeClaimScopeId, parentScopeId);
        return parentScopeId;
    }

    template <class... Types>
    com::UniqueId operator()(const std::variant<Types...>& variant)
    {
        return std::visit(*this, variant);
    }

    Context& _context;
};

struct Visitor
{
    Visitor(Context& context) : _context{context}, _visitorScopeId{context} {}

    void operator()(const ast::node::Location)
    {
        std::cout << "Not implemented!\n";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        std::cout << "Here's a type claim!\n";

        auto& typeClaim = _context.get(typeClaimIdx);

        _context.get(typeClaim._variable)._parentScope = _context._astParentScope;

        com::UniqueId define{"#Define"};
        com::murmur::combine(_visitorScopeId(typeClaimIdx), define);
        _context._mapFact.emplace(define, typeClaimIdx);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        auto& scope = _context.get(scopeIdx);

        scope._parentScope = _context._astParentScope;
        _context._astParentScope = scopeIdx;

        (*this)(scope._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> functionIdx)
    {
        std::cout << "Here's a function!\n";

        auto& function = _context.get(functionIdx);

        _context.get(function._name)._parentScope = _context._astParentScope;
        _context._astParentScope = functionIdx;

        com::UniqueId define{"#Define"};
        com::murmur::combine(_visitorScopeId(functionIdx), define);
        _context._mapFact.emplace(define, functionIdx);

        (*this)(function._arguments);
        (*this)(function._body);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PROGRAM> program)
    {
        _context._astParentScope = program;

        (*this)(_context.get(program)._functions);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(ast::node::TRange<KIND>& range)
    {
        for (uint32_t i = 0; i < range._size; i++)
        {
            (*this)(range[i]);
        }
    }

    template <class... Types>
    void operator()(std::variant<Types...>& variant)
    {
        std::visit(*this, variant);
    }

    Context& _context;

    VisitorScopeId _visitorScopeId;
};

} // namespace

void analyze(ast::State& ast)
{
    Context context{ast._nodePool};

    Visitor visitor{context};

    visitor(ast._program);
}

} // namespace dmit::sem
