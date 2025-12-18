add_library(std-static STATIC)
target_sources(std-static
  PRIVATE
    Arena.c Arena.h
    Array.hpp
    Check.cpp Check.h
    Chronometry.c Chronometry.h
    CompilerInfo.h
    Defer.hpp
    FixedRingBuffer.hpp
    Hash.c Hash.h
    KhrTwoCall.hpp
    List.hpp
    log.h
    Modules.h
    Optional.hpp
    Path.cpp Path.hpp
    Pool.hpp
    RadixSort.hpp
    Ranges.hpp
    Sanitizer.h
    Slice.hpp
    SliceStlString.hpp
    SliceStlVector.hpp
    SliceUtils.hpp
    Sort.hpp
    Trie.hpp
    TrieStrKey.hpp
    Types.h
    Utils.cpp
    Vector.hpp
    VectorUtils.hpp

    log/log.c log/log.h
)
target_include_directories(std-static
  PUBLIC
    ..
)
set_property(TARGET std-static PROPERTY FOLDER "easimer/std")

add_library(std-context SHARED)
target_sources(std-context
  PRIVATE
    ArenaTls.c
)
target_compile_definitions(std-context
  PRIVATE SN_STD_BUILDING=1
  INTERFACE SN_STD_BUILDING=0
)
target_link_libraries(std-context PRIVATE std-static)
set_property(TARGET std-context PROPERTY FOLDER "easimer/std")

add_library(std INTERFACE)
target_link_libraries(std INTERFACE std-context std-static)
set_property(TARGET std PROPERTY FOLDER "easimer/std")

add_library(std::std ALIAS std)

if(SN_STD_BUILD_TESTS)
  add_executable(std-tests
    tests/entry.cpp
    tests/Arena.cpp
    tests/FixedRingBuffer.cpp
    tests/Optional.cpp
    tests/Slice.cpp
    tests/Sort.cpp
    tests/Vector.cpp
  )
  target_link_libraries(std-tests PRIVATE std::std)
  set_property(TARGET std-tests PROPERTY FOLDER "easimer/std")
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT std-tests)
endif()
