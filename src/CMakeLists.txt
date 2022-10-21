# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST "${Tractor_SOURCE_DIR}/include/tractor/noncopyable.h"
                "${Tractor_SOURCE_DIR}/include/tractor/Channel.h"
                "${Tractor_SOURCE_DIR}/include/tractor/EventLoop.h"
                "${Tractor_SOURCE_DIR}/include/tractor/Poller.h"
                "${Tractor_SOURCE_DIR}/include/tractor/Socket.h"
                "${Tractor_SOURCE_DIR}/include/tractor/SockAddr.h"
                "${Tractor_SOURCE_DIR}/include/tractor/Acceptor.h"
  
                "${Tractor_SOURCE_DIR}/include/tractor/Timer.h"
                "${Tractor_SOURCE_DIR}/include/tractor/TimerQueue.h"

                "${Tractor_SOURCE_DIR}/include/tractor/TcpConnection.h"
                "${Tractor_SOURCE_DIR}/include/tractor/TcpServer.h"

                "${Tractor_SOURCE_DIR}/include/tractor/Buffer.h"

                "${Tractor_SOURCE_DIR}/include/tractor/ThreadPool.h"

                "${Tractor_SOURCE_DIR}/include/tractor/BasePoller.h"
                "${Tractor_SOURCE_DIR}/include/tractor/EPoller.h"

                "${Tractor_SOURCE_DIR}/include/spdlog"

                "${Tractor_SOURCE_DIR}/include/tractor/HttpParser.h"
                "${Tractor_SOURCE_DIR}/include/tractor/HttpRequest.h"
                "${Tractor_SOURCE_DIR}/include/tractor/HttpServer.h"
                "${Tractor_SOURCE_DIR}/include/tractor/HttpResponse.h"

                "${Tractor_SOURCE_DIR}/include/tractor/Timestamp.h"

                )


# Make an automatic library - will be static or dynamic based on user setting
add_library(tractor_library Channel.cpp
                            EventLoop.cpp
                            Poller.cpp
                            Socket.cpp
                            SockAddr.cpp
                            Acceptor.cpp
                            Timer.cpp
                            TimerQueue.cpp

                            TcpConnection.cpp
                            TcpServer.cpp

                            Buffer.cpp

                            ThreadPool.cpp

                            EPoller.cpp

                            HttpParser.cpp
                            HttpServer.cpp
                            HttpResponse.cpp

                            Timestamp.cpp
                            ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(tractor_library PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(tractor_library PRIVATE Boost::boost Threads::Threads )

# All users of this library will need at least C++11
target_compile_features(tractor_library PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
