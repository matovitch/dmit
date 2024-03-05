#include "test.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/src/file.hpp"

#include "dmit/fmt/ast/state.hpp"

#include <cstring>


#include "dmit/cmp/cmp.hpp"
#include "dmit/cmp/ast/state.hpp"


struct Aster
{
    dmit::ast::State operator()(const char* const filePath)
    {
      _files.emplace_back(fileFromPath(filePath));

      std::vector<uint8_t> path;

      return _astFromPathAndSource.make(_files.back()._path,
                                        _files.back()._content);
    }

    dmit::ast::FromPathAndSource _astFromPathAndSource;

    dmit::ast::SourceRegister _sourceRegister;

    std::vector<dmit::src::File> _files;
};

TEST_SUITE("json")
{

TEST_CASE("ast")
{
    Aster aster;

    // cmp_ctx_t cmpBufferSize = {0};

    // auto writerSize = [](cmp_ctx_t* ctx, const void *data, size_t count)
    //                   {
    //                       ctx->buf = (char*)(ctx->buf) + count;
    //                       return count;
    //                   };

    // cmp_init(&cmpBufferSize, nullptr, nullptr, nullptr, writerSize);

    //dmit::cmp::write(&cmpBufferSize, aster("test/data/ast/add.in"));

    // std::cout << *((size_t*)(&(cmpBufferSize.buf))) << '\n';


    auto astWriter = [](cmp_ctx_t* context, const dmit::ast::State& ast) -> bool
                     {
                          return dmit::cmp::write(context, ast);
                     };

    auto astAsBytes = dmit::cmp::asBytes(astWriter, aster("test/data/ast/add.in"));

    CHECK(dmit::fmt::asString(aster("test/data/ast/add.in"       )) == fileAsString("test/data/ast/add.out"       ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/call.in"      )) == fileAsString("test/data/ast/call.out"      ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/class.in"     )) == fileAsString("test/data/ast/class.out"     ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/increment.in" )) == fileAsString("test/data/ast/increment.out" ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/module.in"    )) == fileAsString("test/data/ast/module.out"    ));
}

} // TEST_SUITE("json")
