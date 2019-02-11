
add_library(ZSTD INTERFACE)

target_include_directories(ZSTD INTERFACE ${ZEPHYR_BASE}/include/zstd)

if(CONFIG_ZSTD_USING_AS_STD)
    target_compile_options(app PUBLIC -include "zstd.h")
endif()

zephyr_library()
zephyr_library_sources(
  src/time.c
  src/mutex.cpp
  src/thread.cpp
)

zephyr_library_link_libraries(ZSTD)
target_link_libraries(ZSTD INTERFACE zephyr_interface)
