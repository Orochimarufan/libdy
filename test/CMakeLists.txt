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

project(libdy_tests)
include_directories(..)

add_executable(libdy_test main.c)
target_link_libraries(libdy_test libdy)

add_executable(libdy++_test main.cpp)
target_link_libraries(libdy++_test libdy++)
set_target_properties(libdy++_test PROPERTIES LINKER_LANGUAGE CXX)

add_executable(libdy_json_test test_json.c)
target_link_libraries(libdy_json_test libdy)

add_executable(libdy_json_test_file test_jsonfile.c)
target_link_libraries(libdy_json_test_file libdy)

add_custom_target(tests COMMENT Build all test executables)
add_dependencies(tests libdy_test libdy++_test libdy_json_test libdy_json_test_file)
