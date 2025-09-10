# EnableDocs.cmake - Configure Doxygen documentation for iosched project
#
# This module configures Doxygen documentation generation when IOSCHED_ENABLE_DOCS is enabled.
# It handles Doxygen configuration, creates docs targets, and sets up GitHub Pages deployment.

if(NOT IOSCHED_ENABLE_DOCS)
    return()
endif()

message(STATUS "Configuring Doxygen documentation")

# Find Doxygen
find_package(Doxygen REQUIRED)
if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not found. Please install doxygen to build documentation.")
endif()

# Add option to control documentation scope
option(IOSCHED_DOCS_PUBLIC_ONLY "Only generate documentation for public API" ON)

if(IOSCHED_DOCS_PUBLIC_ONLY)
    message(STATUS "Generating public API documentation only.")
    set(PUBLIC_API_HEADER ${CMAKE_SOURCE_DIR}/src/io.hpp)

    # Read the public API header and find all exported files
    file(STRINGS ${PUBLIC_API_HEADER} public_api_includes REGEX "#include.*IWYU pragma: export")

    set(public_doc_files "")
    foreach(include_line ${public_api_includes})
        # Extract the file path from the include line
        string(REGEX REPLACE "^#include \"([^\"]+)\".*" "\\1" header_path ${include_line})
        list(APPEND public_doc_files "${CMAKE_SOURCE_DIR}/src/${header_path}")
    endforeach()

    # Doxygen's INPUT tag requires a space-separated string.
    # We must create a single string variable containing all paths.
    string(REPLACE ";" " " public_doc_files_str "${public_doc_files}")
    set(DOXYGEN_INPUT_FILES "${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/DEVELOPER.md ${public_doc_files_str}")
else()
    message(STATUS "Generating documentation for all sources.")
    # For non-public builds, include all sources
    set(DOXYGEN_INPUT_FILES
        "${CMAKE_SOURCE_DIR}/README.md"
        "${CMAKE_SOURCE_DIR}/DEVELOPER.md"
        "${CMAKE_SOURCE_DIR}/src"
    )
endif()


# Set input and output directories
set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/html)

# Request to configure the file
configure_file(${DOXYGEN_IN} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

# Create docs directory in build folder
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)

# Add custom target for documentation generation
add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

# Add target to deploy docs to GitHub Pages deployment
add_custom_target(docs-deploy
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/docs/html ${CMAKE_CURRENT_SOURCE_DIR}/docs/html
    COMMENT "Copying documentation to docs/html for GitHub Pages deployment"
    DEPENDS docs
    VERBATIM
)

message(STATUS "Doxygen documentation enabled. Use 'cmake --build . --target docs' to generate documentation.")
