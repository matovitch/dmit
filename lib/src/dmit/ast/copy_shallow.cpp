#include "dmit/ast/copy_shallow.hpp"

#include "dmit/ast/blitter.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/com/blit.hpp"
#include "dmit/ast/node.hpp"

namespace dmit::ast
{

namespace
{

constexpr bool K_IS_INTERFACE = true;

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
        make(destRange, srceRange._size);
        _stackPtrIn->_index = destRange[0];
        base()(srceRange);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void copyList(node::TList<KIND>& srceList,
                  node::TList<KIND>& destList)
    {
        _destNodePool.make(destList);

        auto curr = srceList._begin;

        while (get(curr)._next != srceList._begin)
        {
            curr = get(curr)._next;
            grow(destList);
            _destNodePool.back(destList)._isInterface = true;
        }

        _stackPtrIn->_index = _destNodePool.back(destList);
        base()(srceList);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationConclusion(node::TIndex<KIND>)
    {
        _stackPtrIn->_index._value++;
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationConclusionList(node::TIndex<KIND>)
    {
        _stackPtrIn->_index._value--;
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopPreamble(node::TRange<KIND>&){}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopConclusion(node::TRange<KIND>&){}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopPreamble(node::TList<KIND>&){}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopConclusion(node::TList<KIND>&){}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationPreamble(node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationPreambleList(node::TIndex<KIND>) {}

    template <class Type>
    void emptyOption() {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void make(node::TIndex<KIND>& index)
    {
        _destNodePool.make(index);
        index._isInterface = true;
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void make(std::optional<node::TIndex<KIND>>& indexOpt)
    {
        _destNodePool.make(indexOpt);
        indexOpt.value()._isInterface = true;
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void make(node::TRange<KIND>& range, const uint32_t size)
    {
        _destNodePool.make(range, size);
        range._index._isInterface = true;
    }

    template<class Type>
    auto makeBlitter(Type& value)
    {
        return blitter::make<Type, K_IS_INTERFACE>(_destNodePool, value);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void grow(node::TList<KIND>& list)
    {
        _destNodePool.grow(list);
        _destNodePool.back(list)._isInterface = true;
    }

    template <com::TEnumIntegerType<node::Kind> NODE_KIND>
    void operator()(node::TIndex<NODE_KIND>)
    {
        DMIT_COM_ASSERT(!"[AST] Cannot shallow copy node");
    }

    void operator()(node::TIndex<node::Kind::SOURCE> srceSourceIdx)
    {
        auto& srceSource = get(srceSourceIdx);
        auto& destSource = _destNodePool.get(
            node::as<node::Kind::SOURCE>(_stackPtrIn->_index)
        );

        std::memcpy(&destSource, &srceSource, sizeof(TNode<node::Kind::SOURCE>));
    }

    void operator()(node::TIndex<node::Kind::LEXEME> srceLexemeIdx)
    {
        auto& srceLexeme = get(srceLexemeIdx);
        auto& destLexeme = _destNodePool.get(
            node::as<node::Kind::LEXEME>(_stackPtrIn->_index)
        );

        make(destLexeme._source);
        _stackPtrIn->_index = destLexeme._source;
        base()(srceLexeme._source);

        destLexeme._index = srceLexeme._index;
    }

    void operator()(node::TIndex<node::Kind::IDENTIFIER> srceIdentifierIdx)
    {
        auto& srceIdentifier = get(srceIdentifierIdx);
        auto& destIdentifier = _destNodePool.get(
            node::as<node::Kind::IDENTIFIER>(_stackPtrIn->_index)
        );

        make(destIdentifier._lexeme);
        _stackPtrIn->_index = destIdentifier._lexeme;
        base()(srceIdentifier._lexeme);

        destIdentifier._status = node::Status::ASTED;
    }

    void operator()(node::TIndex<node::Kind::EXP_BINOP> srceBinopIdx)
    {
        auto& srceBinop = get(srceBinopIdx);
        auto& destBinop = _destNodePool.get(
            node::as<node::Kind::EXP_BINOP>(_stackPtrIn->_index)
        );

        make(destBinop._operator);
        _stackPtrIn->_index = destBinop._operator;
        base()(srceBinop._operator);

        auto blitterLhs = makeBlitter(destBinop._lhs);
        _stackPtrIn->_index = blitterLhs(srceBinop._lhs);
        base()(srceBinop._lhs);

        auto blitterRhs = makeBlitter(destBinop._rhs);
        _stackPtrIn->_index = blitterRhs(srceBinop._rhs);
        base()(srceBinop._rhs);

        destBinop._status = node::Status::ASTED;
    }

    void operator()(node::TIndex<node::Kind::TYPE> srceTypeIdx)
    {
        auto& srceType = get(srceTypeIdx);
        auto& destType = _destNodePool.get(
            node::as<node::Kind::TYPE>(_stackPtrIn->_index)
        );

        make(destType._name);
        _stackPtrIn->_index = destType._name;
        base()(srceType._name);
    }

    void operator()(node::TIndex<node::Kind::TYPE_CLAIM> srceTypeClaimIdx)
    {
        auto& srceTypeClaim = get(srceTypeClaimIdx);
        auto& destTypeClaim = _destNodePool.get(
            node::as<node::Kind::TYPE_CLAIM>(_stackPtrIn->_index)
        );

        make(destTypeClaim._variable);
        _stackPtrIn->_index = destTypeClaim._variable;
        base()(srceTypeClaim._variable);

        make(destTypeClaim._type);
        _stackPtrIn->_index = destTypeClaim._type;
        base()(srceTypeClaim._type);
    }

    void operator()(node::TIndex<node::Kind::DCL_VARIABLE> srceDclVariableIdx)
    {
        auto& srceDclVariable = get(srceDclVariableIdx);
        auto& destDclVariable = _destNodePool.get(
            node::as<node::Kind::DCL_VARIABLE>(_stackPtrIn->_index)
        );

        make(destDclVariable._typeClaim);
        _stackPtrIn->_index = destDclVariable._typeClaim;
        base()(srceDclVariable._typeClaim);

        destDclVariable._status = node::Status::ASTED;
    }

    void operator()(node::TIndex<node::Kind::DCL_IMPORT> srceImportIdx)
    {
        auto& srceImport = get(srceImportIdx);
        auto& destImport = _destNodePool.get(
            node::as<node::Kind::DCL_IMPORT>(_stackPtrIn->_index)
        );

        auto blitter = makeBlitter(destImport._path);
        _stackPtrIn->_index = blitter(srceImport._path);
        base()(srceImport._path);

        com::blit(srceImport._id,
                  destImport._id);

        destImport._status =
        srceImport._status;
    }

    void operator()(node::TIndex<node::Kind::DEF_FUNCTION> srceFunctionIdx)
    {
        auto& srceFunction = get(srceFunctionIdx);
        auto& destFunction = _destNodePool.get(
            node::as<node::Kind::DEF_FUNCTION>(_stackPtrIn->_index)
        );

        make(destFunction._name);
        _stackPtrIn->_index = destFunction._name;
        base()(srceFunction._name);

        copyRange(srceFunction._arguments,
                  destFunction._arguments);

        if (srceFunction._returnType)
        {
            auto blitter = makeBlitter(destFunction._returnType);
            _stackPtrIn->_index = blitter(srceFunction._returnType.value());
            base()(srceFunction._returnType);
        }
        else
        {
            com::blitDefault(destFunction._returnType);
        }

        destFunction._status = node::Status::ASTED;
    }

    void operator()(node::TIndex<node::Kind::DEF_CLASS> srceDefClassIdx)
    {
        auto& srceDefClass = get(srceDefClassIdx);
        auto& destDefClass = _destNodePool.get(
            node::as<node::Kind::DEF_CLASS>(_stackPtrIn->_index)
        );

        make(destDefClass._name);
        _stackPtrIn->_index = destDefClass._name;
        base()(srceDefClass._name);

        copyRange(srceDefClass._members,
                  destDefClass._members);

        destDefClass._status = node::Status::ASTED;
    }

    void operator()(node::TIndex<node::Kind::PARENT_PATH> srceParentPathIdx)
    {
        auto& srceParentPath = get(srceParentPathIdx);
        auto& destParentPath = _destNodePool.get(
            node::as<node::Kind::PARENT_PATH>(_stackPtrIn->_index)
        );

        auto blitter = makeBlitter(destParentPath._expression);
        _stackPtrIn->_index = blitter(srceParentPath._expression);
        base()(srceParentPath._expression);
    }

    void operator()(node::TIndex<node::Kind::DEFINITION> srceDefinitionIdx)
    {
        auto& srceDefinition = get(srceDefinitionIdx);
        auto& destDefinition = _destNodePool.get(
            node::as<node::Kind::DEFINITION>(_stackPtrIn->_index)
        );

        com::blit(srceDefinition._role,
                  destDefinition._role);

        if (srceDefinition._role != DefinitionRole::EXPORTED)
        {
            return;
        }

        auto blitter = makeBlitter(destDefinition._value);
        _stackPtrIn->_index = blitter(srceDefinition._value);
        base()(srceDefinition._value);
    }

    void operator()(node::TIndex<node::Kind::MODULE> srceModuleIdx)
    {
        auto& srceModule = get(srceModuleIdx)   ;
        auto& destModule = _destNodePool.get(
            node::as<node::Kind::MODULE>(_stackPtrIn->_index)
        );

        if (srceModule._path)
        {
            auto blitter = makeBlitter(destModule._path);
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

        copyList(srceModule._parentPath,
                 destModule._parentPath);

        make(destModule._modules, 0);

        destModule._status =
        srceModule._status;
    }

    void operator()(node::TIndex<node::Kind::VIEW> srceViewIdx)
    {
        auto& srceView = get(srceViewIdx);
        auto& destView = _destNodePool.get(
            node::as<node::Kind::VIEW>(_stackPtrIn->_index)
        );

        copyRange(srceView._modules,
                  destView._modules);

        com::blit(srceView._id,
                  destView._id);

        destView._status =
        srceView._status;
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
