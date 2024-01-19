
# About

The header-only part of the Lol Engine framework.

## Containers

| header | description | examples |
|--------|-------------|----------|
| `<lol/narray>` | n-dimensional dynamic array containers | ● `lol::narray<int, 4>`<br>● `lol::array2d<float>`, `lol::array3d<uint8_t>`<br>● `lol::narray_span<int, 4>`<br>● `lol::span2d<uint8_t>`, `lol::span3d<float>` |

## Math

| header | description | examples |
|--------|-------------|----------|
| `<lol/bigint>`    | big integer calculations | |
| `<lol/half>`      | half-precision (16-bit) floating point numbers | `lol::half x = 2, y = 3;`<br>`x /= y;`<br>`float z = x;` |
| `<lol/math>`      | math constants, functions, random number generators, polynomials | |
| `<lol/noise>`     | Perlin and simplex noise | |
| `<lol/real>`      | arbitrary precision floating point numbers | |
| `<lol/transform>` | quaternion, matrix and SQT transformation utilities | |
| `<lol/vector>`    | GLSL-compatible vector classes | ● `lol::vec2`, `lol::vec3`, `lol::vec4`<br>● `lol::ivec2`, `lol::ivec3`, `lol::ivec4`<br>● also: `lol::u8vec3`, `lol::f16vec4`, `lol::vec7`, `lol::ivec11`…<br>● swizzling: `v1 = v2.xyy;`, `v2 += v3.bgr;a`<br>● functions: `dot()`, `length()`, `min()`, `clamp()`… |

## Sytem

| header | description | examples |
|--------|-------------|----------|
| `<lol/cli>`       | command-line parsing (imported from [cliutils/cli11](https://github.com/CLIUtils/CLI11)) | |
| `<lol/dialogs>`   | portable file dialogs (imported from [samhocevar/portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs)) | |
| `<lol/file>`      | file reading utilities | ● `lol::read(filename, data)` for any `std::string` or `std::vector`<br>● `lol::write(filename, data)` |
| `<lol/msg>`       | simple message logging | ● `lol::msg::info("hello\n");`<br>● `lol::msg::debug("%d %d\n", x, y);`<br>● also `lol::msg::error`, `lol::msg::warn` |
| `<lol/thread>`    | threading and timing | ● `lol::thread`<br>● `lol::queue<int, 200>` (thread-safe FIFO queue)<br>● `lol::timer` (high precision timer) |
| `<lol/unit_test>` | unit test framework | |
| `<lol/utils>`     | various utilities: environment variables, std::map and std::vector extensions… | |

## Text utilities

| header | description | examples |
|--------|-------------|----------|
| `<lol/pegtl>`             | the PEGTL parser (imported from [taocpp/pegtl](https://github.com/taocpp/PEGTL)) | |
| `<lol/algo/suffix_array>` | suffix array for fast string searches (imported from [storm-ptr/step](https://github.com/storm-ptr/step)) | |

## Graphics

| header | description | examples |
|--------|-------------|----------|
| `<lol/color>` | colorspace conversions (RGB, sRGB, HSV…) | |
| `<lol/image>` | image loading, saving, and processing | |

