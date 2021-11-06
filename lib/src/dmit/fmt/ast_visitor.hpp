#include "dmit/fmt/ast/definition_role.hpp"
#include "dmit/fmt/com/unique_id.hpp"
#include "dmit/fmt/src/slice.hpp"

#include "dmit/ast/v_index.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/node.hpp"
#include "dmit/ast/pool.hpp"

#include <sstream>

namespace dmit::fmt
{

namespace
{

struct AstVisitor : ast::TVisitor<AstVisitor>
{
    AstVisitor(ast::State::NodePool& nodePool, std::ostringstream& oss) :
        ast::TVisitor<AstVisitor>{nodePool},
        _oss{oss}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::LEXEME> lexemeIdx)
    {
        _oss << "{\"node\":\"Lexeme\",";
        _oss << "\"slice\":" << getSlice(lexemeIdx) << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER> identifierIdx)
    {
        auto& identifier = get(identifierIdx);

        _oss << "{\"node\":\"Identifier\",";
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

        if (type._status == ast::node::Status::TYPE_BOUND)
        {
            _oss << ",\"id\":\"" << ast::node::v_index::makeId(_nodePool, type._asVIndex) << '"';
        }

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
        _oss << "\"operator\":" ; base()(expBinop._operator ); _oss << ",";
        _oss << "\"lhs\":"      ; base()(expBinop._lhs      ); _oss << ",";
        _oss << "\"rhs\":"      ; base()(expBinop._rhs      ); _oss << "}";
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXPRESSION> expressionIdx)
    {
        auto& expression = get(expressionIdx);

        base()(expression._value);
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
        _oss << "\"typeClaim\":"; base()(dclVariable._typeClaim);
        _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT> scopeVariantIdx)
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
        _oss << "\"name\":"       ; base()(function._name       ); _oss << ',';
        _oss << "\"arguments\":"  ; base()(function._arguments  ); _oss << ',';
        _oss << "\"returnType\":" ; base()(function._returnType ); _oss << ',';
        _oss << "\"body\":"       ; base()(function._body       );

        if (function._status == ast::node::Status::IDENTIFIED)
        {
            _oss << ",\"id\":\"" << function._id << '"';
        }

        _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        auto& defClass = get(defClassIdx);

        _oss << "{\"node\":\"Class\",";
        _oss << "\"name\":"    ; base()(defClass._name    ); _oss << ',';
        _oss << "\"members\":" ; base()(defClass._members );

        if (defClass._status == ast::node::Status::IDENTIFIED)
        {
            _oss << ",\"id\":\"" << defClass._id << '"';
        }

        _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto& import = get(importIdx);

        _oss << "{\"node\":\"Import\",";
        _oss << "\"path\":"; base()(import._path); _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PARENT_PATH> parentPathIdx)
    {
        auto& parentPath = get(parentPathIdx);

        _oss << "{\"node\":\"ParentPath\",";
        _oss << "\"expression\":" ; base()(parentPath._expression ); _oss << ',';
        _oss << "\"next\":"       ; base()(parentPath._next       ); _oss << '}';
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

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
        _oss << "\"modules\":"   ; base()(view._modules);

        if (view._status == ast::node::Status::IDENTIFIED)
        {
            _oss << ",\"id\":\"" << view._id << '"';
        }

        _oss << '}';
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
    void loopPreamble(ast::node::TRange<KIND>&)
    {
        _oss << "[";
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationConclusion(ast::node::TIndex<KIND>)
    {
        _oss << ',';
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void loopIterationPreamble(ast::node::TIndex<KIND>) {}

    std::ostringstream& _oss;
};

} // namespace

} // namespace dmit::fmt
