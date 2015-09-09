#/*
# *  Dynamic Data exchange library [libdy]
# *  Copyright (C) 2015 Taeyeon Mori
# *
# *  This program is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation, either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# */

project(libdy++)
set(LIBDYPP_VERSION "0.1.0")
set(LIBDYPP_SOVERSION 0)

set(HEADERS
    callable.h

    dy++conv.h

    dy++impl.h
    dy++.h
)

set(SOURCES
    dy++.cpp
)

add_definitions(-D__LIBDYPP_LIBRARY)
include_directories(..)
add_library(libdy++ SHARED ${HEADERS} ${SOURCES})
target_link_libraries(libdy++ libdy)
set_target_properties(libdy++ PROPERTIES
    OUTPUT_NAME dy++
    VERSION "${LIBDYPP_VERSION}"
    SOVERSION "${LIBDYPP_SOVERSION}"
    PUBLIC_HEADER "${HEADERS}"
)

# Install
install(TARGETS libdy++
    EXPORT libdy++Targets COMPONENT c++
    RUNTIME DESTINATION bin COMPONENT c++
    LIBRARY DESTINATION lib COMPONENT c++
    ARCHIVE DESTINATION lib COMPONENT c++-dev
    PUBLIC_HEADER DESTINATION include/libdy++ COMPONENT c++-dev
)

# CMake Config
export(TARGETS libdy++ FILE "${PROJECT_BINARY_DIR}/libdy++Targets.cmake")
export(PACKAGE libdypp)

# for build tree
set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/..")
configure_file(libdy++Config.cmake.in "${PROJECT_BINARY_DIR}/libdy++Config.cmake" @ONLY)
# for install tree
set(INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
configure_file(libdy++Config.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libdy++Config.cmake" @ONLY)

# CMake version config
configure_file(libdy++ConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/libdy++ConfigVersion.cmake" @ONLY)

# install CMake configs
if (WIN32 AND NOT CYGWIN)
    set(LIB_CMAKE_PATH "cmake")
else()
    set(LIB_CMAKE_PATH "lib/cmake/libdy++")
endif()

install(EXPORT libdy++Targets DESTINATION "${LIB_CMAKE_PATH}" COMPONENT c++-dev)
install(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libdy++Config.cmake"
        "${PROJECT_BINARY_DIR}/libdy++ConfigVersion.cmake"
    DESTINATION "${LIB_CMAKE_PATH}" COMPONENT c++-dev
)
