# Defines these CMake vars:
# SN_STD_ARCHITECTURE: target CPU architecture; one of: [x64, a64, wasm32]
# SN_STD_PLATFORM_ID: target platform; one of: [win-ARCH, linux-ARCH, android-ARCH, wasm32]
#
# On Android only:
# SN_STD_ANDROID_ABI: target CPU architecture, as it's called by the Android toolchain; one of: [arm64-v8a, x86_64]

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
    set(SN_STD_ARCHITECTURE "a64")
    set(SN_STD_ARCH 2)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(SN_STD_ARCHITECTURE "x64")
    set(SN_STD_ARCH 1)
  else()
    message(FATAL_ERROR "Unsupported target processor ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  set(SN_STD_SYSTEM 1)
  set(SN_STD_PLATFORM_ID "win-${SN_STD_ARCHITECTURE}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
    set(SN_STD_ARCHITECTURE "a64")
    set(SN_STD_ARCH 2)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(SN_STD_ARCHITECTURE "x64")
    set(SN_STD_ARCH 1)
  else()
    message(FATAL_ERROR "Unsupported target processor ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  set(SN_STD_SYSTEM 2)
  set(SN_STD_PLATFORM_ID "uwp-${SN_STD_ARCHITECTURE}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(SN_STD_ARCHITECTURE "a64")
    set(SN_STD_ARCH 2)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(SN_STD_ARCHITECTURE "x64")
    set(SN_STD_ARCH 1)
  else()
    message(FATAL_ERROR "Unsupported target processor ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  set(SN_STD_SYSTEM 3)
  set(SN_STD_PLATFORM_ID "linux-${SN_STD_ARCHITECTURE}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(SN_STD_ARCHITECTURE "a64")
    set(SN_STD_ANDROID_ABI "arm64-v8a")
    set(SN_STD_ARCH 2)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(SN_STD_ARCHITECTURE "x64")
    set(SN_STD_ANDROID_ABI "x86_64")
    set(SN_STD_ARCH 1)
  else()
    message(FATAL_ERROR "Unsupported target processor ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  set(SN_STD_SYSTEM 4)
  set(SN_STD_PLATFORM_ID "android-${SN_STD_ARCHITECTURE}")
elseif(EMSCRIPTEN)
  set(SN_STD_ARCHITECTURE "wasm32")
  set(SN_STD_PLATFORM_ID "wasm32")
  set(SN_STD_SYSTEM 5)
  set(SN_STD_ARCH 3)
else()
  message(FATAL_ERROR "Unsupported target system ${CMAKE_SYSTEM_NAME}")
endif()

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
    SignalTree.hpp
    Slice.hpp
    SliceStlString.hpp
    SliceStlVector.hpp
    SliceUtils.hpp
    Sort.hpp
    Strings.hpp
    Trie.hpp
    TrieStrKey.hpp
    Types.h
    Utils.cpp
    Uuid.hpp
    Vector.hpp
    VectorUtils.hpp
    VU128.c VU128.h
    WorkerPool.cpp WorkerPool.hpp

    json/Value.hpp
    json/Parser.cpp
    json/Parser.hpp
    json/Utils.hpp

    log/log.c log/log.h
)
target_include_directories(std-static
  PUBLIC
    ..
)
target_wall_werror_SN(std-static)
set_property(TARGET std-static PROPERTY FOLDER "easimer/std")

target_compile_definitions(std-static
  PUBLIC
    SN_STD_ARCH=${SN_STD_ARCH}
    SN_STD_SYSTEM=${SN_STD_SYSTEM}
)

if(MSVC)
  target_link_options(std-static PUBLIC "/alternatename:checkFail=checkFailDefault")
endif()

add_library(std-os STATIC)
target_sources(std-os
  PRIVATE
    os/Thread.cpp os/Thread.hpp
    os/Sync.c os/Sync.h
)
target_link_libraries(std-os PUBLIC std-static)
target_wall_werror_SN(std-os)
set_property(TARGET std-os PROPERTY FOLDER "easimer/std")
add_library(std::os ALIAS std-os)

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

add_library(std-gltf STATIC)
target_sources(std-gltf
  PRIVATE
    gltf/AccessorIterable.hpp
    gltf/Descriptor.hpp
    gltf/IntegerConversion.hpp
    gltf/Loader.cpp gltf/Loader.hpp
)
target_link_libraries(std-gltf PUBLIC std::std)
add_library(std::gltf ALIAS std-gltf)
set_property(TARGET std-gltf PROPERTY FOLDER "easimer/std")

if(SN_STD_BUILD_TESTS)
  add_executable(std-tests
    tests/entry.cpp
    tests/Arena.cpp
    tests/Array.cpp
    tests/Chronometry.cpp
    tests/CommandCodec.cpp
    tests/FixedRingBuffer.cpp
    tests/Endian.cpp
    tests/Json.cpp
    tests/Log.cpp
    tests/Optional.cpp
    tests/Path.cpp
    tests/Pool.cpp
    tests/Result.cpp
    tests/SegmentArray.cpp
    tests/Slice.cpp
    tests/Sort.cpp
    tests/Strings.cpp
    tests/Thread.cpp
    tests/Utils.cpp
    tests/Uuid.cpp
    tests/Vector.cpp
    tests/VU128.cpp

    gltf/Tests.cpp
    Box_glb.c
    BoxTextured_glb.c

    tests/CompileC.c
  )
  target_link_libraries(std-tests PRIVATE std::test_exe std::os std::gltf)
  target_wall_werror_SN(std-tests)
  set_property(TARGET std-tests PROPERTY FOLDER "easimer/std")
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT std-tests)
  add_test(
    NAME std-tests
    COMMAND $<TARGET_FILE:std-tests>
  )
  add_custom_command(
    OUTPUT Box_glb.c
    COMMAND embed Box_glb ${CMAKE_SOURCE_DIR}/assets/Box.glb
    DEPENDS ${CMAKE_SOURCE_DIR}/assets/Box.glb
  )
  add_custom_command(
    OUTPUT BoxTextured_glb.c
    COMMAND embed BoxTextured_glb ${CMAKE_SOURCE_DIR}/assets/BoxTextured.glb
    DEPENDS ${CMAKE_SOURCE_DIR}/assets/BoxTextured.glb
  )
endif()

if(SN_STD_BUILD_JSON_TEST_SUITE_RUNNER)
  add_executable(std-json_test_suite json_test_suite/entry.cpp)
  target_link_libraries(std-json_test_suite PRIVATE std-static std-context)
  target_wall_werror_SN(std-json_test_suite)
  set_property(TARGET std-json_test_suite PROPERTY FOLDER "easimer/std")
endif()