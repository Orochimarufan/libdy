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

cmake_minimum_required(VERSION 3.0)

######## Set compiler flags ########
IF(APPLE)
    # assume clang, add C++1y/C++14 stuff
    message(STATUS "Using APPLE CMAKE_CXX_FLAGS")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++")
ELSEIF(UNIX)
    # assume GCC, add C++1y/C++14 stuff
    MESSAGE(STATUS "Using UNIX CMAKE_CXX_FLAGS")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
ELSEIF(MINGW)
    MESSAGE(STATUS "Using MINGW CMAKE_CXX_FLAGS")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y")
ENDIF()

if (WIN32 AND NOT CYGWIN)
    set(LIB_CMAKE_PATH "cmake")
else()
    set(LIB_CMAKE_PATH "lib/cmake/libdy")
endif()

add_subdirectory(libdy)
add_subdirectory(libdy++)
add_subdirectory(pydy)
add_subdirectory(test EXCLUDE_FROM_ALL)
