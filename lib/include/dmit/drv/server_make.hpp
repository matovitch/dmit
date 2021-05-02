#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/lex/state.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/db/database.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <vector>

namespace dmit::drv::srv
{

void makeAst(const std::vector<uint8_t> & path  ,
             const std::vector<uint8_t> & toParse)
{
    dmit::lex::state::Builder lexer;
    dmit::prs::state::Builder parser;
    dmit::ast::state::Builder aster;

    dmit::ast::SourceRegister sourceRegister;

    auto&& lex = lexer(toParse.data(),
                       toParse.size());

    auto&& prs = parser(lex._tokens);

    auto&& ast = aster(prs._tree);

    auto& source = ast._nodePool.get(ast._source);

    sourceRegister.add(source);

    source._srcPath = path;

    source._srcContent.resize(toParse.size());

    std::memcpy(source._srcContent.data(), toParse.data(), toParse.size());

    source._srcOffsets = dmit::src::line_index::makeOffsets(source._srcContent);

    source._lexOffsets .swap(lex._offsets );
    source._lexTokens  .swap(lex._tokens  );

    DMIT_COM_LOG_OUT << ast << '\n';
}

void make(dmit::nng::Socket& socket, dmit::db::Database& database)
{
    // 1. Process query

    int errorCode;

    std::vector<dmit::com::UniqueId  > unitIds ;
    std::vector<std::vector<uint8_t> > paths   ;
    std::vector<std::vector<uint8_t> > sources ;

    if ((errorCode = database.selectUnitIdsPathsSources(unitIds,
                                                        paths,
                                                        sources)) != SQLITE_OK)
    {
        displaySqlite3Error("selectUnitIdsPathsSources", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    for (int i = 0; i < paths.size(); i++)
    {
        makeAst(paths[i], sources[i]);
    }

    // 2. Write reply

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
