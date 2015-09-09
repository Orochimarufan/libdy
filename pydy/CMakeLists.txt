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

# NOTE: The CMake build of PyDy (libdy-python) is to be linked into programs
#       embedding python

project(pydy)
set(LIBDYPYTHON_VERSION "0.1.0")
set(LIBDYPYTHON_SOVERSION 0)

find_package(PythonLibs 3.3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

set(HEADERS
    pydy.h
)

set(SOURCES
    pydy.c
)

include_directories(..)
add_library(libdy-python SHARED ${HEADERS} ${SOURCES})
target_link_libraries(libdy-python libdy ${PYTHON_LIBRARIES})
set_target_properties(libdy-python PROPERTIES
    OUTPUT_NAME dy-python
    VERSION "${LIBDYPYTHON_VERSION}"
    SOVERSION "${LIBDYPYTHON_SOVERSION}"
    PUBLIC_HEADER "${HEADERS}"
)

# Install
install(TARGETS libdy-python
    EXPORT libdy-pythonTargets COMPONENT python
    RUNTIME DESTINATION bin COMPONENT python
    LIBRARY DESTINATION lib COMPONENT python
    ARCHIVE DESTINATION lib COMPONENT python-dev
    PUBLIC_HEADER DESTINATION include COMPONENT python-dev
)

# CMake Config
export(TARGETS libdy-python FILE "${PROJECT_BINARY_DIR}/libdy-pythonTargets.cmake")
export(PACKAGE libdy-python)

# for build tree
set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/..")
configure_file(libdy-pythonConfig.cmake.in "${PROJECT_BINARY_DIR}/libdy-pythonConfig.cmake" @ONLY)
# for install tree
set(INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
configure_file(libdy-pythonConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libdy-pythonConfig.cmake" @ONLY)

# CMake version config
configure_file(libdy-pythonConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/libdy-pythonConfigVersion.cmake" @ONLY)

# install CMake configs
install(EXPORT libdy-pythonTargets DESTINATION "${LIB_CMAKE_PATH}" COMPONENT python-dev)
install(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/libdy-pythonConfig.cmake"
        "${PROJECT_BINARY_DIR}/libdy-pythonConfigVersion.cmake"
    DESTINATION "${LIB_CMAKE_PATH}" COMPONENT python-dev
)
