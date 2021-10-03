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
    void copyRange(node::TRange<KIND>& destRange,
                   node::TRange<KIND>& srceRange)
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
        _stackPtrIn->_location = std::visit(blitterLhs, srceBinop._lhs);
        base()(srceBinop._lhs);

        auto blitterRhs = blitter::make(_destNodePool, destBinop._rhs);
        _stackPtrIn->_location = std::visit(blitterRhs, srceBinop._rhs);
        base()(srceBinop._rhs);
    }

    void operator()(node::TIndex<node::Kind::DCL_IMPORT> srceImportIdx)
    {
        auto& srceImport = get(srceImportIdx);
        auto& destImport = _destNodePool.get(
            std::get<decltype(srceImportIdx)>(_stackPtrIn->_location)
        );

        auto blitter = blitter::make(_destNodePool, destImport._path);
        _stackPtrIn->_location = std::visit(blitter, srceImport._path);
        base()(srceImport._path);
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

        //_stackPtrIn->_location = destFunction._arguments;
        //base()(srceFunction._arguments);

        if (srceFunction._returnType)
        {
            auto blitter = blitter::make(_destNodePool, destFunction._returnType);
            _stackPtrIn->_location = blitter(srceFunction._returnType.value());
            base()(srceFunction._returnType);
        }
    }

    void operator()(node::TIndex<node::Kind::MODULE> srceModuleIdx)
    {
        auto& srceModule = get(srceModuleIdx);
        auto& destModule = _destNodePool.get(
            std::get<decltype(srceModuleIdx)>(_stackPtrIn->_location)
        );

        copyRange(destModule._imports,
                  srceModule._imports);


        _destNodePool.make(destModule._functions, 0);
        //copyRange(destModule._functions,
        //          srceModule._functions);

        _destNodePool.make(destModule._modules, 0);
        //copyRange(destModule._modules,
        //          srceModule._modules);

        if (srceModule._path)
        {
            auto blitter = blitter::make(_destNodePool, destModule._path);
            _stackPtrIn->_location = std::visit(blitter, srceModule._path.value());
            base()(srceModule._path);
        }
        else
        {
            com::blitDefault(destModule._path);
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