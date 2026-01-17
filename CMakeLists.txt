cmake_minimum_required(VERSION 3.22.1)
project(std)

set(CMAKE_CONFIGURATION_TYPES "Debug;DebugSan;RelWithDebInfo;Release")

set(CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(SN_STD_BUILD_TESTS "Build tests" OFF)
if(SN_STD_BUILD_TESTS)
  enable_testing()
endif()

# Setup sanitizer config
if(MSVC)
  # NOTE: MSVC defines __SANTIZE_ADDRESS__ since 16.9 if /fsanitize=address is
  # passed
  set(SN_SANITIZER_CFLAGS "/MTd /Od /Zi /RTC1 /fsanitize=address /D_DISABLE_VECTOR_ANNOTATION=1 /D_DISABLE_STRING_ANNOTATION=1 /D__SANITIZE_ADDRESS__=1")
  set(SN_SANITIZER_LDFLAGS "/incremental:no")
else()
  set(SN_SANITIZER_CFLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined")
  set(SN_SANITIZER_LDFLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined")
endif()

set(CMAKE_C_FLAGS_DEBUGSAN "${CMAKE_C_FLAGS_DEBUG} ${SN_SANITIZER_CFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUGSAN "${CMAKE_CXX_FLAGS_DEBUG} ${SN_SANITIZER_CFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSAN "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${SN_SANITIZER_LDFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUGSAN "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SN_SANITIZER_LDFLAGS}")

function(target_wall_werror_SN TGT)
  if(MSVC)
    target_compile_options(${TGT}
      PRIVATE
        /W3 /WX
    )
  else()
    target_compile_options(${TGT}
      PRIVATE
        -Wall -Werror
    )
  endif()
endfunction()

option(SN_STD_EMSCRIPTEN_PTHREAD "Build with -pthread" ON)
if(EMSCRIPTEN AND SN_STD_EMSCRIPTEN_PTHREAD)
  add_compile_options(-pthread)
  add_link_options(-pthread)
endif()

add_subdirectory(src)
