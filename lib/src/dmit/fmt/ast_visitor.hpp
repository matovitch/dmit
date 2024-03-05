#include "dmit/fmt/ast/definition_role.hpp"
#include "dmit/fmt/com/unique_id.hpp"
#include "dmit/fmt/src/slice.hpp"

#include "dmit/ast/definition_role.hpp"
#include "dmit/ast/v_index.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/sem/interface_map.hpp"

#include "dmit/com/reference.hpp"
#include "dmit/com/tree_node.hpp"

#include <sstream>

namespace dmit::fmt
{

namespace
{

struct AstVisitor : ast::TVisitor<AstVisitor>
{
    AstVisitor(const ast::State& ast, std::ostringstream& oss) :
        ast::TVisitor<AstVisitor>{ast._nodePool},
        _interfacePoolOpt{},
        _oss{oss}
    {}

    AstVisitor(const ast::Bundle& bundle, std::ostringstream& oss) :
        ast::TVisitor<AstVisitor>{bundle._nodePool},
        _interfacePoolOpt{bundle._interfacePoolOpt},
        _oss{oss}
    {}

    AstVisitor(sem::InterfaceMap& interfaceMap, std::ostringstream& oss) :
        ast::TVisitor<AstVisitor>{interfaceMap._astNodePool},
        _interfacePoolOpt{},
        _oss{oss}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::SOURCE>) {}

    void operator()(ast::node::TIndex<ast::node::Kind::LEXEME> lexemeIdx)
    {
        _oss << "{\"node\":\"Lexeme\",";
        _oss << "\"slice\":" << getSlice(lexemeIdx) << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        auto& identifier = get(identifierIdx);

        _oss << "{\"node\":\"Identifier\",";

        if (identifier._status == ast::node::Status::BOUND)
        {
            _oss << "\"id\":\"";

            auto idenfifierVIndex = std::get<ast::node::VIndex>(identifier._asVIndexOrLock);

            if (com::tree::v_index::isInterface<ast::node::Kind>(idenfifierVIndex) && _interfacePoolOpt)
            {
                _oss << ast::node::v_index::makeId(_interfacePoolOpt.value().get(), idenfifierVIndex);
            }
            else
            {
                _oss << ast::node::v_index::makeId(_nodePool, idenfifierVIndex);
            }

            _oss << "\",";
        }

        _oss << "\"lexeme\":"; base()(identifier._lexeme);
        _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_DECIMAL> decimalIdx)
    {
        auto& decimal = get(decimalIdx);

        _oss << "{\"node\":\"Decimal\",";
        _oss << "\"lexeme\":"; base()(decimal._lexeme);
        _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER> integerIdx)
    {
        auto& integer = get(integerIdx);

        _oss << "{\"node\":\"Integer\",";
        _oss << "\"lexeme\":"; base()(integer._lexeme);
        _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE> typeIdx)
    {
        auto& type = get(typeIdx);

        _oss << "{\"node\":\"Type\",";
        _oss << "\"name\":"; base()(type._name);
        _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        auto& typeClaim = get(typeClaimIdx);

        _oss << "{\"node\":\"Type Claim\",";
        _oss << "\"variable\":" ; base()(typeClaim._variable ); _oss << ',';
        _oss << "\"type\":"     ; base()(typeClaim._type     ); _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::STM_RETURN> stmReturnIdx)
    {
        auto& stmReturn = get(stmReturnIdx);

        _oss << "{\"node\":\"Return Statement\",";
        _oss << "\"expression\":"; base()(stmReturn._expression);
        _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PATTERN> patternIdx)
    {
        auto& pattern = get(patternIdx);

        _oss << "{\"node\":\"Pattern\",";
        _oss << "\"variable\":"; base()(pattern._variable);
        _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_MONOP> expMonopIdx)
    {
        auto& expMonop = get(expMonopIdx);

        _oss << "{\"node\":\"Unary Operation\",";
        _oss << "\"operator\":"   ; base()(expMonop._operator   ); _oss << ",";
        _oss << "\"expression\":" ; base()(expMonop._expression ); _oss << ",";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> expBinopIdx)
    {
        auto& expBinop = get(expBinopIdx);

        _oss << "{\"node\":\"Binary Operation\",";

        if (expBinop._status == ast::node::Status::BOUND)
        {
            _oss << "\"id\":\"";

            if (expBinop._asFunction._isInterface && _interfacePoolOpt)
            {
                _oss << ast::node::v_index::makeId(_interfacePoolOpt.value().get(), expBinop._asFunction);
            }
            else
            {
                _oss << ast::node::v_index::makeId(_nodePool, expBinop._asFunction);
            }

            _oss << "\",";
        }

        _oss << "\"operator\":" ; base()(expBinop._operator ); _oss << ",";
        _oss << "\"lhs\":"      ; base()(expBinop._lhs      ); _oss << ",";
        _oss << "\"rhs\":"      ; base()(expBinop._rhs      ); _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::FUN_CALL> funCallIdx)
    {
        auto& funCall = get(funCallIdx);

        _oss << "{\"node\":\"Function Call\",";
        _oss << "\"callee\":"    ; base()(funCall._callee    ); _oss << ",";
        _oss << "\"arguments\":" ; base()(funCall._arguments ); _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE> dclVariableIdx)
    {
        auto& dclVariable = get(dclVariableIdx);

        _oss << "{\"node\":\"Variable Declaration\",";

        if (dclVariable._status == ast::node::Status::IDENTIFIED)
        {
            _oss << "\"id\":\"" << dclVariable._id << "\",";
        }

        _oss << "\"typeClaim\":"; base()(dclVariable._typeClaim);
        _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::ANY> scopeVariantIdx)
    {
        auto& scopeVariant = get(scopeVariantIdx);

        base()(scopeVariant._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        auto& scope = get(scopeIdx);

        base()(scope._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        _oss << "{\"node\":\"Function\",";

        if (function._status == ast::node::Status::IDENTIFIED)
        {
            _oss << "\"id\":\"" << function._id << "\",";
        }

        _oss << "\"name\":"       ; base()(function._name       ); _oss << ',';
        _oss << "\"arguments\":"  ; base()(function._arguments  ); _oss << ',';
        _oss << "\"returnType\":" ; base()(function._returnType );

        if (!functionIdx._isInterface)
        {
            _oss << ",\"body\":"; base()(function._body);
        }

        _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        auto& defClass = get(defClassIdx);

        _oss << "{\"node\":\"Class\",";

        if (defClass._status == ast::node::Status::IDENTIFIED)
        {
            _oss << "\"id\":\"" << defClass._id << "\",";
        }

        _oss << "\"name\":"    ; base()(defClass._name    ); _oss << ',';
        _oss << "\"members\":" ; base()(defClass._members ); _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = get(importIdx);

        _oss << "{\"node\":\"Import\",";

        if (import._status == ast::node::Status::IDENTIFIED)
        {
            _oss << "\"id\":\"" << import._id << "\",";
        }

        _oss << "\"path\":"; base()(import._path); _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PARENT_PATH> parentPathIdx)
    {
        auto& parentPath = get(parentPathIdx);

        base()(parentPath._expression);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        if (definitionIdx._isInterface && definition._role != ast::DefinitionRole::EXPORTED)
        {
            _oss << "{}";
            return;
        }

        _oss << "{\"node\":\"Definition\",";
        _oss << "\"role\":" ; _oss << definition._role ; _oss << ',';
        _oss << "\"value\":"  ; base()(definition._value)  ; _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = get(moduleIdx);

        _oss << "{\"node\":\"Module\",";
        _oss << "\"path\":"        ; base()(module._path        ); _oss << ',';
        _oss << "\"parentPath\":"  ; base()(module._parentPath  ); _oss << ',';
        _oss << "\"imports\":"     ; base()(module._imports     ); _oss << ',';
        _oss << "\"definitions\":" ; base()(module._definitions ); _oss << ',';
        _oss << "\"modules\":"     ; base()(module._modules     ); _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        auto& view = get(viewIdx);

        _oss << "{\"node\":\"View\",";

        if (view._status == ast::node::Status::IDENTIFIED)
        {
            _oss << "\"id\":\"" << view._id << "\",";
        }

        _oss << "\"modules\":"; base()(view._modules); _oss << '}';
    }

    template <class Type>
    void emptyOption()
    {
        _oss << "{}";
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopConclusion(ast::node::TRange<KIND>& range)
    {
        _oss.seekp(range._size ? -1 : 0, std::ios_base::end);

        _oss << "]";
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopConclusion(ast::node::TList<KIND>& list)
    {
        _oss.seekp(empty(list) ? 0 : -1, std::ios_base::end);

        _oss << "]";
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopPreamble(ast::node::TRange<KIND>&)
    {
        _oss << "[";
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopPreamble(ast::node::TList<KIND>&)
    {
        _oss << "[";
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationConclusion(ast::node::TIndex<KIND>)
    {
        _oss << ',';
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationConclusionList(ast::node::TIndex<KIND>)
    {
        _oss << ',';
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationPreamble(ast::node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationPreambleList(ast::node::TIndex<KIND>) {}

    com::TOptionRef<ast::State::NodePool> _interfacePoolOpt;

    std::ostringstream& _oss;
};

} // namespace

} // namespace dmit::fmt
