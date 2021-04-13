project(ili9341)

add_executable(
    ili9341
    src/main.c
    src/ili9341.c
)

target_link_libraries(ili9341 bcm2835)

install(TARGETS ili9341 DESTINATION bin)
