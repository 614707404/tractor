add_executable(poller_test Poller_test.cpp)
target_compile_features(poller_test PRIVATE cxx_std_17)

target_link_libraries(poller_test PRIVATE tractor_library)


add_executable(Socket_test Socket_test.cpp)
target_link_libraries(Socket_test PRIVATE tractor_library)

add_executable(Acceptor_test Acceptor_test.cpp)
target_link_libraries(Acceptor_test PRIVATE tractor_library)

add_executable(TcpServer_test TcpServer_test.cpp)
target_link_libraries(TcpServer_test PRIVATE tractor_library)