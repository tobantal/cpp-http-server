#[=======================================================================]
# @file AddHttpService.cmake
# @brief CMake function to generate HTTP service executable
#
# Generates a minimal main.cpp that instantiates the application class
# and calls run(). Backward compatible: existing main.cpp files work as-is.
#
# Usage:
#   add_http_service(TARGET MyService APP_CLASS MyNamespace::MyApp)
#
# The user's APP_CLASS must:
#   - Inherit from BoostBeastApplication or implement IWebApplication
#   - Have a run(int argc, char* argv[]) method
#   - Be accessible via include directories
#
# Example:
#   add_http_service(TARGET my-service APP_CLASS MyServiceApp)
#
#   target_include_directories(my-service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
#=======================================================================]

include_guard(GLOBAL)

#[=======================================================================]
# @brief Generate and build an HTTP service executable
# @param TARGET Name of the executable target
# @param APP_CLASS Fully-qualified application class (e.g. MyNamespace::MyApp)
#=======================================================================]
function(add_http_service TARGET APP_CLASS)
    cmake_parse_arguments(PARSE_ARGV 2
        ARG ""          # No options
        ""              # No one-value options
        ""              # No multi-value options
    )

    if(NOT ARG_UNPARSED_ARGUMENTS STREQUAL "")
        message(FATAL_ERROR "add_http_service: Unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "add_http_service: TARGET is required")
    endif()

    if(NOT DEFINED ARG_APP_CLASS)
        message(FATAL_ERROR "add_http_service: APP_CLASS is required")
    endif()

    # Extract namespace and class name
    # APP_CLASS can be "MyClass" or "MyNamespace::MyClass"
    if(ARG_APP_CLASS MATCHES "^(.+::)?(.+)$")
        set(_namespace "${CMAKE_MATCH_1}")
        set(_class_name "${CMAKE_MATCH_2}")
    else()
        message(FATAL_ERROR "add_http_service: Invalid APP_CLASS format: ${ARG_APP_CLASS}")
    endif()

    # Remove trailing :: from namespace if present
    string(REGEX REPLACE "::$" "" _namespace "${_namespace}")

    # Default include path: lowercase underscore version of class name
    # e.g. MyServiceApp -> my_service_app.hpp
    if(_namespace STREQUAL "")
        set(_default_include "${_class_name}.hpp")
    else()
        # Convert "MyNamespace::MyClass" -> "my_namespace/my_class.hpp"
        string(REPLACE "::" "/" _include_path "${ARG_APP_CLASS}")
        string(TOLOWER "${_include_path}" _include_path_lower)
        set(_default_include "${_include_path_lower}.hpp")
    endif()

    # Generate main.cpp content
    set(_generated_main_content "
#include \"${_default_include}\"

int main(int argc, char* argv[]) {
    ${ARG_APP_CLASS} app;
    return app.run(argc, argv);
}
")

    # Write generated main.cpp to build directory
    set(_generated_main_file "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_main.cpp")
    file(WRITE "${_generated_main_file}" "${_generated_main_content}")

    # Create executable target with generated source
    add_executable(${TARGET} ${_generated_main_file})

    # Link with microservice-boost
    target_link_libraries(${TARGET} PRIVATE microservice-boost)

    # Set C++ standard
    target_compile_features(${TARGET} PRIVATE cxx_std_17)

    message(STATUS "add_http_service: Created target '${TARGET}' with APP_CLASS '${ARG_APP_CLASS}'")
    message(STATUS "add_http_service: Generated main.cpp includes '${_default_include}'")
endfunction()
