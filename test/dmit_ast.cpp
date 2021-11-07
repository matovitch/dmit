#include "test.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/fmt/ast/state.hpp"

#include <cstring>

struct Aster
{
    dmit::ast::State operator()(const char* const filePath)
    {
        const auto& toParse = fileAsVector(filePath);

        std::vector<uint8_t> path;

        return _astFromPathAndSource.make(path, toParse);
    }

    dmit::ast::FromPathAndSource _astFromPathAndSource;

    dmit::ast::SourceRegister _sourceRegister;
};

TEST_SUITE("inout")
{

TEST_CASE("ast")
{
    Aster aster;

    CHECK(dmit::fmt::asString(aster("test/data/ast/add.in"       )) == fileAsString("test/data/ast/add.out"       ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/call.in"      )) == fileAsString("test/data/ast/call.out"      ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/class.in"     )) == fileAsString("test/data/ast/class.out"     ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/increment.in" )) == fileAsString("test/data/ast/increment.out" ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/module.in"    )) == fileAsString("test/data/ast/module.out"    ));
}

} // TEST_SUITE("inout")
