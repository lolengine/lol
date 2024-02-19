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

TEST_CASE("sample conversion from 8-bit to float")
{
    auto cv1 = lol::audio::sample::convert<int8_t, float>;
    CHECK(cv1(-0x80) == -1.0f);
    CHECK(cv1( 0x7f) ==  1.0f);

    auto cv2 = lol::audio::sample::convert<uint8_t, float>;
    CHECK(cv2(0x00) == -1.0f);
    CHECK(cv2(0xff) ==  1.0f);
}

TEST_CASE("sample conversion from float to 16-bit")
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

TEST_CASE("sample conversion from 16-bit to float")
{
    auto cv1 = lol::audio::sample::convert<int16_t, float>;
    CHECK(cv1(-0x8000) == -1.0f);
    CHECK(cv1( 0x7fff) ==  1.0f);

    auto cv2 = lol::audio::sample::convert<uint16_t, float>;
    CHECK(cv2(0x0000) == -1.0f);
    CHECK(cv2(0xffff) ==  1.0f);
}

TEST_CASE("sample conversion between signed and unsigned 8-bit")
{
    auto cv1 = lol::audio::sample::convert<int8_t, uint8_t>;
    CHECK(cv1(-0x80) == 0x00);
    CHECK(cv1(-0x40) == 0x40);
    CHECK(cv1(-0x01) == 0x7f);
    CHECK(cv1( 0x00) == 0x80);
    CHECK(cv1( 0x3f) == 0xbf);
    CHECK(cv1( 0x7f) == 0xff);

    auto cv2 = lol::audio::sample::convert<uint8_t, int8_t>;
    CHECK(cv2(0x00) == -0x80);
    CHECK(cv2(0x40) == -0x40);
    CHECK(cv2(0x7f) == -0x01);
    CHECK(cv2(0x80) ==  0x00);
    CHECK(cv2(0xbf) ==  0x3f);
    CHECK(cv2(0xff) ==  0x7f);
}

TEST_CASE("sample conversion from 16-bit to 8-bit")
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
}

TEST_CASE("round-trip conversion from 8-bit to 8-bit")
{
    auto cv1 = lol::audio::sample::convert<int8_t, float>;
    auto cv2 = lol::audio::sample::convert<float, int8_t>;
    CHECK(cv2(cv1(-0x80)) == -0x80);
    CHECK(cv2(cv1(-0x7f)) == -0x7f);
    CHECK(cv2(cv1(-0x40)) == -0x40);
    CHECK(cv2(cv1(-0x20)) == -0x20);
    CHECK(cv2(cv1(-0x02)) == -0x02);
    CHECK(cv2(cv1(-0x01)) == -0x01);
    CHECK(cv2(cv1( 0x00)) ==  0x00);
    CHECK(cv2(cv1( 0x01)) ==  0x01);
    CHECK(cv2(cv1( 0x02)) ==  0x02);
    CHECK(cv2(cv1( 0x20)) ==  0x20);
    CHECK(cv2(cv1( 0x40)) ==  0x40);
    CHECK(cv2(cv1( 0x7f)) ==  0x7f);
}
