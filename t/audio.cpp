#include <lol/lib/doctest_main>
#include <lol/audio/stream>

TEST_CASE("sample conversion between floating point types")
{
    auto cv1 = lol::audio::sample::convert<float, float>;
    CHECK(cv1(-1.0f) == -1.0f);
    CHECK(cv1( 0.0f) ==  0.0f);
    CHECK(cv1( 1.0f) ==  1.0f);

    auto cv2 = lol::audio::sample::convert<float, double>;
    CHECK(cv2(-1.0f) == -1.0);
    CHECK(cv2( 0.0f) ==  0.0);
    CHECK(cv2( 1.0f) ==  1.0);

    auto cv3 = lol::audio::sample::convert<double, float>;
    CHECK(cv3(-1.0) == -1.0f);
    CHECK(cv3( 0.0) ==  0.0f);
    CHECK(cv3( 1.0) ==  1.0f);

    auto cv4 = lol::audio::sample::convert<double, double>;
    CHECK(cv4(-1.0) == -1.0);
    CHECK(cv4( 0.0) ==  0.0);
    CHECK(cv4( 1.0) ==  1.0);
}

TEST_CASE("sample conversion from float to 8-bit")
{
    auto cv1 = lol::audio::sample::convert<float, int8_t>;
    CHECK(cv1(-1.5f) == -128);
    CHECK(cv1(-1.0f) == -128);
    CHECK(cv1(-0.5f) == -64);
    CHECK(cv1( 0.0f) ==  0);
    CHECK(cv1( 0.5f) ==  64);
    CHECK(cv1( 1.0f) ==  127);
    CHECK(cv1( 1.5f) ==  127);

    auto cv2 = lol::audio::sample::convert<float, uint8_t>;
    CHECK(cv2(-1.5f) == 0);
    CHECK(cv2(-1.0f) == 0);
    CHECK(cv2(-0.5f) == 64);
    CHECK(cv2( 0.0f) == 128);
    CHECK(cv2( 0.5f) == 192);
    CHECK(cv2( 1.0f) == 255);
    CHECK(cv2( 1.5f) == 255);
}

TEST_CASE("sample conversion from 8-bit to float")
{
    auto cv1 = lol::audio::sample::convert<int8_t, float>;
    CHECK(cv1(-128) == -1.0f);
    CHECK(cv1( 127) ==  1.0f);

    auto cv2 = lol::audio::sample::convert<uint8_t, float>;
    CHECK(cv2(  0) == -1.0f);
    CHECK(cv2(255) ==  1.0f);
}

TEST_CASE("sample conversion from float to 16-bit")
{
    auto cv1 = lol::audio::sample::convert<float, int16_t>;
    CHECK(cv1(-1.5f) == -32768);
    CHECK(cv1(-1.0f) == -32768);
    CHECK(cv1(-0.5f) == -16384);
    CHECK(cv1( 0.0f) ==  0);
    CHECK(cv1( 0.5f) ==  16384);
    CHECK(cv1( 1.0f) ==  32767);
    CHECK(cv1( 1.5f) ==  32767);

    auto cv2 = lol::audio::sample::convert<float, uint16_t>;
    CHECK(cv2(-1.5f) == 0);
    CHECK(cv2(-1.0f) == 0);
    CHECK(cv2(-0.5f) == 16384);
    CHECK(cv2( 0.0f) == 32768);
    CHECK(cv2( 0.5f) == 49152);
    CHECK(cv2( 1.0f) == 65535);
    CHECK(cv2( 1.5f) == 65535);
}

TEST_CASE("sample conversion from 16-bit to float")
{
    auto cv1 = lol::audio::sample::convert<int16_t, float>;
    CHECK(cv1(-32768) == -1.0f);
    CHECK(cv1( 32767) ==  1.0f);

    auto cv2 = lol::audio::sample::convert<uint16_t, float>;
    CHECK(cv2(    0) == -1.0f);
    CHECK(cv2(65535) ==  1.0f);
}

TEST_CASE("round-trip conversion from 8-bit to 8-bit")
{
    auto cv1 = lol::audio::sample::convert<int8_t, float>;
    auto cv2 = lol::audio::sample::convert<float, int8_t>;
    CHECK(cv2(cv1(-128)) == -128);
    CHECK(cv2(cv1(-127)) == -127);
    CHECK(cv2(cv1( -64)) ==  -64);
    CHECK(cv2(cv1( -32)) ==  -32);
    CHECK(cv2(cv1(  -2)) ==   -2);
    CHECK(cv2(cv1(  -1)) ==   -1);
    CHECK(cv2(cv1(   0)) ==    0);
    CHECK(cv2(cv1(   1)) ==    1);
    CHECK(cv2(cv1(   2)) ==    2);
    CHECK(cv2(cv1(  32)) ==   32);
    CHECK(cv2(cv1(  64)) ==   64);
    CHECK(cv2(cv1( 127)) ==  127);
}
