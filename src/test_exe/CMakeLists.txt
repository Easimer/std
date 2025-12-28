add_library(std-test_exe STATIC)
target_sources(std-test_exe
  PRIVATE
    entry.cpp
)
target_link_libraries(std-test_exe
  PUBLIC
    std-static
    std-context
)
target_compile_definitions(std-test_exe
  PUBLIC
    SN_TEST_EXECUTABLE=1
)
set_property(TARGET std-test_exe PROPERTY FOLDER "easimer/std")
add_library(std::test_exe ALIAS std-test_exe)

