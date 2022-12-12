find_package(PkgConfig)
pkg_check_modules(PC_TURBOJPEG  libturbojpeg)

find_path(TURBOJPEG_INCLUDE_DIRS
    NAMES turbojpeg.h
    HINTS ${PC_TURBOJPEG_INCLUDEDIR}
    ${PC_TURBOJPEG_INCLUDE_DIRS}
)

find_library(TURBOJPEG_LIBRARIES
    NAMES turbojpeg
    HINTS ${PC_TURBOJPEG_LIBDIR}
    ${PC_TURBOJPEG_LIBRARY_DIRS}
)

if(TURBOJPEG_INCLUDE_DIRS AND TURBOJPEG_LIBRARIES)
    set(TURBOJPEG_FOUND TRUE)
    set(TURBOJPEG_LIBRARY ${TURBOJPEG_LIBRARIES})
    set(TURBOJPEG_INCLUDE_DIR ${TURBOJPEG_INCLUDE_DIRS})
    set(TURBOJPEG_VERSION ${PC_TURBOJPEG_VERSION})
endif()

include(FindPackageHandleStandardArgs)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(zlib REQUIRED_VARS TURBOJPEG_INCLUDE_DIRS TURBOJPEG_LIBRARIES 
#	FOUND_VAR TURBOJPEG_FOUND VERSION_VAR TURBOJPEG_VERSION)

mark_as_advanced(TURBOJPEG_LIBRARIES TURBOJPEG_LIBRARY TURBOJPEG_INCLUDE_DIRS TURBOJPEG_INCLUDE_DIR TURBOJPEG_FOUND)
