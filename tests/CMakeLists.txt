add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
if (BUILD_CDROID_TESTS)
  add_subdirectory(gui)
  add_subdirectory(porting)

endif(BUILD_CDROID_TESTS)
