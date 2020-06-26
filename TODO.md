
## `lol` headers to keep

    src/lol/base/types.h (vec_t shortcuts eg. vec2, ivec2, u8vec3 etc.)
    src/lol/math/constants.h


## headers we could clean up and keep

    src/lol/algorithm/aabb_tree.h
    src/lol/algorithm/portal.h
    src/lol/base/avl_tree.h
    src/lol/math/geometry.h
    src/lol/math/interp.h (but what is it?)


## headers to keep in the engine

    src/lol/algorithm/portal.h
    src/lol/audio/*
    src/lol/base/log.h
    src/lol/debug/*
    src/lol/engine.h
    src/lol/engine/*
    src/lol/engine-internal.h
    src/lol/extras.h
    src/lol/gpu/*
    src/lol/image/image.h
    src/lol/image/movie.h
    src/lol/image/resource.h
    src/lol/legacy.h
    src/lol/lua.h
    src/lol/net/http.h
    src/lol/public.h
    src/lol/sys/init.h


## headers to remove one day

    src/lol/base/enum.h (can’t see the point)

## Interesting libraries:

  - magic enum https://github.com/Neargye/magic_enum
  - event dispatcher https://github.com/wqking/eventpp
  - random https://github.com/effolkronium/random
  - neither (handle exceptions with the type system) https://github.com/LoopPerfect/neither
  - lock-free queue https://github.com/rigtorp/SPSCQueue
  - fixed point math https://github.com/MikeLankamp/fpm

## Other libraries

  - coroutines (uses macros) https://github.com/jamboree/co2
  - SIMD https://awesomeopensource.com/project/ospray/tsimd
  - deep neural networks https://github.com/yixuan/MiniDNN
  - emoji https://github.com/99xt/emojicpp

