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
#include "dmit/com/blit.hpp"

#include <optional>
#include <cstdint>
#include <vector>

namespace dmit::sem
{

namespace
{

struct Stack
{
    com::UniqueId _prefix;
    bool _isDeclaring;
};

struct InterfaceMaker : ast::TVisitor<InterfaceMaker, Stack>
{
    InterfaceMaker(ast::State::NodePool& astNodePool,
                   Context& context) :
        TVisitor<InterfaceMaker, Stack>{astNodePool},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE> typeIdx)
    {
        auto&& slice = getSlice(get(typeIdx)._name);

        auto id = com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        _context.makeTaskMedium
        (
            [this, typeIdx](const ast::node::VIndex& vIndex)
            {
                get(typeIdx)._status = ast::node::Status::TYPE_BOUND;

                com::blit(vIndex, get(typeIdx)._asVIndex);
            },
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

        if (!_stackPtrIn->_isDeclaring)
        {
            return;
        }

        auto&& slice = getSlice(defClass._name);

        com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        com::blit(_stackPtrIn->_prefix, defClass._id);

        _context.notifyEvent(_stackPtrIn->_prefix, defClassIdx);

        defClass._status = ast::node::Status::IDENTIFIED;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        base()(function._arguments);
        base()(function._returnType);

        if (!_stackPtrIn->_isDeclaring)
        {
            return;
        }

        auto&& slice = getSlice(function._name);

        com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        com::blit(_stackPtrIn->_prefix, function._id);

        function._status = ast::node::Status::IDENTIFIED;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        if (definition._role == ast::DefinitionRole::EXPORTED)
        {
            base()(definition._value);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = get(moduleIdx);

        _stackPtrIn->_isDeclaring = true;

        base()(module._definitions);

        _stackPtrIn->_isDeclaring = false;

        for (uint32_t i = 0; i < module._imports._size; i++)
        {
            _stackPtrIn->_prefix = get(module._imports[i])._id;
            base()(module._definitions);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        auto& view = get(viewIdx);

        _stackPtrIn->_prefix = view._id;

        base()(get(viewIdx)._modules);
    }

    Context& _context;
};

} // namespace

InterfaceMap::InterfaceMap() : _astNodePoolRef{_astNodePool} {}

void InterfaceMap::registerBundle(Context& context, ast::Bundle &bundle)
{
    // 0. Skip empty bundle

    if (!bundle._views._size)
    {
        return;
    }

    // 1. Perform shallow copy and register in the interface map

    auto& views = _viewsPool.make();

    _astNodePool.make(views, bundle._views._size);

    for (uint32_t i = 0; i < bundle._views._size; i++)
    {
        ast::copyShallow(bundle._views[i],
                         bundle._nodePool,
                         views[i],
                         _astNodePool);

        _asSimpleMap.emplace(bundle._nodePool.get(bundle._views[i])._id, views[i]);
    }

    // 2. Sematical analysis of the bundle's interface

    InterfaceMaker interfaceMaker{_astNodePool, context};

    context.makeTaskFromWork
    (
        [&interfaceMaker, &views]
        {
            interfaceMaker.base()(views);
        },
        context._coroutinePoolLarge
    );

    context.run();

    // 3. Bind the interface AST node pool to the bundle

    bundle._interfacePoolOpt = _astNodePool;
}

ast::node::TIndex<ast::node::Kind::VIEW> InterfaceMap::getView(const com::UniqueId& id) const
{
    return _asSimpleMap.at(id);
}

} // namespace dmit::sem
