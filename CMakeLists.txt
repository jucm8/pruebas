# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

add_subdirectory(src)

add_llext_target(
  extension1
  OUTPUT ext1.llext
  SOURCES ./src/ext1.c ./src/embench/beebsc.c
)
