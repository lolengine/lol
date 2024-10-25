#include <lol/lib/doctest>
#include <lol/narray>

TEST_CASE("array2d")
{
    auto a = lol::array2d<int>(3, 2);

    for (int n = 0; n < 6; ++n)
        a[n] = n;

    CHECK(a(0, 0) == 0);
    CHECK(a(1, 0) == 1);
    CHECK(a(2, 0) == 2);

    CHECK(a(0, 1) == 3);
    CHECK(a(1, 1) == 4);
    CHECK(a(2, 1) == 5);
}
