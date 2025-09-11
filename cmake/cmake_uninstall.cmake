message(STATUS "Uninstalling project...")

if(EXISTS "install_manifest.txt")
    file(STRINGS "install_manifest.txt" manifest_files)
    foreach(file ${manifest_files})
        message(STATUS "Uninstalling ${file}")
        file(REMOVE "${file}")
    endforeach()
else()
    message(STATUS "install_manifest.txt not found. Using fallback uninstall method.")

    # Remove shared library
    if(EXISTS "@CMAKE_INSTALL_PREFIX@/lib/libio.so")
      file(REMOVE "@CMAKE_INSTALL_PREFIX@/lib/libio.so")
      message(STATUS "Removed @CMAKE_INSTALL_PREFIX@/lib/libio.so")
    endif()
    if(EXISTS "@CMAKE_INSTALL_PREFIX@/lib/libio.dylib")
      file(REMOVE "@CMAKE_INSTALL_PREFIX@/lib/libio.dylib")
      message(STATUS "Removed @CMAKE_INSTALL_PREFIX@/lib/libio.dylib")
    endif()
    if(EXISTS "@CMAKE_INSTALL_PREFIX@/bin/io.dll")
      file(REMOVE "@CMAKE_INSTALL_PREFIX@/bin/io.dll")
      message(STATUS "Removed @CMAKE_INSTALL_PREFIX@/bin/io.dll")
    endif()

    # Remove static library
    if(EXISTS "@CMAKE_INSTALL_PREFIX@/lib/libio.a")
      file(REMOVE "@CMAKE_INSTALL_PREFIX@/lib/libio.a")
      message(STATUS "Removed @CMAKE_INSTALL_PREFIX@/lib/libio.a")
    endif()
    if(EXISTS "@CMAKE_INSTALL_PREFIX@/lib/io.lib")
      file(REMOVE "@CMAKE_INSTALL_PREFIX@/lib/io.lib")
      message(STATUS "Removed @CMAKE_INSTALL_PREFIX@/lib/io.lib")
    endif()

    # Remove include directory
    if(EXISTS "@CMAKE_INSTALL_PREFIX@/include/io")
      file(REMOVE_RECURSE "@CMAKE_INSTALL_PREFIX@/include/io")
      message(STATUS "Removed @CMAKE_INSTALL_PREFIX@/include/io")
    endif()
endif()

message(STATUS "Uninstall complete.")
