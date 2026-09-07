cmake_minimum_required(VERSION 3.15)
project(DiffEqHostTests LANGUAGES C)
set(CMAKE_C_STANDARD 11)
enable_testing()
add_compile_options(-Wall -Wextra -Wpedantic -Werror -g -O1 -UNDEBUG)
option(SANITIZE "Enable undefined behavior sanitizer" ON)
if(SANITIZE AND CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined)
endif()
add_library(core STATIC ../src/ode/validity.c ../src/ode/ode.c ../src/ode/rk4.c ../src/ode/gsolve.c ../src/math/expression.c ../src/ode/model.c ../src/app_state.c ../src/storage.c ../src/ode/table.c ../src/ode/sampling.c ../src/graph/geometry.c)
target_include_directories(core PUBLIC ../include host)
# Host-only function entry counters and syscall wrappers; target sources/build
# receive none of these options. Counts are printed beside framebuffer hashes.
target_compile_options(core PRIVATE -finstrument-functions)
set_source_files_properties(../src/storage.c PROPERTIES COMPILE_DEFINITIONS
  "write=host_write;read=host_read;open=host_open;close=host_close")
add_library(host_metrics STATIC host/metrics.c)
target_include_directories(host_metrics PRIVATE ../include)
target_link_libraries(core PUBLIC host_metrics)
add_executable(test_core test_core.c)
target_link_libraries(test_core core m)
add_test(NAME numerical_core COMMAND test_core)

add_executable(test_expression test_expression.c)
target_link_libraries(test_expression core m)
add_test(NAME expressions COMMAND test_expression)
set_tests_properties(numerical_core expressions PROPERTIES TIMEOUT 30)

add_executable(test_model test_model.c)
target_link_libraries(test_model core m)
add_test(NAME equation_models COMMAND test_model)
set_tests_properties(equation_models PROPERTIES TIMEOUT 30)

# The actual UI/app code against a small drawing/key adapter, not a CPU emulator.
set(HOST_APP_SOURCES ../src/graph/trace.c ../src/main.c ../src/app.c ../src/ui/common.c
  ../src/ui/editor.c ../src/ui/forms.c ../src/graph/renderer.c ../src/ui/graph_screen.c
  ../src/ui/table_screen.c host/gint_host.c)
add_executable(host_app ${HOST_APP_SOURCES})
target_include_directories(host_app PRIVATE host)
target_link_libraries(host_app core m)
target_compile_definitions(host_app PRIVATE DIFFEQ_STORAGE_DIR=".")
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME ui_workflows COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_ui.py $<TARGET_FILE:host_app>)
add_test(NAME explicit_session_policy COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_autosave.py $<TARGET_FILE:host_app>)
set_tests_properties(ui_workflows PROPERTIES TIMEOUT 60)
add_executable(test_graph test_graph.c)
target_link_libraries(test_graph core m)
add_test(NAME graph_geometry COMMAND test_graph)
set_tests_properties(graph_geometry PROPERTIES TIMEOUT 30)

add_executable(test_gsolve test_gsolve.c)
target_link_libraries(test_gsolve core m)
add_test(NAME gsolve_numerics COMMAND test_gsolve)
set_tests_properties(gsolve_numerics PROPERTIES TIMEOUT 30)

add_executable(test_table_storage test_table_storage.c)
target_link_libraries(test_table_storage core m)
add_test(NAME table_and_storage COMMAND test_table_storage)
set_tests_properties(table_and_storage PROPERTIES TIMEOUT 30)

add_executable(test_navigation test_navigation.c)
target_link_libraries(test_navigation core m)
add_test(NAME navigation_stress COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_navigation.py $<TARGET_FILE:test_navigation>
  $<TARGET_FILE:host_app>)
set_tests_properties(navigation_stress PROPERTIES TIMEOUT 30)

add_executable(test_graph_auto test_graph_auto.c ../src/graph/renderer.c
  ../src/ui/common.c host/gint_host.c)
target_include_directories(test_graph_auto PRIVATE host)
target_link_libraries(test_graph_auto core m)
add_test(NAME graph_auto_y COMMAND test_graph_auto)

# Compile the target common.c cancellation/timer branches with deterministic
# getkey/timer adapters. This tests app policy, not gint's OS implementation.
add_executable(test_key_lifecycle test_key_lifecycle.c ../src/ui/common.c host/gint_host.c)
target_include_directories(test_key_lifecycle BEFORE PRIVATE native-keys host)
target_compile_definitions(test_key_lifecycle PRIVATE FXCG50 DIFFEQ_TEST_NATIVE_KEYS)
target_link_libraries(test_key_lifecycle core m)
add_test(NAME key_lifecycle COMMAND test_key_lifecycle)
add_test(NAME trace_repeat COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_trace.py $<TARGET_FILE:host_app>)
add_executable(test_colors test_colors.c ../src/graph/renderer.c ../src/ui/common.c host/gint_host.c)
target_include_directories(test_colors PRIVATE host)
target_link_libraries(test_colors core m)
add_test(NAME output_color_storage COMMAND test_colors)
add_executable(test_fields test_fields.c ../src/ui/editor.c ../src/ui/common.c host/gint_host.c)
target_include_directories(test_fields PRIVATE host)
target_link_libraries(test_fields core m)
add_test(NAME field_selection COMMAND test_fields)
add_test(NAME hardware_feedback_ui COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_feedback.py $<TARGET_FILE:host_app>)
set_tests_properties(hardware_feedback_ui PROPERTIES TIMEOUT 60)
foreach(validity_test test_valid_regions test_segment_stream)
  add_executable(${validity_test} ${validity_test}.c ../src/graph/trace.c ../src/graph/renderer.c ../src/ui/common.c host/gint_host.c)
  target_include_directories(${validity_test} PRIVATE host)
  target_link_libraries(${validity_test} core m)
  add_test(NAME ${validity_test} COMMAND ${validity_test})
endforeach()
