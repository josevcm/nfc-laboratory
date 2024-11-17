set(CMAKE_C_STANDARD 99)

set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c)

add_library(ed25519 STATIC
        src/main/c/add_scalar.c
        src/main/c/fe.c
        src/main/c/ge.c
        src/main/c/key_exchange.c
        src/main/c/keypair.c
        src/main/c/sc.c
        src/main/c/seed.c
        src/main/c/sha512.c
        src/main/c/sign.c
        src/main/c/verify.c
)

target_compile_options(ed25519 PRIVATE -DED25519_NO_SEED)

target_include_directories(ed25519 PUBLIC ${PUBLIC_SOURCE_DIR})
target_include_directories(ed25519 PRIVATE ${LIBUSB_INCLUDE_DIR})

target_link_libraries(ed25519)