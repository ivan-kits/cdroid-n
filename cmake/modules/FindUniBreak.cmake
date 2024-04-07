find_package(PkgConfig)
pkg_check_modules(PC_ZLIB  zlib)

find_path(UNIBREAK_INCLUDE_DIRS
    NAMES wordbreak.h
    HINTS ${PC_ZLIB_INCLUDEDIR}
    ${PC_ZLIB_INCLUDE_DIRS}
)

find_library(UNIBREAK_LIBRARIES
    NAMES unibreak
    HINTS ${PC_ZLIB_LIBDIR}
    ${PC_ZLIB_LIBRARY_DIRS}
)

#message("UNIBREAK_LIBRARIES=${UNIBREAK_LIBRARIES} UNIBREAK_INCLUDE_DIRS=${UNIBREAK_INCLUDE_DIRS} PC_UNIBREAK_VERSION=${PC_UNIBREAK_VERSION}")
if(UNIBREAK_INCLUDE_DIRS AND UNIBREAK_LIBRARIES)
    set(UNIBREAK_FOUND TRUE)
    set(UNIBREAK_LIBRARY ${UNIBREAK_LIBRARIES})
    set(UNIBREAK_INCLUDE_DIR ${UNIBREAK_INCLUDE_DIRS})
    set(UNIBREAK_VERSION "5.0.0")
    if(NOT TARGET libunibreak::libunibreak)
        add_library(libunibreak::libunibreak UNKNOWN IMPORTED)
        set_target_properties(libunibreak::libunibreak PROPERTIES
          INTERFACE_COMPILE_DEFINITIONS "${_PNG_COMPILE_DEFINITIONS}"
	  INTERFACE_INCLUDE_DIRECTORIES "${UNIBREAK_INCLUDE_DIRS}")

	if(EXISTS "${UNIBREAK_LIBRARY}")
          set_target_properties(libunibreak::libunibreak PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
	    IMPORTED_LOCATION "${UNIBREAK_LIBRARY}")
        endif()
    endif()
endif()

include(FindPackageHandleStandardArgs)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(UniBreak REQUIRED_VARS UNIBREAK_INCLUDE_DIRS UNIBREAK_LIBRARIES 
#	FOUND_VAR UNIBREAK_FOUND VERSION_VAR UNIBREAK_VERSION)

mark_as_advanced(UNIBREAK_LIBRARIES UNIBREAK_LIBRARY UNIBREAK_INCLUDE_DIRS UNIBREAK_INCLUDE_DIR UNIBREAK_FOUND UNIBREAK_VERSION)

