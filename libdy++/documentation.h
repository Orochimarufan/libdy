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

// This file only contains metainformation for Doxygen

/** @mainpage libdy C++ bindings: libdy++
 * @section mp_intro Introduction
 * libdy++ contains C++ bindings to the libdy dynamic data exchange library.
 *
 * @section mp_objects libdy++ Objects
 * libdy++ comes with a number of classes for wrapping libdy objects in C++ objects. \n
 * The Object class take care of reference counting (RAII) and operator overloading. \n
 * Various subclasses exist for some of the concrete types in libdy, featuring
 * typechecks on construction. \n
 * For more information see @link Objects the Objects module @endlink.
 *
 * @section mp_conversion Conversion between native types and libdy objects
 * Implicit conversion between native types and libdy objects can be enabled by
 * implementing a conversion template. Implementations for some C/C++ and Qt types
 * are shipped in optional header files. \n
 * For more information see @link Conversion the Conversion module @endlink.
 *
 * @section mp_errors Errors in libdy++
 * libdy++'s error handling is built around libdy exceptions and C++ exceptions. \n
 * The Exception class wraps a libdy exception object into a std::exception derived
 * C++ exception that can be thrown. \n
 * For more information see @link Exceptions the Exceptions module @endlink.
 *
 * @section mp_callable Creating callable libdy userdata objects with libdy++
 * libdy++ can create callable libdy userdata objects from any std::function
 * object. \n
 * There are two types of wrappers for 'functions' and 'methods', with
 * the method version providing the container the userdata was retrieved from as
 * the first argument if that information is available. \n
 * For more information see @link Callables the Callables module @endlink.
 *
 * @section mp_copy Copyright & Licensing
 * libdy and libdy++ are &copy; 2015 by Taeyeon Mori. \n
 * Currently, the only licensing option is the GNU GPL v3+ \n
 * @verbatim
Dynamic Data exchange library [libdy]
Copyright (C) 2015 Taeyeon Mori

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. @endverbatim
 */

/**
 * @defgroup Objects Objects
 * @brief The libdy object wrappers
 */

/** @ingroup Objects
 * @defgroup Constants Constants
 * @brief libdy constants
 */

/** @ingroup Objects
 * @defgroup Iterators Iterators
 * @brief C++ iterators over libdy objects
 */

/** @ingroup Objects
 * @defgroup Conversion Conversion
 * @brief Facilities to support conversion from and to libdy types
 *
 * Custom conversion paths can be created by specializing Dy::conv::convert
 */

/** @ingroup Objects
 * @defgroup Callables C++ Callables
 * @brief Callable userdata creation
 */

/**
 * @defgroup Exceptions Exceptions
 * @brief The libdy++ exception system
 * The libdy++ exception system is based on libdy exceptions wrapped in std::exception
 * compatible objects that are then thrown using the C++ exception mechanism.
 *
 * The libdy++ Exception objects can then be caught (by reference) in standard
 * C++ manner and handled using the appropriate methods.
 *
 * @warning Every error handler <b>MUST</b> call Exception::clear() when it's
 *          done handling the Exception and doesn't re-throw it. This will clear
 *          the underlying libdy error state. Failing to do so will result in
 *          undefined behavior on libdy calls because the error state is out
 *          of sync.
 */

/**
 * @defgroup Utility Utility
 * @brief Generic utilities used in libdy++
 */
