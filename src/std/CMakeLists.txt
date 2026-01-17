add_library(std-static STATIC)
target_sources(std-static
  PRIVATE
    Arena.c Arena.h
    Array.hpp
    Check.cpp Check.h
    Chronometry.c Chronometry.h
    CommandDecoder.hpp
    CommandEncoder.hpp
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
    Result.hpp
    Sanitizer.h
    SegmentArray.hpp
    Slice.hpp
    SliceStlString.hpp
    SliceStlVector.hpp
    SliceUtils.hpp
    Sort.hpp
    Trie.hpp
    TrieStrKey.hpp
    Types.h
    Utils.cpp
    Uuid.hpp
    Vector.hpp
    VectorUtils.hpp

    log/log.c log/log.h
)
target_include_directories(std-static
  PUBLIC
    ..
)
target_wall_werror_SN(std-static)
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
target_wall_werror_SN(std-context)
set_property(TARGET std-context PROPERTY FOLDER "easimer/std")

if(MSVC)
  target_compile_options(std-context PRIVATE /GS-)
  target_link_options(std-context PRIVATE /incremental:no /opt:icf /opt:ref)
endif()

add_library(std INTERFACE)
target_link_libraries(std INTERFACE std-context std-static)
set_property(TARGET std PROPERTY FOLDER "easimer/std")

add_library(std::std ALIAS std)

if(SN_STD_BUILD_TESTS)
  add_executable(std-tests
    tests/entry.cpp
    tests/Arena.cpp
    tests/CommandCodec.cpp
    tests/FixedRingBuffer.cpp
    tests/Optional.cpp
    tests/Pool.cpp
    tests/Result.cpp
    tests/SegmentArray.cpp
    tests/Slice.cpp
    tests/Sort.cpp
    tests/Vector.cpp
    tests/Utils.cpp
    tests/Uuid.cpp
  )
  target_link_libraries(std-tests PRIVATE std::test_exe)
  target_wall_werror_SN(std-tests)
  set_property(TARGET std-tests PROPERTY FOLDER "easimer/std")
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT std-tests)
  add_test(
    NAME std-tests
    COMMAND $<TARGET_FILE:std-tests>
  )
endif()
