#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/copy_shallow.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"

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

    std::optional<ast::node::TIndex<ast::node::Kind::PARENT_PATH>> _parentPath;
};

struct InterfaceMaker : ast::TVisitor<InterfaceMaker, Stack>
{
    InterfaceMaker(ast::State::NodePool      & astNodePool,
                   InterfaceMap::SymbolTable & symbolTable) :
        TVisitor<InterfaceMaker, Stack>{astNodePool},
        _symbolTable{symbolTable}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void awaitDefinition(const com::UniqueId sliceId,
                         const std::optional<ast::node::TIndex<ast::node::Kind::PARENT_PATH>>& parentPath,
                         com::UniqueId& typeId)
    {
        // TODO
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        auto& type = get(get(typeClaimIdx)._type);

        auto&& slice = ast::lexeme::getSlice(get(type._name)._lexeme, _nodePool);

        const com::UniqueId sliceId{slice._head, slice.size()};

        com::murmur::combine(
            sliceId,
            _stackPtrIn->_prefix
        );

        auto fit = _symbolTable.find(_stackPtrIn->_prefix);

        if (fit == _symbolTable.end())
        {
            awaitDefinition(sliceId,
                            _stackPtrIn->_parentPath,
                            _stackPtrIn->_prefix);
        }

        com::blit(_stackPtrIn->_prefix, type._id);

        DMIT_COM_ASSERT(!"[SEM] TODO");
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
    }


    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        base()(get(functionIdx)._arguments);
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

        _stackPtrIn->_prefix     = module._id;
        _stackPtrIn->_parentPath = module._parentPath;

        base()(module._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    InterfaceMap::SymbolTable& _symbolTable;
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
    InterfaceMaker interfaceMaker{bundle._nodePool, _symbolTable};

    interfaceMaker.base()(bundle._views);

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
