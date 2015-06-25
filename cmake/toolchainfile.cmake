#set(CMAKE_SYSTEM_NAME Linux)
#set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/usr/bin/x86_64-unknown-linux-gnu-gcc)
#set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/usr/bin/x86_64-unknown-linux-gnu-g++)

set(EXTRA_FLAGS "-Wall -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${EXTRA_FLAGS}" CACHE STRING "Buildroot CXXFLAGS" FORCE)
