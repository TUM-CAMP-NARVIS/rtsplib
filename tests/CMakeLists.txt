add_executable( test_client ${CMAKE_CURRENT_SOURCE_DIR}/test_client.cpp)
target_link_libraries(test_client PRIVATE
        rtsplib
        CONAN_PKG::spdlog)

add_executable( test_server ${CMAKE_CURRENT_SOURCE_DIR}/test_server.cpp)
target_link_libraries(test_server PRIVATE
        rtsplib
        CONAN_PKG::spdlog)