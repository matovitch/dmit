#include "dmit/ast/deep_copy.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

namespace dmit::ast
{

namespace
{

template <class Type>
struct TBlitter
{
    TBlitter(State::NodePool& nodePool, Type& value) :
        _nodePool{nodePool},
        _value{value}
    {}

    template <com::TEnumIntegerType<node::Kind> NODE_KIND>
    node::Location operator()(node::TIndex<NODE_KIND>)
    {
        node::TIndex<NODE_KIND> nodeIndex;

        _nodePool.make(nodeIndex);

        com::blit(nodeIndex, _value);

        return node::Location{nodeIndex};
    }

    template <class... Types>
    node::Location operator()(std::variant<Types...>& variant)
    {
        return std::visit(*this, variant);
    }

    State::NodePool & _nodePool;
    Type            & _value;
};

namespace blitter
{

template<class Type>
TBlitter<Type> make(State::NodePool& nodePool, Type& value)
{
    return TBlitter<Type>{nodePool, value};
}

} // namespace blitter

struct Stack
{
    node::Location _location;
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
        _stackPtrIn->_location = destRange[0];
        base()(srceRange);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationConclusion(node::TIndex<KIND>)
    {
        std::get<node::TIndex<KIND>>(_stackPtrIn->_location)._value++;
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
        DMIT_COM_ASSERT(!"Not implemented");
    }

    void operator()(node::TIndex<node::Kind::SOURCE> srceSourceIdx)
    {
        auto& srceSource = get(srceSourceIdx);
        auto& destSource = _destNodePool.get(
            std::get<decltype(srceSourceIdx)>(_stackPtrIn->_location)
        );

        std::memcpy(&destSource, &srceSource, sizeof(TNode<node::Kind::SOURCE>));
    }

    void operator()(node::TIndex<node::Kind::LEXEME> srceLexemeIdx)
    {
        auto& srceLexeme = get(srceLexemeIdx);
        auto& destLexeme = _destNodePool.get(
            std::get<decltype(srceLexemeIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destLexeme._source);
        _stackPtrIn->_location = destLexeme._source;
        base()(srceLexeme._source);

        destLexeme._index = srceLexeme._index;
    }

    void operator()(node::TIndex<node::Kind::LIT_INTEGER> srceIntegerIdx)
    {
        auto& srceInteger = get(srceIntegerIdx);
        auto& destInteger = _destNodePool.get(
            std::get<decltype(srceIntegerIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destInteger._lexeme);
        _stackPtrIn->_location = destInteger._lexeme;
        base()(srceInteger._lexeme);
    }

    void operator()(node::TIndex<node::Kind::LIT_DECIMAL> srceDecimalIdx)
    {
        auto& srceDecimal = get(srceDecimalIdx);
        auto& destDecimal = _destNodePool.get(
            std::get<decltype(srceDecimalIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destDecimal._lexeme);
        _stackPtrIn->_location = destDecimal._lexeme;
        base()(srceDecimal._lexeme);
    }

    void operator()(node::TIndex<node::Kind::LIT_IDENTIFIER> srceIdentifierIdx)
    {
        auto& srceIdentifier = get(srceIdentifierIdx);
        auto& destIdentifier = _destNodePool.get(
            std::get<decltype(srceIdentifierIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destIdentifier._lexeme);
        _stackPtrIn->_location = destIdentifier._lexeme;
        base()(srceIdentifier._lexeme);
    }

    void operator()(node::TIndex<node::Kind::EXP_MONOP> srceMonopIdx)
    {
        auto& srceMonop = get(srceMonopIdx);
        auto& destMonop = _destNodePool.get(
            std::get<decltype(srceMonopIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destMonop._operator);
        _stackPtrIn->_location = destMonop._operator;
        base()(srceMonop._operator);

        auto blitterLhs = blitter::make(_destNodePool, destMonop._expression);
        _stackPtrIn->_location = blitterLhs(srceMonop._expression);
        base()(srceMonop._expression);
    }

    void operator()(node::TIndex<node::Kind::EXP_BINOP> srceBinopIdx)
    {
        auto& srceBinop = get(srceBinopIdx);
        auto& destBinop = _destNodePool.get(
            std::get<decltype(srceBinopIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destBinop._operator);
        _stackPtrIn->_location = destBinop._operator;
        base()(srceBinop._operator);

        auto blitterLhs = blitter::make(_destNodePool, destBinop._lhs);
        _stackPtrIn->_location = blitterLhs(srceBinop._lhs);
        base()(srceBinop._lhs);

        auto blitterRhs = blitter::make(_destNodePool, destBinop._rhs);
        _stackPtrIn->_location = blitterRhs(srceBinop._rhs);
        base()(srceBinop._rhs);
    }

    void operator()(node::TIndex<node::Kind::DCL_IMPORT> srceImportIdx)
    {
        auto& srceImport = get(srceImportIdx);
        auto& destImport = _destNodePool.get(
            std::get<decltype(srceImportIdx)>(_stackPtrIn->_location)
        );

        auto blitter = blitter::make(_destNodePool, destImport._path);
        _stackPtrIn->_location = blitter(srceImport._path);
        base()(srceImport._path);
    }

    void operator()(node::TIndex<node::Kind::TYPE_CLAIM> srceTypeClaimIdx)
    {
        auto& srceTypeClaim = get(srceTypeClaimIdx);
        auto& destTypeClaim = _destNodePool.get(
            std::get<decltype(srceTypeClaimIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destTypeClaim._variable);
        _stackPtrIn->_location = destTypeClaim._variable;
        base()(srceTypeClaim._variable);

        _destNodePool.make(destTypeClaim._type);
        _stackPtrIn->_location = destTypeClaim._type;
        base()(srceTypeClaim._type);
    }

    void operator()(node::TIndex<node::Kind::EXPRESSION> srceExpressionIdx)
    {
        auto& srceExpression = get(srceExpressionIdx);
        auto& destExpression = _destNodePool.get(
            std::get<decltype(srceExpressionIdx)>(_stackPtrIn->_location)
        );

        auto blitter = blitter::make(_destNodePool, destExpression._value);
        _stackPtrIn->_location = blitter(srceExpression._value);
        base()(srceExpression._value);
    }

    void operator()(node::TIndex<node::Kind::FUN_CALL> srceFunCallIdx)
    {
        auto& srceFunCall = get(srceFunCallIdx);
        auto& destFunCall = _destNodePool.get(
            std::get<decltype(srceFunCallIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destFunCall._callee);
        _stackPtrIn->_location = destFunCall._callee;
        base()(srceFunCall._callee);

        copyRange(srceFunCall._arguments,
                  destFunCall._arguments);
    }

    void operator()(node::TIndex<node::Kind::DCL_VARIABLE> srceDclVariableIdx)
    {
        auto& srceDclVariable = get(srceDclVariableIdx);
        auto& destDclVariable = _destNodePool.get(
            std::get<decltype(srceDclVariableIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destDclVariable._typeClaim);
        _stackPtrIn->_location = destDclVariable._typeClaim;
        base()(srceDclVariable._typeClaim);
    }

    void operator()(node::TIndex<node::Kind::STM_RETURN> srceStmReturnIdx)
    {
        auto& srceStmReturn = get(srceStmReturnIdx);
        auto& destStmReturn = _destNodePool.get(
            std::get<decltype(srceStmReturnIdx)>(_stackPtrIn->_location)
        );

        auto blitter = blitter::make(_destNodePool, destStmReturn._expression);
        _stackPtrIn->_location = blitter(srceStmReturn._expression);
        base()(srceStmReturn._expression);
    }

    void operator()(node::TIndex<node::Kind::SCOPE_VARIANT> srceScopeVariantIdx)
    {
        auto& srceScopeVariant = get(srceScopeVariantIdx);
        auto& destScopeVariant = _destNodePool.get(
            std::get<decltype(srceScopeVariantIdx)>(_stackPtrIn->_location)
        );

        auto blitter = blitter::make(_destNodePool, destScopeVariant._value);
        _stackPtrIn->_location = blitter(srceScopeVariant._value);
        base()(srceScopeVariant._value);
    }

    void operator()(node::TIndex<node::Kind::SCOPE> srceScopeIdx)
    {
        auto& srceScope = get(srceScopeIdx);
        auto& destScope = _destNodePool.get(
            std::get<decltype(srceScopeIdx)>(_stackPtrIn->_location)
        );

        copyRange(srceScope._variants,
                  destScope._variants);
    }

    void operator()(node::TIndex<node::Kind::TYP_DEFINITION> srceTypeIdx)
    {
        auto& srceType = get(srceTypeIdx);
        auto& destType = _destNodePool.get(
            std::get<decltype(srceTypeIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destType._name);
        _stackPtrIn->_location = destType._name;
        base()(srceType._name);

        copyRange(srceType._members,
                  destType._members);
    }

    void operator()(node::TIndex<node::Kind::FUN_DEFINITION> srceFunctionIdx)
    {
        auto& srceFunction = get(srceFunctionIdx);
        auto& destFunction = _destNodePool.get(
            std::get<decltype(srceFunctionIdx)>(_stackPtrIn->_location)
        );

        _destNodePool.make(destFunction._name);
        _stackPtrIn->_location = destFunction._name;
        base()(srceFunction._name);

        copyRange(srceFunction._arguments,
                  destFunction._arguments);

        _destNodePool.make(destFunction._body);
        _stackPtrIn->_location = destFunction._body;
        base()(srceFunction._body);

        if (srceFunction._returnType)
        {
            auto blitter = blitter::make(_destNodePool, destFunction._returnType);
            _stackPtrIn->_location = blitter(srceFunction._returnType.value());
            base()(srceFunction._returnType);
        }
        else
        {
            com::blitDefault(destFunction._returnType);
        }

        com::blit(srceFunction._status,
                  destFunction._status);
    }

    void operator()(node::TIndex<node::Kind::MODULE> srceModuleIdx)
    {
        auto& srceModule = get(srceModuleIdx);
        auto& destModule = _destNodePool.get(
            std::get<decltype(srceModuleIdx)>(_stackPtrIn->_location)
        );

        if (srceModule._path)
        {
            auto blitter = blitter::make(_destNodePool, destModule._path);
            _stackPtrIn->_location = blitter(srceModule._path.value());
            base()(srceModule._path);
        }
        else
        {
            com::blitDefault(destModule._path);
        }

        copyRange(srceModule._imports,
                  destModule._imports);

        copyRange(srceModule._types,
                  destModule._types);

        copyRange(srceModule._functions,
                  destModule._functions);

        _destNodePool.make(destModule._modules, 0);

        // Copy parent path

        com::blitDefault(destModule._parentPath);

        auto parent = std::get<decltype(srceModuleIdx)>(srceModule._parent);

        while (get(parent)._path)
        {
            auto& parentPath = get(parent)._path;
            auto  prefix     = destModule._parentPath;

                                         _destNodePool.make (destModule._parentPath);
            auto& destModuleParentPath = _destNodePool.get  (destModule._parentPath);

            auto blitter = blitter::make(_destNodePool, destModuleParentPath._expression);
            _stackPtrIn->_location = blitter(parentPath.value());
            base()(parentPath);

            destModuleParentPath._next = prefix;
            parent = std::get<decltype(srceModuleIdx)>(get(parent)._parent);
        }
    }

    State::NodePool& _destNodePool;
};

} // namespace

void deepCopy(node::TIndex<node::Kind::MODULE>   srceModule,
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
