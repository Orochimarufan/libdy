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

project(libdy)
set(LIBDY_VERSION 0.1.0)
set(LIBDY_SOVERSION 0)

set(PRIVATE_HEADERS
    dict_p.h
    dy_p.h
    freelist_p.h
    host_p.h
    list_p.h
    string_p.h
    userdata_p.h
)

set(HEADERS
    config.h
    runtime.h

    buildstring.h
    json_token.h
    linalloc.h

    types.h
    object.h
    constants.h
    numbers.h
    dystring.h
    collections.h
    json.h
    call.h
    userdata.h

    exceptions.h
    exceptions_c.h

    dy.h
)

set(SOURCES
    buildstring.c
    dict.c
    dy.c
    error.c
    freelist.c
    hash.c
    host.c
    json.c
    json_token.c
    linalloc.c
    list.c
    string.c
    string_intern.c
    userdata.c
)

add_definitions(-DBUILDING_LIBDY_CORE)
add_library(libdy SHARED ${HEADERS} ${PRIVATE_HEADERS} ${SOURCES})
set_target_properties(libdy PROPERTIES
    OUTPUT_NAME dy
    VERSION "${LIBDY_VERSION}"
    SOVERSION "${LIBDY_SOVERSION}"
    PUBLIC_HEADER "${HEADERS}"
)

if (UNIX AND NOT APPLE)
   target_link_libraries(libdy m)
endif()

# Install
install(TARGETS libdy
    EXPORT libdyTargets COMPONENT core
    RUNTIME DESTINATION bin COMPONENT core
    LIBRARY DESTINATION lib COMPONENT core
    ARCHIVE DESTINATION lib COMPONENT core-dev
    PUBLIC_HEADER DESTINATION include/libdy COMPONENT core-dev
)

# CMake Config
export(TARGETS libdy FILE "${PROJECT_BINARY_DIR}/libdyTargets.cmake")
export(PACKAGE libdy)

# for build tree
set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/..")
configure_file(libdyConfig.cmake.in "${PROJECT_BINARY_DIR}/libdyConfig.cmake" @ONLY)
# for install tree
set(INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
configure_file(libdyConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libdyConfig.cmake" @ONLY)

# CMake version config
configure_file(libdyConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/libdyConfigVersion.cmake" @ONLY)

# install CMake configs
if (WIN32 AND NOT CYGWIN)
    set(LIB_CMAKE_PATH "cmake")
else()
    set(LIB_CMAKE_PATH "lib/cmake/libdy")
endif()

install(EXPORT libdyTargets DESTINATION "${LIB_CMAKE_PATH}" COMPONENT core-dev)
install(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libdyConfig.cmake"
        "${PROJECT_BINARY_DIR}/libdyConfigVersion.cmake"
    DESTINATION "${LIB_CMAKE_PATH}" COMPONENT core-dev
)
