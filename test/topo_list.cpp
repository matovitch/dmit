#include "test.hpp"

#include "list/list.hpp"

using CharList = list::TMake<char, 2>;
using CharListCellPool = typename CharList::CellPool;

TEST_CASE("list")
{
    CharListCellPool charListCellPool;

    CharList charList{charListCellPool};

    CHECK(charList.empty() == true);

    charList.emplace_front('k');

    CHECK(charList.empty() == false);

    CHECK(*(charList.begin()) == 'k');

    charList.emplace_front('j');

    CHECK(*(charList.begin()) == 'j');

    charList.emplace_front('l');

    const char* ptr = "ljk";

    for (const auto& aChar : charList)
    {
        CHECK(aChar == *ptr++);
    }

    charList.pop_front();
    charList.pop_front();
    charList.pop_front();

    CHECK(charList.empty() == true);
}
