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

# Set input and output directories
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
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

# Add target to deploy docs to GitHub Pages format
add_custom_target(docs-deploy
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/docs/html ${CMAKE_CURRENT_SOURCE_DIR}/docs/html
    COMMENT "Copying documentation to docs/html for GitHub Pages deployment"
    DEPENDS docs
    VERBATIM
)

message(STATUS "Doxygen documentation enabled. Use 'cmake --build . --target docs' to generate documentation.")