add_library(zstd STATIC
  zstd.h
  zstd_errors.h
  zstddeclib.c
)

set_target_properties(zstd PROPERTIES POSITION_INDEPENDENT_CODE OFF)
if(BUILD_PSP2)
  target_compile_options(zstd PRIVATE -fno-PIC -fno-pie -mword-relocations -marm)
endif()
