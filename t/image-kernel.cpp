#include <lol/image/kernel>

TEST_CASE("bayer kernel")
{
    auto &k = image::kernel::bayer({4, 4});

}

/*
    CHECK
    // Underflow
    CHECK(lol::audio::sample::sadd<int8_t>(-0x80, -0x80) == -0x80);
    CHECK(lol::audio::sample::sadd<int8_t>(-0x41, -0x41) == -0x80);
    CHECK(lol::audio::sample::sadd<int8_t>(-0x40, -0x41) == -0x80);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<int8_t>(-0x40, -0x40) == -0x80);
    CHECK(lol::audio::sample::sadd<int8_t>(-0x3f, -0x3f) == -0x7e);
    CHECK(lol::audio::sample::sadd<int8_t>(-0x01, -0x01) == -0x02);
    CHECK(lol::audio::sample::sadd<int8_t>(-0x01,  0x00) == -0x01);
    CHECK(lol::audio::sample::sadd<int8_t>( 0x00,  0x00) ==  0x00);
    CHECK(lol::audio::sample::sadd<int8_t>( 0x00,  0x01) ==  0x01);
    CHECK(lol::audio::sample::sadd<int8_t>( 0x01,  0x01) ==  0x02);
    CHECK(lol::audio::sample::sadd<int8_t>( 0x3f,  0x3f) ==  0x7e);
    CHECK(lol::audio::sample::sadd<int8_t>( 0x3f,  0x40) ==  0x7f);

    // Overflow
    CHECK(lol::audio::sample::sadd<int8_t>( 0x40,  0x40) ==  0x7f);
    CHECK(lol::audio::sample::sadd<int8_t>( 0x7f,  0x7f) ==  0x7f);
}

TEST_CASE("sample saturated add: uint8_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<uint8_t>(0x00, 0x00) == 0x00);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x3f, 0x3f) == 0x00);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<uint8_t>(0x40, 0x40) == 0x00);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x41, 0x41) == 0x02);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x7f, 0x7f) == 0x7e);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x7f, 0x80) == 0x7f);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x80, 0x80) == 0x80);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x80, 0x81) == 0x81);
    CHECK(lol::audio::sample::sadd<uint8_t>(0x81, 0x81) == 0x82);
    CHECK(lol::audio::sample::sadd<uint8_t>(0xbf, 0xbf) == 0xfe);
    CHECK(lol::audio::sample::sadd<uint8_t>(0xbf, 0xc0) == 0xff);

    // Overflow
    CHECK(lol::audio::sample::sadd<uint8_t>(0xc0, 0xc0) == 0xff);
    CHECK(lol::audio::sample::sadd<uint8_t>(0xff, 0xff) == 0xff);
}

TEST_CASE("sample saturated add: int16_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<int16_t>(-0x8000, -0x8000) == -0x8000);
    CHECK(lol::audio::sample::sadd<int16_t>(-0x4001, -0x4001) == -0x8000);
    CHECK(lol::audio::sample::sadd<int16_t>(-0x4000, -0x4001) == -0x8000);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<int16_t>(-0x4000, -0x4000) == -0x8000);
    CHECK(lol::audio::sample::sadd<int16_t>(-0x3fff, -0x3fff) == -0x7ffe);
    CHECK(lol::audio::sample::sadd<int16_t>(-0x0001, -0x0001) == -0x0002);
    CHECK(lol::audio::sample::sadd<int16_t>(-0x0001,  0x0000) == -0x0001);
    CHECK(lol::audio::sample::sadd<int16_t>( 0x0000,  0x0000) ==  0x0000);
    CHECK(lol::audio::sample::sadd<int16_t>( 0x0000,  0x0001) ==  0x0001);
    CHECK(lol::audio::sample::sadd<int16_t>( 0x0001,  0x0001) ==  0x0002);
    CHECK(lol::audio::sample::sadd<int16_t>( 0x3fff,  0x3fff) ==  0x7ffe);
    CHECK(lol::audio::sample::sadd<int16_t>( 0x3fff,  0x4000) ==  0x7fff);

    // Overflow
    CHECK(lol::audio::sample::sadd<int16_t>( 0x4000,  0x4000) ==  0x7fff);
    CHECK(lol::audio::sample::sadd<int16_t>( 0x7fff,  0x7fff) ==  0x7fff);
}

TEST_CASE("sample saturated add: uint16_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<uint16_t>(0x0000, 0x0000) == 0x0000);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x3fff, 0x3fff) == 0x0000);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<uint16_t>(0x4000, 0x4000) == 0x0000);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x4001, 0x4001) == 0x0002);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x7fff, 0x7fff) == 0x7ffe);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x7fff, 0x8000) == 0x7fff);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x8000, 0x8000) == 0x8000);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x8000, 0x8001) == 0x8001);
    CHECK(lol::audio::sample::sadd<uint16_t>(0x8001, 0x8001) == 0x8002);
    CHECK(lol::audio::sample::sadd<uint16_t>(0xbfff, 0xbfff) == 0xfffe);
    CHECK(lol::audio::sample::sadd<uint16_t>(0xbfff, 0xc000) == 0xffff);

    // Overflow
    CHECK(lol::audio::sample::sadd<uint16_t>(0xc000, 0xc000) == 0xffff);
    CHECK(lol::audio::sample::sadd<uint16_t>(0xffff, 0xffff) == 0xffff);
}

TEST_CASE("sample saturated add: uint32_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<uint32_t>(0x00000000, 0x00000000) == 0x00000000);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x3fffffff, 0x3fffffff) == 0x00000000);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<uint32_t>(0x40000000, 0x40000000) == 0x00000000);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x40000001, 0x40000001) == 0x00000002);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x7fffffff, 0x7fffffff) == 0x7ffffffe);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x7fffffff, 0x80000000) == 0x7fffffff);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x80000000, 0x80000000) == 0x80000000);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x80000000, 0x80000001) == 0x80000001);
    CHECK(lol::audio::sample::sadd<uint32_t>(0x80000001, 0x80000001) == 0x80000002);
    CHECK(lol::audio::sample::sadd<uint32_t>(0xbfffffff, 0xbfffffff) == 0xfffffffe);
    CHECK(lol::audio::sample::sadd<uint32_t>(0xbfffffff, 0xc0000000) == 0xffffffff);

    // Overflow
    CHECK(lol::audio::sample::sadd<uint32_t>(0xc0000000, 0xc0000000) == 0xffffffff);
    CHECK(lol::audio::sample::sadd<uint32_t>(0xffffffff, 0xffffffff) == 0xffffffff);
}

TEST_CASE("sample saturated add: int32_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<int32_t>(-0x80000000, -0x80000000) == -0x80000000);
    CHECK(lol::audio::sample::sadd<int32_t>(-0x40000001, -0x40000001) == -0x80000000);
    CHECK(lol::audio::sample::sadd<int32_t>(-0x40000000, -0x40000001) == -0x80000000);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<int32_t>(-0x40000000, -0x40000000) == -0x80000000);
    CHECK(lol::audio::sample::sadd<int32_t>(-0x3fffffff, -0x3fffffff) == -0x7ffffffe);
    CHECK(lol::audio::sample::sadd<int32_t>(-0x00000001, -0x00000001) == -0x00000002);
    CHECK(lol::audio::sample::sadd<int32_t>(-0x00000001,  0x00000000) == -0x00000001);
    CHECK(lol::audio::sample::sadd<int32_t>( 0x00000000,  0x00000000) ==  0x00000000);
    CHECK(lol::audio::sample::sadd<int32_t>( 0x00000000,  0x00000001) ==  0x00000001);
    CHECK(lol::audio::sample::sadd<int32_t>( 0x00000001,  0x00000001) ==  0x00000002);
    CHECK(lol::audio::sample::sadd<int32_t>( 0x3fffffff,  0x3fffffff) ==  0x7ffffffe);
    CHECK(lol::audio::sample::sadd<int32_t>( 0x3fffffff,  0x40000000) ==  0x7fffffff);

    // Overflow
    CHECK(lol::audio::sample::sadd<int32_t>( 0x40000000,  0x40000000) ==  0x7fffffff);
    CHECK(lol::audio::sample::sadd<int32_t>( 0x7fffffff,  0x7fffffff) ==  0x7fffffff);
}

TEST_CASE("sample saturated add: uint64_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<uint64_t>(0x0000000000000000, 0x0000000000000000) == 0x0000000000000000);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x3fffffffffffffff, 0x3fffffffffffffff) == 0x0000000000000000);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<uint64_t>(0x4000000000000000, 0x4000000000000000) == 0x0000000000000000);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x4000000000000001, 0x4000000000000001) == 0x0000000000000002);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x7fffffffffffffff, 0x7fffffffffffffff) == 0x7ffffffffffffffe);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x7fffffffffffffff, 0x8000000000000000) == 0x7fffffffffffffff);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x8000000000000000, 0x8000000000000000) == 0x8000000000000000);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x8000000000000000, 0x8000000000000001) == 0x8000000000000001);
    CHECK(lol::audio::sample::sadd<uint64_t>(0x8000000000000001, 0x8000000000000001) == 0x8000000000000002);
    CHECK(lol::audio::sample::sadd<uint64_t>(0xbfffffffffffffff, 0xbfffffffffffffff) == 0xfffffffffffffffe);
    CHECK(lol::audio::sample::sadd<uint64_t>(0xbfffffffffffffff, 0xc000000000000000) == 0xffffffffffffffff);

    // Overflow
    CHECK(lol::audio::sample::sadd<uint64_t>(0xc000000000000000, 0xc000000000000000) == 0xffffffffffffffff);
    CHECK(lol::audio::sample::sadd<uint64_t>(0xffffffffffffffff, 0xffffffffffffffff) == 0xffffffffffffffff);
}

TEST_CASE("sample saturated add: int64_t")
{
    // Underflow
    CHECK(lol::audio::sample::sadd<int64_t>(-0x8000000000000000, -0x8000000000000000) == -0x8000000000000000);
    CHECK(lol::audio::sample::sadd<int64_t>(-0x4000000000000001, -0x4000000000000001) == -0x8000000000000000);
    CHECK(lol::audio::sample::sadd<int64_t>(-0x4000000000000000, -0x4000000000000001) == -0x8000000000000000);

    // Standard operating mode
    CHECK(lol::audio::sample::sadd<int64_t>(-0x4000000000000000, -0x4000000000000000) == -0x8000000000000000);
    CHECK(lol::audio::sample::sadd<int64_t>(-0x3fffffffffffffff, -0x3fffffffffffffff) == -0x7ffffffffffffffe);
    CHECK(lol::audio::sample::sadd<int64_t>(-0x0000000000000001, -0x0000000000000001) == -0x0000000000000002);
    CHECK(lol::audio::sample::sadd<int64_t>(-0x0000000000000001,  0x0000000000000000) == -0x0000000000000001);
    CHECK(lol::audio::sample::sadd<int64_t>( 0x0000000000000000,  0x0000000000000000) ==  0x0000000000000000);
    CHECK(lol::audio::sample::sadd<int64_t>( 0x0000000000000000,  0x0000000000000001) ==  0x0000000000000001);
    CHECK(lol::audio::sample::sadd<int64_t>( 0x0000000000000001,  0x0000000000000001) ==  0x0000000000000002);
    CHECK(lol::audio::sample::sadd<int64_t>( 0x3fffffffffffffff,  0x3fffffffffffffff) ==  0x7ffffffffffffffe);
    CHECK(lol::audio::sample::sadd<int64_t>( 0x3fffffffffffffff,  0x4000000000000000) ==  0x7fffffffffffffff);

    // Overflow
    CHECK(lol::audio::sample::sadd<int64_t>( 0x4000000000000000,  0x4000000000000000) ==  0x7fffffffffffffff);
    CHECK(lol::audio::sample::sadd<int64_t>( 0x7fffffffffffffff,  0x7fffffffffffffff) ==  0x7fffffffffffffff);
}
*/
