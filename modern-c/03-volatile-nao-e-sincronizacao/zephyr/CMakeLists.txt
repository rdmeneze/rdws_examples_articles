cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(volatile_nao_e_sincronizacao)

# O buffer de Felipe Neves é baixado por scripts/baixa_neves.sh (não é redistribuído aqui).
set(NEVES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../build/neves-src CACHE PATH "buffer_circ.c/.h")
if(NOT EXISTS ${NEVES_DIR}/buffer_circ.c)
  message(FATAL_ERROR "Rode antes: scripts/baixa_neves.sh ${NEVES_DIR}")
endif()

target_sources(app PRIVATE src/main.c ${NEVES_DIR}/buffer_circ.c)
target_include_directories(app PRIVATE ${NEVES_DIR})
