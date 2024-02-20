#include <lol/lib/doctest_main>
#include <lol/audio/stream>

TEST_CASE("sample conversion: float|double ←→ float|double")
{
    auto cv1 = lol::audio::sample::convert<float, float>;
    CHECK(cv1(-1.0f) == -1.0f);
    CHECK(cv1(-0.5f) == -0.5f);
    CHECK(cv1( 0.0f) ==  0.0f);
    CHECK(cv1( 0.5f) ==  0.5f);
    CHECK(cv1( 1.0f) ==  1.0f);

    auto cv2 = lol::audio::sample::convert<float, double>;
    CHECK(cv2(-1.0f) == -1.0);
    CHECK(cv2(-0.5f) == -0.5);
    CHECK(cv2( 0.0f) ==  0.0);
    CHECK(cv2( 0.5f) ==  0.5);
    CHECK(cv2( 1.0f) ==  1.0);

    auto cv3 = lol::audio::sample::convert<double, float>;
    CHECK(cv3(-1.0) == -1.0f);
    CHECK(cv3(-0.5) == -0.5f);
    CHECK(cv3( 0.0) ==  0.0f);
    CHECK(cv3( 0.5) ==  0.5f);
    CHECK(cv3( 1.0) ==  1.0f);

    auto cv4 = lol::audio::sample::convert<double, double>;
    CHECK(cv4(-1.0) == -1.0);
    CHECK(cv4(-0.5) == -0.5);
    CHECK(cv4( 0.0) ==  0.0);
    CHECK(cv4( 0.5) ==  0.5);
    CHECK(cv4( 1.0) ==  1.0);
}

TEST_CASE("sample conversion: float → int8|uint8")
{
    auto cv1 = lol::audio::sample::convert<float, int8_t>;
    CHECK(cv1(-1.5f) == -0x80);
    CHECK(cv1(-1.0f) == -0x80);
    CHECK(cv1(-0.5f) == -0x40);
    CHECK(cv1( 0.0f) ==  0x00);
    CHECK(cv1( 0.5f) ==  0x40);
    CHECK(cv1( 1.0f) ==  0x7f);
    CHECK(cv1( 1.5f) ==  0x7f);

    auto cv2 = lol::audio::sample::convert<float, uint8_t>;
    CHECK(cv2(-1.5f) == 0x00);
    CHECK(cv2(-1.0f) == 0x00);
    CHECK(cv2(-0.5f) == 0x40);
    CHECK(cv2( 0.0f) == 0x80);
    CHECK(cv2( 0.5f) == 0xc0);
    CHECK(cv2( 1.0f) == 0xff);
    CHECK(cv2( 1.5f) == 0xff);
}

TEST_CASE("sample conversion: int8|uint8 → float")
{
    auto cv1 = lol::audio::sample::convert<int8_t, float>;
    CHECK(cv1(-0x80) == -1.0f);
    CHECK(cv1( 0x7f) ==  1.0f);

    for (int n = -0x80; n < 0x7f; ++n)
    {
        CAPTURE(n);
        CHECK(cv1(n) < cv1(n + 1));
    }

    auto cv2 = lol::audio::sample::convert<uint8_t, float>;
    CHECK(cv2(0x00) == -1.0f);
    CHECK(cv2(0xff) ==  1.0f);

    for (int n = 0x00; n < 0xff; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(n) < cv2(n + 1));
    }
}

TEST_CASE("sample conversion: float → int16|uint16")
{
    auto cv1 = lol::audio::sample::convert<float, int16_t>;
    CHECK(cv1(-1.5f) == -0x8000);
    CHECK(cv1(-1.0f) == -0x8000);
    CHECK(cv1(-0.5f) == -0x4000);
    CHECK(cv1( 0.0f) ==  0x0000);
    CHECK(cv1( 0.5f) ==  0x4000);
    CHECK(cv1( 1.0f) ==  0x7fff);
    CHECK(cv1( 1.5f) ==  0x7fff);

    auto cv2 = lol::audio::sample::convert<float, uint16_t>;
    CHECK(cv2(-1.5f) == 0x0000);
    CHECK(cv2(-1.0f) == 0x0000);
    CHECK(cv2(-0.5f) == 0x4000);
    CHECK(cv2( 0.0f) == 0x8000);
    CHECK(cv2( 0.5f) == 0xc000);
    CHECK(cv2( 1.0f) == 0xffff);
    CHECK(cv2( 1.5f) == 0xffff);
}

TEST_CASE("sample conversion: int16|uint16 → float")
{
    auto cv1 = lol::audio::sample::convert<int16_t, float>;
    CHECK(cv1(-0x8000) == -1.0f);
    CHECK(cv1( 0x7fff) ==  1.0f);
    for (int n = -0x8000; n < 0x7fff; ++n)
    {
        CAPTURE(n);
        CHECK(cv1(n) < cv1(n + 1));
    }

    auto cv2 = lol::audio::sample::convert<uint16_t, float>;
    CHECK(cv2(0x0000) == -1.0f);
    CHECK(cv2(0xffff) ==  1.0f);
    for (int n = 0x0000; n < 0xffff; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(n) < cv2(n + 1));
    }
}

TEST_CASE("sample conversion: int8 ←→ uint8")
{
    auto cv1 = lol::audio::sample::convert<int8_t, uint8_t>;
    for (int n = -0x80; n <= 0x7f; ++n)
    {
        CAPTURE(n);
        CHECK(cv1(n) == n + 0x80);
    }

    auto cv2 = lol::audio::sample::convert<uint8_t, int8_t>;
    for (int n = 0x00; n <= 0xff; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(n) == n - 0x80);
    }
}

TEST_CASE("sample conversion: int16 ←→ uint16")
{
    auto cv1 = lol::audio::sample::convert<int16_t, uint16_t>;
    for (int n = -0x8000; n <= 0x7fff; ++n)
    {
        CAPTURE(n);
        CHECK(cv1(n) == n + 0x8000);
    }

    auto cv2 = lol::audio::sample::convert<uint16_t, int16_t>;
    for (int n = 0x0000; n <= 0xffff; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(n) == n - 0x8000);
    }
}

TEST_CASE("sample conversion: int16|uint16 → int8|uint8")
{
    auto cv1 = lol::audio::sample::convert<int16_t, int8_t>;
    CHECK(cv1(-0x8000) == -0x80);
    CHECK(cv1(-0x4000) == -0x40);
    CHECK(cv1(-0x0080) == -0x01);
    CHECK(cv1(-0x0001) == -0x01);
    CHECK(cv1( 0x0000) ==  0x00);
    CHECK(cv1( 0x00ff) ==  0x00);
    CHECK(cv1( 0x3fff) ==  0x3f);
    CHECK(cv1( 0x7fff) ==  0x7f);

    auto cv2 = lol::audio::sample::convert<uint16_t, int8_t>;
    CHECK(cv2(0x0000) == -0x80);
    CHECK(cv2(0x4000) == -0x40);
    CHECK(cv2(0x7f80) == -0x01);
    CHECK(cv2(0x7fff) == -0x01);
    CHECK(cv2(0x8000) ==  0x00);
    CHECK(cv2(0x80ff) ==  0x00);
    CHECK(cv2(0xbfff) ==  0x3f);
    CHECK(cv2(0xffff) ==  0x7f);

    auto cv3 = lol::audio::sample::convert<int16_t, uint8_t>;
    CHECK(cv3(-0x8000) == 0x00);
    CHECK(cv3(-0x4000) == 0x40);
    CHECK(cv3(-0x0080) == 0x7f);
    CHECK(cv3(-0x0001) == 0x7f);
    CHECK(cv3( 0x0000) == 0x80);
    CHECK(cv3( 0x00ff) == 0x80);
    CHECK(cv3( 0x3fff) == 0xbf);
    CHECK(cv3( 0x7fff) == 0xff);

    auto cv4 = lol::audio::sample::convert<uint16_t, uint8_t>;
    CHECK(cv4(0x0000) == 0x00);
    CHECK(cv4(0x4000) == 0x40);
    CHECK(cv4(0x7f80) == 0x7f);
    CHECK(cv4(0x7fff) == 0x7f);
    CHECK(cv4(0x8000) == 0x80);
    CHECK(cv4(0x80ff) == 0x80);
    CHECK(cv4(0xbfff) == 0xbf);
    CHECK(cv4(0xffff) == 0xff);
}

TEST_CASE("sample conversion: uint8 → int16|uint16")
{
    auto cv1 = lol::audio::sample::convert<uint8_t, int16_t>;
    for (int n = 0x00; n <= 0xff; ++n)
    {
        CAPTURE(n);
        CHECK(cv1(n) == n * 0x101 - 0x8000);
    }

    auto cv2 = lol::audio::sample::convert<uint8_t, uint16_t>;
    for (int n = 0x00; n <= 0xff; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(n) == n * 0x101);
    }
}

TEST_CASE("sample conversion: int8 → float → int8")
{
    auto cv1 = lol::audio::sample::convert<int8_t, float>;
    auto cv2 = lol::audio::sample::convert<float, int8_t>;
    for (int n = -0x80; n <= 0x7f; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(cv1(n)) == n);
    }
}

TEST_CASE("sample conversion: uint8 → float → uint8")
{
    auto cv1 = lol::audio::sample::convert<uint8_t, float>;
    auto cv2 = lol::audio::sample::convert<float, uint8_t>;
    for (int n = 0x00; n <= 0xff; ++n)
    {
        CAPTURE(n);
        CHECK(cv2(cv1(n)) == n);
    }
}
