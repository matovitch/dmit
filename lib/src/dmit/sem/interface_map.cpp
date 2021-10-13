#include "dmit/sem/interface_map.hpp"

#include "dmit/sem/context.hpp"

#include "dmit/ast/copy_shallow.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"

#include <optional>
#include <vector>

namespace dmit::sem
{

namespace
{

struct Stack
{
    com::UniqueId _prefix;
};

struct InterfaceMaker : ast::TVisitor<InterfaceMaker, Stack>
{
    InterfaceMaker(ast::State::NodePool      & astNodePool,
                   InterfaceMap::SymbolTable & symbolTable,
                   Context& context) :
        TVisitor<InterfaceMaker, Stack>{astNodePool},
        _symbolTable{symbolTable},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE> typeIdx)
    {
        auto&& slice = ast::lexeme::getSlice(get(get(typeIdx)._name)._lexeme, _nodePool);

        const com::UniqueId sliceId{slice._head, slice.size()};

        auto id = com::murmur::combine(sliceId,_stackPtrIn->_prefix);

        _context.makeTask
        (
            [this, typeIdx, id]()
            {
                auto status = (_symbolTable.find(id) != _symbolTable.end());

                if (status)
                {
                    com::blit(id, get(typeIdx)._id);
                }

                return status;
            },
            _context._coroutinePoolMedium,
            typeIdx,
            id
        );
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        base()(get(typeClaimIdx)._type);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        auto& defClass = get(defClassIdx);

        base()(defClass._members);

        auto&& slice = ast::lexeme::getSlice(get(defClass._name)._lexeme, _nodePool);

        com::murmur::combine(
            com::UniqueId{slice._head, slice.size()},
            _stackPtrIn->_prefix
        );

        _symbolTable.emplace(_stackPtrIn->_prefix, defClassIdx);

        _context.notifyEvent(_stackPtrIn->_prefix);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        base()(function._arguments);

        if (function._returnType)
        {
            base()(function._returnType.value());
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        if (definition._status == ast::DefinitionStatus::EXPORTED)
        {
            base()(definition._value);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = get(moduleIdx);

        _stackPtrIn->_prefix = module._id;

        base()(module._definitions);

        for (uint32_t i = 0; i < module._imports._size; i++)
        {
            _stackPtrIn->_prefix = get(module._imports[i])._id;
            base()(module._definitions);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    InterfaceMap::SymbolTable& _symbolTable;

    Context& _context;
};

} // namespace

InterfaceMap::InterfaceMap(const std::vector<ast::Bundle>& bundles, ast::State::NodePool& astNodePool) :
    _astNodePool{astNodePool}
{
    for (const auto& bundle : bundles)
    {
        if (!bundle._views._size)
        {
            continue;
        }

        auto& views = _viewsPool.make();

        _astNodePool.make(views, bundle._views._size);

        for (uint32_t i = 0; i < bundle._views._size; i++)
        {
            _asSimpleMap.emplace(bundle._nodePool.get(bundle._views[i])._id, views[i]);
        }
    }
}

void InterfaceMap::registerBundle(ast::Bundle& bundle)
{
    // Create and assign the work
    InterfaceMaker interfaceMaker{bundle._nodePool, _symbolTable, _context};

    _context.makeTaskFromWork
    (
        [&interfaceMaker, &bundle]()
        {
            interfaceMaker.base()(bundle._views);
        },
        _context._coroutinePoolLarge
    );

    _context.run();

    // Copy the views
    for (uint32_t i = 0; i < bundle._views._size; i++)
    {
        const auto viewId = bundle._nodePool.get(bundle._views[i])._id;

        ast::copyShallow(bundle._views[i],
                         bundle._nodePool,
                         _asSimpleMap.at(viewId),
                         _astNodePool);
    }
}

} // namespace dmit::sem
