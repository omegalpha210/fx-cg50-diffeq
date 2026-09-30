cmake_minimum_required(VERSION 3.15)
project(DiffEq VERSION 0.12.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)

include(GenerateG3A)
find_package(Gint 2.11 REQUIRED)

# Runtime sources only. Host adapters, instrumentation and fault injection are
# configured separately in tests/CMakeLists.txt. Preserve the existing link order.
set(DIFFEQ_TARGET_SOURCES
  src/ode/events.c
  src/ode/rk45.c
  src/ode/solver.c
  src/ode/phase.c
  src/graph/phase.c
  src/ode/initial.c
  src/ode/validity.c
  src/graph/trace.c
  src/main.c
  src/app.c
  src/app_state.c
  src/ode/ode.c
  src/ode/rk4.c
  src/ode/gsolve.c
  src/math/expression.c
  src/ode/model.c
  src/ui/menu.c
  src/ui/common.c
  src/ui/editor.c
  src/ui/forms.c
  src/ui/solver.c
  src/storage.c
  src/ode/table.c
  src/ui/table_screen.c
  src/ode/sampling.c
  src/graph/geometry.c
  src/graph/renderer.c
  src/ui/graph_screen.c
  src/power.c
)
add_executable(diffeq ${DIFFEQ_TARGET_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${DIFFEQ_TARGET_SOURCES})
target_include_directories(diffeq PRIVATE include)
target_compile_options(diffeq PRIVATE
  -Wall -Wextra -Werror -Wframe-larger-than=3072 -Os -g -fstack-usage)
target_link_options(diffeq PRIVATE "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/diffeq.map")
target_link_libraries(diffeq Gint::Gint)

# Final user output stays in dist; object files, ELF and map stay in build/target.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist")
generate_g3a(TARGET diffeq
  OUTPUT "dist/DIFFEQ.g3a"
  NAME "DIFF EQ"
  VERSION "00.12.0000"
  ICONS assets/icon-uns.png assets/icon-sel.png)
