#include "dmit/ast/copy_shallow.hpp"

#include "dmit/ast/blitter.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

namespace dmit::ast
{

namespace
{

struct Stack
{
    node::Index _index;
};

struct ShallowCopier : TVisitor<ShallowCopier, Stack>
{
    ShallowCopier(State::NodePool             & srceNodePool,
               State::NodePool                & destNodePool,
               node::TIndex<node::Kind::VIEW> & destView) :
        TVisitor<ShallowCopier, Stack>{srceNodePool, destView},
        _destNodePool{destNodePool}
    {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void copyRange(node::TRange<KIND>& srceRange,
                   node::TRange<KIND>& destRange)
    {
        _destNodePool.make(destRange, srceRange._size);
        _stackPtrIn->_index = destRange[0];
        base()(srceRange);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationConclusion(node::TIndex<KIND>)
    {
        _stackPtrIn->_index._value++;
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopPreamble(node::TRange<KIND>&){}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopConclusion(node::TRange<KIND>&){}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationPreamble(node::TIndex<KIND>) {}

    template <class Type>
    void emptyOption() {}

    template <com::TEnumIntegerType<node::Kind> NODE_KIND>
    void operator()(node::TIndex<NODE_KIND>)
    {
        std::cout << (int)NODE_KIND << '\n';
        DMIT_COM_ASSERT(!"Not implemented");
    }

    void operator()(node::TIndex<node::Kind::SOURCE> srceSourceIdx)
    {
        auto& srceSource = get(srceSourceIdx);
        auto& destSource = _destNodePool.get(
            as<node::Kind::SOURCE>(_stackPtrIn->_index)
        );

        std::memcpy(&destSource, &srceSource, sizeof(TNode<node::Kind::SOURCE>));
    }

    void operator()(node::TIndex<node::Kind::LEXEME> srceLexemeIdx)
    {
        auto& srceLexeme = get(srceLexemeIdx);
        auto& destLexeme = _destNodePool.get(
            as<node::Kind::LEXEME>(_stackPtrIn->_index)
        );

        _destNodePool.make(destLexeme._source);
        _stackPtrIn->_index = destLexeme._source;
        base()(srceLexeme._source);

        destLexeme._index = srceLexeme._index;
    }

    void operator()(node::TIndex<node::Kind::LIT_IDENTIFIER> srceIdentifierIdx)
    {
        auto& srceIdentifier = get(srceIdentifierIdx);
        auto& destIdentifier = _destNodePool.get(
            as<node::Kind::LIT_IDENTIFIER>(_stackPtrIn->_index)
        );

        _destNodePool.make(destIdentifier._lexeme);
        _stackPtrIn->_index = destIdentifier._lexeme;
        base()(srceIdentifier._lexeme);
    }

    void operator()(node::TIndex<node::Kind::EXP_BINOP> srceBinopIdx)
    {
        auto& srceBinop = get(srceBinopIdx);
        auto& destBinop = _destNodePool.get(
            as<node::Kind::EXP_BINOP>(_stackPtrIn->_index)
        );

        _destNodePool.make(destBinop._operator);
        _stackPtrIn->_index = destBinop._operator;
        base()(srceBinop._operator);

        auto blitterLhs = blitter::make(_destNodePool, destBinop._lhs);
        _stackPtrIn->_index = blitterLhs(srceBinop._lhs);
        base()(srceBinop._lhs);

        auto blitterRhs = blitter::make(_destNodePool, destBinop._rhs);
        _stackPtrIn->_index = blitterRhs(srceBinop._rhs);
        base()(srceBinop._rhs);
    }

    void operator()(node::TIndex<node::Kind::TYPE_CLAIM> srceTypeClaimIdx)
    {
        auto& srceTypeClaim = get(srceTypeClaimIdx);
        auto& destTypeClaim = _destNodePool.get(
            as<node::Kind::TYPE_CLAIM>(_stackPtrIn->_index)
        );

        _destNodePool.make(destTypeClaim._variable);
        _stackPtrIn->_index = destTypeClaim._variable;
        base()(srceTypeClaim._variable);

        _destNodePool.make(destTypeClaim._type);
        _stackPtrIn->_index = destTypeClaim._type;
        base()(srceTypeClaim._type);
    }

    void operator()(node::TIndex<node::Kind::DCL_IMPORT> srceImportIdx)
    {
        auto& srceImport = get(srceImportIdx);
        auto& destImport = _destNodePool.get(
            as<node::Kind::DCL_IMPORT>(_stackPtrIn->_index)
        );

        auto blitter = blitter::make(_destNodePool, destImport._path);
        _stackPtrIn->_index = blitter(srceImport._path);
        base()(srceImport._path);
    }

    void operator()(node::TIndex<node::Kind::FUN_DEFINITION> srceFunctionIdx)
    {
        auto& srceFunction = get(srceFunctionIdx);
        auto& destFunction = _destNodePool.get(
            as<node::Kind::FUN_DEFINITION>(_stackPtrIn->_index)
        );

        if (srceFunction._status != FunctionStatus::EXPORTED)
        {
            return;
        }

        _destNodePool.make(destFunction._name);
        _stackPtrIn->_index = destFunction._name;
        base()(srceFunction._name);

        copyRange(srceFunction._arguments,
                  destFunction._arguments);

        if (srceFunction._returnType)
        {
            auto blitter = blitter::make(_destNodePool, destFunction._returnType);
            _stackPtrIn->_index = blitter(srceFunction._returnType.value());
            base()(srceFunction._returnType);
        }
        else
        {
            com::blitDefault(destFunction._returnType);
        }

        com::blit(srceFunction._status,
                  destFunction._status);
    }

    void operator()(node::TIndex<node::Kind::TYP_DEFINITION> srceTypeIdx)
    {
        auto& srceType = get(srceTypeIdx);
        auto& destType = _destNodePool.get(
            as<node::Kind::TYP_DEFINITION>(_stackPtrIn->_index)
        );

        _destNodePool.make(destType._name);
        _stackPtrIn->_index = destType._name;
        base()(srceType._name);

        copyRange(srceType._members,
                  destType._members);
    }

    void operator()(node::TIndex<node::Kind::PARENT_PATH> srceParentPathIdx)
    {
        auto& srceParentPath = get(srceParentPathIdx)   ;
        auto& destParentPath = _destNodePool.get(
            as<node::Kind::PARENT_PATH>(_stackPtrIn->_index)
        );

        auto blitter = blitter::make(_destNodePool, destParentPath._expression);
        _stackPtrIn->_index = blitter(srceParentPath._expression);
        base()(srceParentPath._expression);

        if (srceParentPath._next)
        {
            auto blitter = blitter::make(_destNodePool, destParentPath._next);
            _stackPtrIn->_index = blitter(srceParentPath._next.value());
            base()(srceParentPath._next);
        }
        else
        {
            com::blitDefault(destParentPath._next);
        }
    }

    void operator()(node::TIndex<node::Kind::MODULE> srceModuleIdx)
    {
        auto& srceModule = get(srceModuleIdx)   ;
        auto& destModule = _destNodePool.get(
            as<node::Kind::MODULE>(_stackPtrIn->_index)
        );

        if (srceModule._path)
        {
            auto blitter = blitter::make(_destNodePool, destModule._path);
            _stackPtrIn->_index = blitter(srceModule._path.value());
            base()(srceModule._path);
        }
        else
        {
            com::blitDefault(destModule._path);
        }

        copyRange(srceModule._types,
                  destModule._types);

        copyRange(srceModule._functions,
                  destModule._functions);

        if (srceModule._parentPath)
        {
            auto blitter = blitter::make(_destNodePool, destModule._parentPath);
            _stackPtrIn->_index = blitter(srceModule._parentPath.value());
            base()(srceModule._parentPath);
        }
        else
        {
            com::blitDefault(destModule._parentPath);
        }

        _destNodePool.make(destModule._modules, 0);
    }

    void operator()(node::TIndex<node::Kind::VIEW> srceViewIdx)
    {
        auto& srceView = get(srceViewIdx);
        auto& destView = _destNodePool.get(
            as<node::Kind::VIEW>(_stackPtrIn->_index)
        );

        copyRange(srceView._modules,
                  destView._modules);
    }

    State::NodePool& _destNodePool;
};

} // namespace

void copyShallow(node::TIndex<node::Kind::VIEW> srceView,
                 State::NodePool                & srceNodePool,
                 node::TIndex<node::Kind::VIEW> destView,
                 State::NodePool                & destNodePool)
{
    ShallowCopier shallowCopier{srceNodePool,
                                destNodePool,
                                destView};

    shallowCopier.base()(srceView);
}

} // namespace dmit::ast
