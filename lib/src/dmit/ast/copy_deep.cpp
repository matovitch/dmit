#include "dmit/ast/copy_deep.hpp"

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

struct DeepCopier : TVisitor<DeepCopier, Stack>
{
    DeepCopier(State::NodePool                  & srceNodePool,
               State::NodePool                  & destNodePool,
               node::TIndex<node::Kind::MODULE> & destModule) :
        TVisitor<DeepCopier, Stack>{srceNodePool, destModule},
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
        DMIT_COM_ASSERT(!"[AST] Cannot deep copy node");
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

    void operator()(node::TIndex<node::Kind::LIT_INTEGER> srceIntegerIdx)
    {
        auto& srceInteger = get(srceIntegerIdx);
        auto& destInteger = _destNodePool.get(
            as<node::Kind::LIT_INTEGER>(_stackPtrIn->_index)
        );

        _destNodePool.make(destInteger._lexeme);
        _stackPtrIn->_index = destInteger._lexeme;
        base()(srceInteger._lexeme);
    }

    void operator()(node::TIndex<node::Kind::LIT_DECIMAL> srceDecimalIdx)
    {
        auto& srceDecimal = get(srceDecimalIdx);
        auto& destDecimal = _destNodePool.get(
            as<node::Kind::LIT_DECIMAL>(_stackPtrIn->_index)
        );

        _destNodePool.make(destDecimal._lexeme);
        _stackPtrIn->_index = destDecimal._lexeme;
        base()(srceDecimal._lexeme);
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

    void operator()(node::TIndex<node::Kind::EXP_MONOP> srceMonopIdx)
    {
        auto& srceMonop = get(srceMonopIdx);
        auto& destMonop = _destNodePool.get(
            as<node::Kind::EXP_MONOP>(_stackPtrIn->_index)
        );

        _destNodePool.make(destMonop._operator);
        _stackPtrIn->_index = destMonop._operator;
        base()(srceMonop._operator);

        auto blitterLhs = blitter::make(_destNodePool, destMonop._expression);
        _stackPtrIn->_index = blitterLhs(srceMonop._expression);
        base()(srceMonop._expression);
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

    void operator()(node::TIndex<node::Kind::EXPRESSION> srceExpressionIdx)
    {
        auto& srceExpression = get(srceExpressionIdx);
        auto& destExpression = _destNodePool.get(
            as<node::Kind::EXPRESSION>(_stackPtrIn->_index)
        );

        auto blitter = blitter::make(_destNodePool, destExpression._value);
        _stackPtrIn->_index = blitter(srceExpression._value);
        base()(srceExpression._value);
    }

    void operator()(node::TIndex<node::Kind::FUN_CALL> srceFunCallIdx)
    {
        auto& srceFunCall = get(srceFunCallIdx);
        auto& destFunCall = _destNodePool.get(
            as<node::Kind::FUN_CALL>(_stackPtrIn->_index)
        );

        _destNodePool.make(destFunCall._callee);
        _stackPtrIn->_index = destFunCall._callee;
        base()(srceFunCall._callee);

        copyRange(srceFunCall._arguments,
                  destFunCall._arguments);
    }

    void operator()(node::TIndex<node::Kind::DCL_VARIABLE> srceDclVariableIdx)
    {
        auto& srceDclVariable = get(srceDclVariableIdx);
        auto& destDclVariable = _destNodePool.get(
            as<node::Kind::DCL_VARIABLE>(_stackPtrIn->_index)
        );

        _destNodePool.make(destDclVariable._typeClaim);
        _stackPtrIn->_index = destDclVariable._typeClaim;
        base()(srceDclVariable._typeClaim);
    }

    void operator()(node::TIndex<node::Kind::STM_RETURN> srceStmReturnIdx)
    {
        auto& srceStmReturn = get(srceStmReturnIdx);
        auto& destStmReturn = _destNodePool.get(
            as<node::Kind::STM_RETURN>(_stackPtrIn->_index)
        );

        auto blitter = blitter::make(_destNodePool, destStmReturn._expression);
        _stackPtrIn->_index = blitter(srceStmReturn._expression);
        base()(srceStmReturn._expression);
    }

    void operator()(node::TIndex<node::Kind::SCOPE_VARIANT> srceScopeVariantIdx)
    {
        auto& srceScopeVariant = get(srceScopeVariantIdx);
        auto& destScopeVariant = _destNodePool.get(
            as<node::Kind::SCOPE_VARIANT>(_stackPtrIn->_index)
        );

        auto blitter = blitter::make(_destNodePool, destScopeVariant._value);
        _stackPtrIn->_index = blitter(srceScopeVariant._value);
        base()(srceScopeVariant._value);
    }

    void operator()(node::TIndex<node::Kind::SCOPE> srceScopeIdx)
    {
        auto& srceScope = get(srceScopeIdx);
        auto& destScope = _destNodePool.get(
            as<node::Kind::SCOPE>(_stackPtrIn->_index)
        );

        copyRange(srceScope._variants,
                  destScope._variants);
    }

    void operator()(node::TIndex<node::Kind::DEF_CLASS> srceTypeIdx)
    {
        auto& srceType = get(srceTypeIdx);
        auto& destType = _destNodePool.get(
            as<node::Kind::DEF_CLASS>(_stackPtrIn->_index)
        );

        _destNodePool.make(destType._name);
        _stackPtrIn->_index = destType._name;
        base()(srceType._name);

        copyRange(srceType._members,
                  destType._members);
    }

    void operator()(node::TIndex<node::Kind::DEF_FUNCTION> srceFunctionIdx)
    {
        auto& srceFunction = get(srceFunctionIdx);
        auto& destFunction = _destNodePool.get(
            as<node::Kind::DEF_FUNCTION>(_stackPtrIn->_index)
        );

        _destNodePool.make(destFunction._name);
        _stackPtrIn->_index = destFunction._name;
        base()(srceFunction._name);

        copyRange(srceFunction._arguments,
                  destFunction._arguments);

        _destNodePool.make(destFunction._body);
        _stackPtrIn->_index = destFunction._body;
        base()(srceFunction._body);

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
    }

    void operator()(node::TIndex<node::Kind::DEFINITION> srceDefinitionIdx)
    {
        auto& srceDefinition = get(srceDefinitionIdx);
        auto& destDefinition = _destNodePool.get(
            as<node::Kind::DEFINITION>(_stackPtrIn->_index)
        );

        auto blitter = blitter::make(_destNodePool, destDefinition._value);
        _stackPtrIn->_index = blitter(srceDefinition._value);
        base()(srceDefinition._value);

        com::blit(srceDefinition._status,
                  destDefinition._status);
    }

    void operator()(node::TIndex<node::Kind::MODULE> srceModuleIdx)
    {
        auto& srceModule = get(srceModuleIdx);
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

        copyRange(srceModule._imports,
                  destModule._imports);

        copyRange(srceModule._definitions,
                  destModule._definitions);

        _destNodePool.make(destModule._modules, 0);

        // Copy parent path

        com::blitDefault(destModule._parentPath);

        auto parent = as<node::Kind::MODULE>(srceModule._parent);

        while (get(parent)._path)
        {
            auto& parentPath = get(parent)._path;
            auto  prefix     = destModule._parentPath;

                                         _destNodePool.make (destModule._parentPath);
            auto& destModuleParentPath = _destNodePool.get  (destModule._parentPath);

            auto blitter = blitter::make(_destNodePool, destModuleParentPath._expression);
            _stackPtrIn->_index = blitter(parentPath.value());
            base()(parentPath);

            destModuleParentPath._next = prefix;
            parent = as<node::Kind::MODULE>(get(parent)._parent);
        }
    }

    State::NodePool& _destNodePool;
};

} // namespace

void copyDeep(node::TIndex<node::Kind::MODULE>   srceModule,
              State::NodePool                  & srceNodePool,
              node::TIndex<node::Kind::MODULE>   destModule,
              State::NodePool                  & destNodePool)
{
    DeepCopier deepCopier{srceNodePool,
                          destNodePool,
                          destModule};

    deepCopier.base()(srceModule);
}

} // namespace dmit::ast
