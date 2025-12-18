std
===

C++ foundation library.

Noteworthy modules:
- [Arena](src/std/Arena.h)
- [Slice\<T\>](src/std/Slice.hpp)
- [SliceUtils](src/std/SliceUtils.hpp)
- [Vector\<T\>](src/std/Vector.hpp)
- [CHECK](src/std/Check.h)

## Integration

### CMake

```cmake
FetchContent_Declare(
    easimer-std
    GIT_REPOSITORY https://git.easimer.net/easimer/std
    # # Pin to a specific commit:
    # GIT_TAG 55e9f50870e99709e0bf41df4bf5e403ff434ef2
)

FetchContent_MakeAvailable(easimer-std)
```

Then Link against `std::std`.
This will add `/src/` as an include directory and the headers can be included like this:
```
#include "std/Arena.h"
#include <std/Slice.hpp>
```

## License

This library is distributed under the [Mozilla Public License Version 2.0](LICENSE).

This repository contains a [modified version](src/std/log) of [rxi's logger library](https://github.com/rxi/log.c), licensed under the MIT license.

