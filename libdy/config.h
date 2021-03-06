/*
 *  Dynamic Data exchange library [libdy]
 *  Copyright (C) 2015 Taeyeon Mori
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// ----------------------------------------------------------------------------
// API macros
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_LIBDY_CORE
	#define LIBDY_API __declspec(dllexport)
  #else
	#define LIBDY_API __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define LIBDY_API __attribute__ ((visibility ("default")))
  #else
    #define LIBDY_API
  #endif
#endif
