#include "dmit/sem/analyze.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/assert.hpp"
#include "dmit/com/murmur.hpp"

#include <iostream> // TODO remove
#include <cstring>

namespace dmit::sem
{

namespace
{

template <class Variant, class BlipType>
void blitVariant(const BlipType& toBlip, Variant& variant)
{
    const Variant toBlipAsVariant = toBlip;
    std::memcpy(&variant, &toBlipAsVariant, sizeof(Variant));
}

struct VisitorScopeId
{
    VisitorScopeId(Context& context) : _context{context} {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    com::UniqueId operator()(const ast::node::TIndex<KIND>)
    {
        std::cout << "[SEM] VisitorScopeId is not applicable for this node\n";
        return com::UniqueId{};
    }

    com::UniqueId operator()(ast::node::TIndex<ast::node::Kind::PROGRAM>)
    {
        std::cout << "[SEM] VisitorScopeId PROGRAM\n";

        return com::UniqueId{"#Rootscope"};
    }

    com::UniqueId operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        std::cout << "[SEM] VisitorScopeId SCOPE\n";

        com::UniqueId id{"#Scope"};

        const auto& parentScope = _context.get(scopeIdx)._parentScope;
        DMIT_COM_ASSERT(parentScope);
        com::murmur::combine((*this)(parentScope.value()), id);

        return id;
    }

    com::UniqueId operator()(ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> functionIdx)
    {
        std::cout << "[SEM] VisitorScopeId FUN_DEFINITION\n";

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

    com::UniqueId operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        std::cout << "[SEM] VisitorScopeId TYPE_CLAIM\n";

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

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(const ast::node::TIndex<KIND>)
    {
        std::cout << "[SEM] Visitor is not applicable for this node\n";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        std::cout << "Here's a type claim!\n";

        auto& typeClaim = _context.get(typeClaimIdx);

        blitVariant(_context._astParentScope, _context.get(typeClaim._variable)._parentScope);

        com::UniqueId define{"#Define"};
        com::murmur::combine(_visitorScopeId(typeClaimIdx), define);
        _context._mapFact.emplace(define, typeClaimIdx);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE> dclVariableIdx)
    {
        std::cout << "Here's a declaration of variable!\n";

        (*this)(_context.get(dclVariableIdx)._typeClaim);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::STM_RETURN> stmReturnIdx)
    {
        std::cout << "Here's a return statement!\n";

        (*this)(_context.get(stmReturnIdx)._expression);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT> scopeVariantIdx)
    {
        std::cout << "Here's a scope variant!\n";

        (*this)(_context.get(scopeVariantIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        auto& scope = _context.get(scopeIdx);

        blitVariant(_context._astParentScope, scope._parentScope);
        _context._astParentScope = scopeIdx;

        (*this)(scope._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> functionIdx)
    {
        std::cout << "Here's a function!\n";

        auto& function = _context.get(functionIdx);

        blitVariant(_context._astParentScope, _context.get(function._name)._parentScope);
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
