cmake_minimum_required(VERSION 3.15)
project(DiffEqHello LANGUAGES C)
include(GenerateG3A)
find_package(Gint 2.11 REQUIRED)
add_executable(diffeq-hello src/main.c)
target_compile_options(diffeq-hello PRIVATE -Wall -Wextra -Werror -Os)
target_link_libraries(diffeq-hello Gint::Gint)
generate_g3a(TARGET diffeq-hello OUTPUT "DIFFEQ-hello.g3a"
  NAME "DEQ TEST" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
