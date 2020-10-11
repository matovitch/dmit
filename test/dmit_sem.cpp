#include "test.hpp"

#include "dmit/sem/scheduler.hpp"
#include "dmit/sem/analyze.hpp"
#include "dmit/sem/message.hpp"
#include "dmit/sem/task.hpp"
#include "dmit/sem/work.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include "dmit/src/partition.hpp"

TEST_CASE("sem")
{
    dmit::ast::state::Builder aster;
    dmit::prs::state::Builder parser;
    dmit::lex::state::Builder lexer;

    const auto& toParse = fileAsString("test/data/ast/increment.in");

    const auto toParseAsBytes = reinterpret_cast<const uint8_t*>(toParse.data());

    const auto& lex = lexer(toParseAsBytes,
                            toParse.size());

    const auto& prs = parser(lex._tokens);

    auto& ast = aster(prs._tree);

    dmit::src::Partition partition{toParseAsBytes, lex._offsets};

    dmit::sem::analyze(partition, ast);
}

TEST_CASE("semSched")
{
    dmit::sem::message::TPool<char> messagePool;

    auto& mesgA = messagePool.make();
    auto& mesgB = messagePool.make();
    auto& mesgC = messagePool.make();

    dmit::sem::TWork<char> workA{[]{std::cout << "A\n"; return 'A';}, mesgA};
    dmit::sem::TWork<char> workB{[]{std::cout << "B\n"; return 'B';}, mesgB};
    dmit::sem::TWork<char> workC{[]{std::cout << "C\n"; return 'C';}, mesgC};

    dmit::sem::Scheduler scheduler;

    dmit::sem::task::TPool<char> taskPool;

    auto taskA = scheduler.makeTask(taskPool);
    auto taskB = scheduler.makeTask(taskPool);
    auto taskC = scheduler.makeTask(taskPool);

    taskA().assignWork(workA);
    taskB().assignWork(workB);
    taskC().assignWork(workC);

    /*auto depAB = */scheduler.attach(taskA, taskB);
    /*auto depCA = */scheduler.attach(taskC, taskA);

    mesgC.send(); // simulate for refCount

    scheduler.run();

    std::cout << mesgA.read() << '\n';
    std::cout << mesgB.read() << '\n';
    std::cout << mesgC.read() << '\n';
}
