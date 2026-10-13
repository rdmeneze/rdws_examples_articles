cmake_minimum_required(VERSION 3.20)
project(servidor_coap_dtls_psk C)

include(FetchContent)

find_package(OpenSSL REQUIRED)

# libcoap com DTLS pelo OpenSSL. Os exemplos do libcoap ficam ligados para o build
# entregar também o coap-client, usado no teste (../scripts/teste.sh).
set(ENABLE_DTLS ON CACHE BOOL "" FORCE)
set(DTLS_BACKEND "openssl" CACHE STRING "" FORCE)
set(ENABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    libcoap
    URL https://github.com/obgm/libcoap/archive/refs/tags/v4.3.5.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(libcoap)

add_executable(servidor servidor.c)
target_link_libraries(servidor PRIVATE coap-3)
