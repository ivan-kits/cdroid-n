set(PLPLOT_INCLUDE_DIR ${plplot_SOURCE_DIR}/include 
      ${plplot_SOURCE_DIR}/bindings/c++ 
      ${plplot_BINARY_DIR}/include)

set(PLPLOT_LIBRARY plplot plplotcxx)
set(PLPLOT_LIBRARIES ${PLPLOT_LIBRARY})
set(PLPLOT_INCLUDE_DIRS ${PLPLOT_INCLUDE_DIR})
set(PLPLOT_FOUND TRUE)
