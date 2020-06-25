
# About

The header-only part of the Lol Engine framework.

## Sytem headers

| header | description | examples |
|--------|-------------|----------|
| `<lol/cli>`       | command-line parsing (imported from [cliutils/cli11](https://github.com/CLIUtils/CLI11)) | |
| `<lol/dialogs>`   | portable file dialogs (imported from [samhocevar/portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs)) | |
| `<lol/file>`      | file reading utilities | |
| `<lol/msg>`       | simple message logging | |
| `<lol/thread>`    | threading and timers | |
| `<lol/unit_test>` | unit test framework | |
| `<lol/utils>`     | various utilities: environment variables, string formatting, std::map and std::vector extensions… | |

## Containers

| header | description | examples |
|--------|-------------|----------|
| `<lol/narray>` | n-dimensional dynamic array containers | ● `lol::narray<T, dimensions>`<br>● `lol::array2d<T>` and `lol::array3d<T>`<br>● `lol::narray_view<T, dimensions>`<br>● `lol::array2d_view<T>` and `lol::array3d_view<T>` |

## Text utilities

| header | description | examples |
|--------|-------------|----------|
| `<lol/pegtl>`             | the PEGTL parsing library (imported from [taocpp/pegtl](https://github.com/taocpp/PEGTL)) | |
| `<lol/algo/suffix_array>` | suffix array library (imported from [storm-ptr/step](https://github.com/storm-ptr/step)) | |

## Graphics

| header | description | examples |
|--------|-------------|----------|
| `<lol/color>` | colorspace conversions (RGB, sRGB, HSV…) | |
| `<lol/image>` | image loading, saving, and processing | |

## Math

| header | description | examples |
|--------|-------------|----------|
| `<lol/bigint>`    | big integer calculations | |
| `<lol/half>`      | half-precision (16-bit) floating point numbers | |
| `<lol/math>`      | math constants, functions, random number generators, polynomials | |
| `<lol/noise>`     | Perlin and simplex noise | |
| `<lol/real>`      | arbitrary precision floating point numbers | |
| `<lol/transform>` | quaternion, matrix and SQT transformation utilities | |
| `<lol/vector>`    | GLSL-compatible vector classes | |

