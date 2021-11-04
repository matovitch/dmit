#include "dmit/sem/analyze.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

#include <cstdint>

namespace dmit::sem
{

struct ExportLister : ast::TVisitor<ExportLister>
{
    ExportLister(ast::State::NodePool& astNodePool,
                 Context& context) :
        TVisitor<ExportLister>{astNodePool},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        _context.notifyEvent(get(defClassIdx)._id, defClassIdx);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        _context.notifyEvent(get(functionIdx)._id, functionIdx);
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
        base()(get(moduleIdx)._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    Context& _context;
};

struct Analyzer : ast::TVisitor<Analyzer>
{
    Analyzer(ast::State::NodePool& astNodePool,
             InterfaceMap& interfaceMap) :
        TVisitor<Analyzer>{astNodePool},
        _interfaceMap{interfaceMap}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = get(moduleIdx);

        ExportLister exportLister{_interfaceMap._astNodePool,
                                  _context};

        for (uint32_t i = 0; i < module._imports._size; i++)
        {
            auto viewIdx = _interfaceMap.getView(get(module._imports[i])._id);

            exportLister.base()(viewIdx);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    InterfaceMap& _interfaceMap;
    Context       _context;
};

int8_t analyze(InterfaceMap& interfaceMap, ast::Bundle& bundle)
{
    Analyzer analyzer{bundle._nodePool, interfaceMap};

    analyzer.base()(bundle._views);

    return 0;
}

} // namespace dmit::sem
