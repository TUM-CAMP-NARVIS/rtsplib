
set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_connect_rtspclients.cpp
        )

add_executable(rtsplib_test ${TEST_SOURCES})
set_target_properties(rtsplib_test PROPERTIES CXX_STANDARD 11)

target_link_libraries(rtsplib_test PUBLIC
        rtsplib
        CONAN_PKG::spdlog
        )

# conan weirdness ...
if (TARGET CONAN_PKG::GTest)
  target_link_libraries(rtsplib_test PUBLIC CONAN_PKG::GTest)
else()
  target_link_libraries(rtsplib_test PUBLIC CONAN_PKG::gtest)
endif()

target_include_directories(rtsplib_test
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/src
        ${PROJECT_SOURCE_DIR}/src
        )
