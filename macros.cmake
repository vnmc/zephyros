# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Shared macros.
#


# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MACOSX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
endif()

# define the platform preprocessor definition
if(OS_WINDOWS)
  set(PLATFORM -DOS_WIN)
elseif(OS_MACOSX)
  set(PLATFORM -DOS_MACOSX)
elseif(OS_LINUX)
  set(PLATFORM -DOS_LINUX)
endif()


# CEF distributable files
if(OS_WINDOWS)
  set(CEF_BINARY_FILES
    d3dcompiler_43.dll
    d3dcompiler_47.dll
    libcef.dll
    libEGL.dll
    libGLESv2.dll
    natives_blob.bin
    snapshot_blob.bin
  )
  if(PROJECT_ARCH STREQUAL "x86")
    # only used on 32-bit platforms
    set(CEF_BINARY_FILES
      ${CEF_BINARY_FILES}
      wow_helper.exe
    )
  endif()
elseif(OS_LINUX)
  # list of CEF binary files
  set(CEF_BINARY_FILES
    libcef.so
    natives_blob.bin
    snapshot_blob.bin
  )
endif()

# list of CEF resource files
set(CEF_RESOURCE_FILES
  cef.pak
  cef_100_percent.pak
  cef_200_percent.pak
  cef_extensions.pak
  devtools_resources.pak
  icudtl.dat
  locales
)


# WinSparkle distributable files
if(OS_WINDOWS)
  set(WINSPARKLE_BINARY_FILES
    WinSparkle.dll
  )
endif()

# CrashRpt distributable files
if(OS_WINDOWS)
  set(CRASHREPORTER_BINARY_FILES
    crashrpt_lang.ini
    CrashRpt1402.dll
    CrashSender1402.exe
    dbghelp.dll
  )
endif()


# Append platform specific sources to a list of sources.
macro(APPEND_PLATFORM_SOURCES name_of_list)
  if(OS_LINUX AND ${name_of_list}_LINUX)
    list(APPEND ${name_of_list} ${${name_of_list}_LINUX})
  endif()
  if(OS_POSIX AND ${name_of_list}_POSIX)
    list(APPEND ${name_of_list} ${${name_of_list}_POSIX})
  endif()
  if(OS_WINDOWS AND ${name_of_list}_WINDOWS)
    list(APPEND ${name_of_list} ${${name_of_list}_WINDOWS})
  endif()
  if(OS_MACOSX AND ${name_of_list}_MACOSX)
    list(APPEND ${name_of_list} ${${name_of_list}_MACOSX})
  endif()
endmacro()

# Add a logical target that can be used to link the specified libraries into an
# executable target.
macro(ADD_LOGICAL_TARGET target debug_lib release_lib)
  add_library(${target} ${CEF_LIBTYPE} IMPORTED)
  set_target_properties(${target} PROPERTIES
    IMPORTED_LOCATION "${release_lib}"
    IMPORTED_LOCATION_DEBUG "${debug_lib}"
    IMPORTED_LOCATION_RELEASE "${release_lib}"
    )
endmacro()

# Determine the target output directory based on platform and generator.
macro(SET_CEF_TARGET_OUT_DIR)
  if(${CMAKE_GENERATOR} STREQUAL "Ninja")
    # Ninja does not create a subdirectory named after the configuration.
    set(CEF_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  elseif(OS_LINUX)
    set(CEF_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
  else()
    set(CEF_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
  endif()
endmacro()

# Copy a list of files from one directory to another. Relative files paths are maintained.
macro(COPY_FILES target file_list source_dir target_dir)
  foreach(FILENAME ${file_list})
    set(source_file ${source_dir}/${FILENAME})
    set(target_file ${target_dir}/${FILENAME})
    if(IS_DIRECTORY ${source_file})
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_file}" "${target_file}"
        VERBATIM
        )
    else()
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source_file}" "${target_file}"
        VERBATIM
        )
    endif()
  endforeach()
endmacro()

# Copy a file/directory keeping symbolic links
macro(COPY_R target src dest)
  get_filename_component(directory_path "${dest}" DIRECTORY)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND mkdir -p "${directory_path}"
    COMMAND rm -rf "${dest}"
    COMMAND cp -R "${src}" "${dest}"
    VERBATIM
  )
endmacro()

# Rename a directory replacing the target if it already exists.
macro(RENAME_DIRECTORY target source_dir target_dir)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    # Remove the target directory if it already exists.
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${target_dir}"
    # Rename the source directory to target directory.
    COMMAND ${CMAKE_COMMAND} -E rename "${source_dir}" "${target_dir}"
    VERBATIM
    )
endmacro()

macro(CODESIGN target app_path app_name codesigning_id) 
  # codesign Helper Bundle
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND codesign -s ${codesigning_id} --deep -f 
    "${app_path}/${app_name}"
    VERBATIM
  )
endmacro()


#
# Linux macros.
#

if(OS_LINUX)

# Use pkg-config to find Linux libraries and update compiler/linker variables.
macro(FIND_LINUX_LIBRARIES libraries)
  # Read pkg-config info into variables.
  execute_process(COMMAND pkg-config --cflags ${libraries} OUTPUT_VARIABLE FLL_CFLAGS)
  execute_process(COMMAND pkg-config --libs-only-L --libs-only-other ${libraries} OUTPUT_VARIABLE FLL_LDFLAGS)
  execute_process(COMMAND pkg-config --libs-only-l ${libraries} OUTPUT_VARIABLE FLL_LIBS)

  # Strip leading and trailing whitepspace.
  STRING(STRIP "${FLL_CFLAGS}"  FLL_CFLAGS)
  STRING(STRIP "${FLL_LDFLAGS}" FLL_LDFLAGS)
  STRING(STRIP "${FLL_LIBS}"    FLL_LIBS)

  # Update the variables.
  set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} ${FLL_CFLAGS}")
  set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} ${FLL_CFLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} ${FLL_LDFLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FLL_LDFLAGS}")
  set(CEF_STANDARD_LIBS         "${CEF_STANDARD_LIBS} ${FLL_LIBS}")
endmacro()

# Set SUID permissions on the specified executable.
macro(SET_LINUX_SUID_PERMISSIONS target executable)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "*** Run the following command manually to set SUID permissions ***"
    COMMAND ${CMAKE_COMMAND} -E echo "EXE=\"${executable}\" && sudo -- chown root:root $EXE && sudo -- chmod 4755 $EXE"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    VERBATIM
    )
endmacro()

endif(OS_LINUX)


#
# Mac OS X macros.
#

if(OS_MACOSX)

# Set Xcode target properties.
function(SET_XCODE_TARGET_PROPERTIES target)
  set_target_properties(${target} PROPERTIES
    XCODE_ATTRIBUTE_ALWAYS_SEARCH_USER_PATHS                    NO
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD                 "c++11"     # -std=c++11
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY                           "libc++"
    XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME                     NO          # -fno-objc-link-runtime
    XCODE_ATTRIBUTE_CLANG_WARN_OBJC_MISSING_PROPERTY_SYNTHESIS  NO          # -Wno-objc-missing-property-synthesis
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP                            NO
    XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING[variant=Release]        YES         # -Wl,-dead_strip
    XCODE_ATTRIBUTE_GCC_C_LANGUAGE_STANDARD                     "c99"       # -std=c99
    XCODE_ATTRIBUTE_GCC_CW_ASM_SYNTAX                           NO          # No -fasm-blocks
    XCODE_ATTRIBUTE_GCC_DYNAMIC_NO_PIC                          NO
    XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS                   NO          # -fno-exceptions
    XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI                         NO          # -fno-rtti
    XCODE_ATTRIBUTE_GCC_ENABLE_PASCAL_STRINGS                   NO          # No -mpascal-strings
    XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN              YES         # -fvisibility-inlines-hidden
    XCODE_ATTRIBUTE_GCC_OBJC_CALL_CXX_CDTORS                    YES         # -fobjc-call-cxx-cdtors
    XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN                  NO          # -fvisibility=hidden
    XCODE_ATTRIBUTE_GCC_THREADSAFE_STATICS                      YES         # -fthreadsafe-statics
    XCODE_ATTRIBUTE_GCC_TREAT_WARNINGS_AS_ERRORS                NO          # -Werror
    XCODE_ATTRIBUTE_GCC_VERSION                                 "com.apple.compilers.llvm.clang.1_0"
    XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE              YES         # -Wnewline-eof
    XCODE_ATTRIBUTE_USE_HEADERMAP                               NO
    OSX_ARCHITECTURES_DEBUG                                     "${CMAKE_OSX_ARCHITECTURES}"
    OSX_ARCHITECTURES_RELEASE                                   "${CMAKE_OSX_ARCHITECTURES}"
    )
endfunction()

# Override default add_library function.
function(add_library name)
  _add_library(${name} ${ARGN})
  SET_XCODE_TARGET_PROPERTIES(${name})
endfunction()

# Override default add_executable function.
function(add_executable name)
  _add_executable(${name} ${ARGN})
  SET_XCODE_TARGET_PROPERTIES(${name})
endfunction()

# Fix the framework link in the helper executable.
macro(FIX_MACOSX_HELPER_FRAMEWORK_LINK target app_path)
  get_property(target_type TARGET ${target} PROPERTY TYPE)
  if(${target_type} STREQUAL "SHARED_LIBRARY")
    set(framework_path "${app_path}/${target}.framework/Versions/Current/${target}")
  else()
    set(framework_path "${app_path}/${target}.app/Contents/MacOS/${target}")
  endif()

  # embed in framework/app bundle
  add_custom_command(TARGET ${target}
    POST_BUILD
    COMMAND install_name_tool -change
      "@executable_path/Chromium Embedded Framework"
      "@executable_path/../../../../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework"
      "${framework_path}"
    VERBATIM
  )

endmacro()

# Fix the framework link in the main executable.
macro(FIX_MACOSX_MAIN_FRAMEWORK_LINK target app_path use_rpath)
  get_property(target_type TARGET ${target} PROPERTY TYPE)
  if(${target_type} STREQUAL "SHARED_LIBRARY")
    set(framework_path "${app_path}/${target}.framework/Versions/Current/${target}")
  else()
    set(framework_path "${app_path}/${target}.app/Contents/MacOS/${target}")
  endif()

  if(${use_rpath})
    set(install_path "@rpath/Chromium Embedded Framework.framework/Chromium Embedded Framework")
  else()
#    set(install_path "@executable_path/../Chromium Embedded Framework.framework/Chromium Embedded Framework")
    set(install_path "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework")
  endif()

  # embed in framework/app bundle
  add_custom_command(TARGET ${target}
    POST_BUILD
    COMMAND install_name_tool -change "@executable_path/Chromium Embedded Framework" "${install_path}" "${framework_path}"
    VERBATIM
  )
endmacro()

# Make the other helper app bundles.
macro(MAKE_MACOSX_HELPERS target app_path)
  add_custom_command(TARGET ${target}
    POST_BUILD
    # The exported variables need to be set for generators other than Xcode.
    COMMAND export BUILT_PRODUCTS_DIR=${app_path} &&
            export CONTENTS_FOLDER_PATH=${target}.app/Contents &&
            ${ZEPHYROS_DIR}/tools/make_more_helpers.sh "Frameworks" "${target}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    VERBATIM
    )
endmacro()

# Manually process and copy over resource files.
macro(COPY_MACOSX_RESOURCES resource_list prefix_list target source_dir app_path)
  foreach(FILENAME ${resource_list})
    # Remove one or more prefixes from the source paths.
    set(TARGET_FILENAME "${FILENAME}")
    foreach(PREFIX ${prefix_list})
      string(REGEX REPLACE "^.*${PREFIX}" "" TARGET_FILENAME ${TARGET_FILENAME})
    endforeach()

    # Determine the absolute source and target paths.
    set(TARGET_PATH "${app_path}/Contents/Resources/${TARGET_FILENAME}")
    if(IS_ABSOLUTE ${FILENAME})
      set(SOURCE_PATH ${FILENAME})
    else()
      set(SOURCE_PATH "${source_dir}/${FILENAME}")
    endif()

    if(${FILENAME} MATCHES ".xib$")
      # Change the target file extension.
      string(REGEX REPLACE ".xib$" ".nib" TARGET_PATH ${TARGET_PATH})

      get_filename_component(TARGET_DIRECTORY ${TARGET_PATH} PATH)
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        # Create the target directory.
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_DIRECTORY}"
        # Compile the XIB file to a NIB.
        COMMAND /usr/bin/ibtool --output-format binary1 --compile "${TARGET_PATH}" "${SOURCE_PATH}"
        VERBATIM
        )
    elseif(NOT ${TARGET_FILENAME} STREQUAL "Info.plist")
      # Copy the file as-is.
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE_PATH}" "${TARGET_PATH}"
        VERBATIM
        )
    endif()
  endforeach()
endmacro()

macro(EMBED_FRAMEWORK target framework)
  if(${CMAKE_GENERATOR} STREQUAL "Ninja")
    # Ninja does not create a subdirectory named after the configuration.
    set(outdir "${CMAKE_CURRENT_BINARY_DIR}")
  else()
    set(outdir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
  endif()

  get_filename_component(framework_name "${framework}" NAME)
  get_property(target_type TARGET ${target} PROPERTY TYPE)
  if(${target_type} STREQUAL "SHARED_LIBRARY")
    COPY_R(${target} "${framework}" "${outdir}/${target}.framework/Versions/Current/Frameworks/${framework_name}")
  else()
    COPY_R(${target} "${framework}" "${outdir}/${target}.app/Contents/Frameworks/${framework_name}")
  endif()
endmacro()

macro(EMBED_HELPERS target target_helper)
  if(${CMAKE_GENERATOR} STREQUAL "Ninja")
    # Ninja does not create a subdirectory named after the configuration.
    set(outdir "${CMAKE_CURRENT_BINARY_DIR}")
  else()
    set(outdir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
  endif()

  COPY_R(${target} "${outdir}/${target_helper}.app" "${outdir}/${target}.app/Contents/Frameworks/${target_helper}.app")
  MAKE_MACOSX_HELPERS(${target} ${outdir})
endmacro()

endif(OS_MACOSX)


#
# Windows macros.
#

if(OS_WINDOWS)

# Add custom manifest files to an executable target.
macro(ADD_WINDOWS_MANIFEST manifest_path target)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND "mt.exe" -nologo
            -manifest \"${manifest_path}/${target}.exe.manifest\" \"${manifest_path}/compatibility.manifest\"
            -outputresource:"${CEF_TARGET_OUT_DIR}/${target}.exe"\;\#1
    COMMENT "Adding manifest..." 
    )
endmacro()

endif(OS_WINDOWS)
