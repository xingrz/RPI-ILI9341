project(bcm2835)

add_library(bcm2835 STATIC "bcm2835.c")

target_include_directories(
    bcm2835 PUBLIC
    ${bcm2835_SOURCE_DIR}
)
