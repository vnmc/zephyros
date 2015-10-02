cmake_minimum_required(VERSION 2.8.12.2)

###############################################################################
# Setup / Bootstrapping
###############################################################################

# set the project name
set(APP_NAME App)

# the folder containing the webapp
set(WEBAPP app)

if(NOT(DEFINED IS_ZEPHYROS_BUILD))
  project(${APP_NAME})

  # have the user specify the Zephyros directory
  set(ZEPHYROS_DIR "" CACHE PATH "The Zephyros root directory (source or binary)")
  if("${ZEPHYROS_DIR}" STREQUAL "")
    message(FATAL_ERROR "Please set ZEPHYROS_DIR to the Zephyros directory (source or binary)")
  endif()

  set(ZEPHYROS_BUILD_DIR "" CACHE PATH "The directory containing the built Zephyros libraries")

  # include "macros.cmake" from Zephyros's root directory
  set(CMAKE_MODULE_PATH "${ZEPHYROS_DIR}")
  include("macros")

  set_property(GLOBAL PROPERTY OS_FOLDERS ON)
endif()


###############################################################################
# Source files
###############################################################################

# the native sources
set(APP_SRCS
  src/app.cpp
  src/stdafx.h
)

# app resources
set(APP_RESOURCES)
set(APP_RESOURCES_MACOSX
  res/mac/Base.lproj/MainMenu.xib
  res/mac/en.lproj/Localizable.strings
  res/mac/Images.xcassets
  ${WEBAPP}
)
set(APP_RESOURCES_WINDOWS
  res/windows/app.rc
  res/windows/content.rc
  res/windows/app.ico
  res/windows/small.ico
)
APPEND_PLATFORM_SOURCES(APP_RESOURCES)

# plist files for Mac
set(APP_CEF_PLIST res/mac/Info-CEF.plist)
set(APP_CEFHELPER_PLIST res/mac/Info-CEF-Helper.plist)
set(APP_WEBVIEW_PLIST res/mac/Info-WebView.plist)
set(APP_APPSTORE_PLIST res/mac/Info-AppStore.plist)

set(APP_ALL_SRCS
  ${APP_SRCS}
  ${APP_RESOURCES}
)


###############################################################################
# Configuration
###############################################################################

# on Mac, let the user choose the desired app target types (CEF, WebView, AppStore)
if(OS_MACOSX)
  option(BUILD_CEF_VERSION "Build a CEF version of the app" ON)
  option(BUILD_WEBVIEW_VERSION "Build a WebView version of the app" OFF)
  option(BUILD_APPSTORE_VERSION "Build a Mac AppStore version of the app" OFF)

  set(_sum_ 0)
  if(${BUILD_CEF_VERSION})
    math(EXPR _sum_ "${_sum_} + 1")
  endif()
  if(${BUILD_WEBVIEW_VERSION})
    math(EXPR _sum_ "${_sum_} + 1")
  endif()
  if(${BUILD_APPSTORE_VERSION})
    math(EXPR _sum_ "${_sum_} + 1")
  endif()

  # set the app names:
  # if only one target type is selected, use APP_NAME as app name,
  # otherwise append the target type
  if(${_sum_} EQUAL 0)
    message(FATAL_ERROR "You need to select at least one app type (BUILD_*_VERSION)")
  elseif(${_sum_} EQUAL 1)
    set(APP_NAME_CEF ${APP_NAME})
    set(APP_NAME_WEBVIEW ${APP_NAME})
    set(APP_NAME_APPSTORE ${APP_NAME})
  else()
    set(APP_NAME_CEF "${APP_NAME}_CEF")
    set(APP_NAME_WEBVIEW "${APP_NAME}_WebView")
    set(APP_NAME_APPSTORE "${APP_NAME}_AppStore")
  endif()
endif()

# on Windows, let the user choose if she wants to use WinSparkle and CrashRpt
if(OS_WINDOWS)
  option(USE_WINSPARKLE "Use the WinSparkle updater" ON)
  option(USE_CRASHRPT "Use the CrashRpt crash reporter" ON)
endif()


# set Zephyros libraries and library paths
if(DEFINED IS_ZEPHYROS_BUILD)
  # this CMake file is included by the CMake file in Zephyros's root directory

  # set the Zephyros libraries to the Zephyros targets
  if(OS_MACOSX)
    set(LIB_ZEPHYROS_CEF Zephyros_CEF)
    set(LIB_ZEPHYROS_CEFHELPER Zephyros_CEF_Helper)
    set(LIB_ZEPHYROS_WEBVIEW Zephyros_WebView)
    set(LIB_ZEPHYROS_APPSTORE Zephyros_AppStore)

    set(LIB_ZEPHYROS_CEF_PATH "${ZEPHYROS_BUILD_DIR}/${LIB_ZEPHYROS_CEF}.framework")
    set(LIB_ZEPHYROS_CEFHELPER_PATH "${ZEPHYROS_BUILD_DIR}/${LIB_ZEPHYROS_CEFHELPER}.framework")
    set(LIB_ZEPHYROS_WEBVIEW_PATH "${ZEPHYROS_BUILD_DIR}/${LIB_ZEPHYROS_WEBVIEW}.framework")
    set(LIB_ZEPHYROS_APPSTORE_PATH "${ZEPHYROS_BUILD_DIR}/${LIB_ZEPHYROS_APPSTORE}.framework")
  else()
    set(LIB_ZEPHYROS_CEF Zephyros)
    set(LIB_DLL_WRAPPER libcef_dll_wrapper)
  endif()
else()
  # this CMake file is a used as a standalone

  # find the Zephyros libraries
  if(OS_MACOSX)
    find_library(LIB_ZEPHYROS_CEF Zephyros_CEF PATHS ${ZEPHYROS_BUILD_DIR})
    find_library(LIB_ZEPHYROS_CEFHELPER Zephyros_CEF_Helper PATHS ${ZEPHYROS_BUILD_DIR})
    find_library(LIB_ZEPHYROS_WEBVIEW Zephyros_WebView PATHS ${ZEPHYROS_BUILD_DIR})
    find_library(LIB_ZEPHYROS_APPSTORE Zephyros_AppStore PATHS ${ZEPHYROS_BUILD_DIR})

    set(LIB_ZEPHYROS_CEF_PATH "${LIB_ZEPHYROS_CEF}")
    set(LIB_ZEPHYROS_CEFHELPER_PATH "${LIB_ZEPHYROS_CEFHELPER}")
    set(LIB_ZEPHYROS_WEBVIEW_PATH "${LIB_ZEPHYROS_WEBVIEW}")
    set(LIB_ZEPHYROS_APPSTORE_PATH "${LIB_ZEPHYROS_APPSTORE}")
  elseif(OS_WINDOWS)
    set(LIB_ZEPHYROS_CEF "${ZEPHYROS_BUILD_DIR}/Zephyros.lib")
    set(LIB_DLL_WRAPPER "${ZEPHYROS_BUILD_DIR}/libcef_dll_wrapper.lib")
  elseif(OS_LINUX)
    set(LIB_ZEPHYROS_CEF "${ZEPHYROS_BUILD_DIR}/Zephyros.a")
    set(LIB_DLL_WRAPPER "${ZEPHYROS_BUILD_DIR}/libcef_dll_wrapper.a")
  endif()
endif()


# find libraries
if(OS_MACOSX)
  find_library(LIB_APPKIT AppKit)
  find_library(LIB_CEF "Chromium Embedded Framework" PATHS ${ZEPHYROS_DIR}/lib/cef/DLL/libcef/mac)
endif()

SET_CEF_TARGET_OUT_DIR()


# configure the executables
if(OS_MACOSX)
  # CEF version
  if(${BUILD_CEF_VERSION})
    # CEF Helper App
    set(APP_NAME_CEFHELPER "${APP_NAME_CEF} Helper")
    add_executable(${APP_NAME_CEFHELPER} MACOSX_BUNDLE ${APP_SRCS})

    if(DEFINED IS_ZEPHYROS_BUILD)
      add_dependencies(${APP_NAME_CEFHELPER} Zephyros_CEF_Helper)
      target_include_directories(${APP_NAME_CEFHELPER} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
    else()
      target_include_directories(${APP_NAME_CEFHELPER} PRIVATE ${CMAKE_SOURCE_DIR}/src ${LIB_ZEPHYROS_CEFHELPER_PATH}/Headers ${ZEPHYROS_DIR} ${ZEPHYROS_DIR}/lib/cef)
    endif()

    target_compile_definitions(${APP_NAME_CEFHELPER} PRIVATE -DUSE_CEF ${PLATFORM})
    target_link_libraries(${APP_NAME_CEFHELPER} PRIVATE ${LIB_APPKIT} ${LIB_ZEPHYROS_CEFHELPER} ${LIB_CEF})
    set_target_properties(${APP_NAME_CEFHELPER} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/${APP_CEFHELPER_PLIST}"
    )
    EMBED_FRAMEWORK("${APP_NAME_CEFHELPER}" "${LIB_ZEPHYROS_CEFHELPER_PATH}")
    FIX_MACOSX_HELPER_FRAMEWORK_LINK("${APP_NAME_CEFHELPER}" "${CEF_TARGET_OUT_DIR}")

    # CEF App
    add_executable(${APP_NAME_CEF} MACOSX_BUNDLE ${APP_ALL_SRCS})

    if(DEFINED IS_ZEPHYROS_BUILD)
      add_dependencies(${APP_NAME_CEF} "${APP_NAME_CEFHELPER}" Zephyros_CEF)
      target_include_directories(${APP_NAME_CEF} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
    else()
      add_dependencies(${APP_NAME_CEF} "${APP_NAME_CEFHELPER}")
      target_include_directories(${APP_NAME_CEF} PRIVATE ${CMAKE_SOURCE_DIR}/src ${LIB_ZEPHYROS_CEF_PATH}/Headers ${ZEPHYROS_DIR} ${ZEPHYROS_DIR}/lib/cef)
    endif()

    target_compile_definitions(${APP_NAME_CEF} PRIVATE -DUSE_CEF ${PLATFORM})
    target_link_libraries(${APP_NAME_CEF} PRIVATE ${LIB_APPKIT} ${LIB_ZEPHYROS_CEF} ${LIB_CEF})

    set_target_properties(${APP_NAME_CEF} PROPERTIES
      RESOURCE "${APP_RESOURCES}"
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/${APP_CEF_PLIST}"
    )
    EMBED_FRAMEWORK("${APP_NAME_CEF}" "${LIB_ZEPHYROS_CEF_PATH}")
    EMBED_FRAMEWORK("${APP_NAME_CEF}" "${LIB_CEF}")
    EMBED_HELPERS("${APP_NAME_CEF}" "${APP_NAME_CEFHELPER}")
    FIX_MACOSX_MAIN_FRAMEWORK_LINK("${APP_NAME_CEF}" "${CEF_TARGET_OUT_DIR}" NO)
  endif()

  # WebView App
  if(${BUILD_WEBVIEW_VERSION})
    add_executable(${APP_NAME_WEBVIEW} MACOSX_BUNDLE ${APP_ALL_SRCS})

    if(DEFINED IS_ZEPHYROS_BUILD)
      add_dependencies(${APP_NAME_WEBVIEW} Zephyros_WebView)
      target_include_directories(${APP_NAME_WEBVIEW} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/base/webview)
    else()
      target_include_directories(${APP_NAME_WEBVIEW} PRIVATE ${CMAKE_SOURCE_DIR}/src ${LIB_ZEPHYROS_WEBVIEW_PATH}/Headers)
    endif()

    target_compile_definitions(${APP_NAME_WEBVIEW} PRIVATE -DUSE_WEBVIEW ${PLATFORM})
    target_link_libraries(${APP_NAME_WEBVIEW} PRIVATE ${LIB_APPKIT} ${LIB_ZEPHYROS_WEBVIEW})

    set_target_properties(${APP_NAME_WEBVIEW} PROPERTIES
      RESOURCE "${APP_RESOURCES}"
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/${APP_WEBVIEW_PLIST}"
    )
    EMBED_FRAMEWORK("${APP_NAME_WEBVIEW}" "${LIB_ZEPHYROS_WEBVIEW_PATH}")
  endif()

  # AppStore App
  if(${BUILD_APPSTORE_VERSION})
    add_executable(${APP_NAME_APPSTORE} MACOSX_BUNDLE ${APP_ALL_SRCS})

    if(DEFINED IS_ZEPHYROS_BUILD)
      add_dependencies(${APP_NAME_APPSTORE} Zephyros_AppStore)
      target_include_directories(${APP_NAME_APPSTORE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/base/webview)
    else()
      target_include_directories(${APP_NAME_APPSTORE} PRIVATE ${CMAKE_SOURCE_DIR}/src ${LIB_ZEPHYROS_APPSTORE_PATH}/Headers)
    endif()

    target_compile_definitions(${APP_NAME_APPSTORE} PRIVATE -DUSE_WEBVIEW -DAPPSTORE ${PLATFORM})
    target_link_libraries(${APP_NAME_APPSTORE} PRIVATE ${LIB_APPKIT} ${LIB_ZEPHYROS_APPSTORE})

    set_target_properties(${APP_NAME_APPSTORE} PROPERTIES
      RESOURCE "${APP_RESOURCES}"
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/${APP_APPSTORE_PLIST}"
    )
    EMBED_FRAMEWORK("${APP_NAME_APPSTORE}" "${LIB_ZEPHYROS_APPSTORE_PATH}")
  endif()
elseif(OS_WINDOWS)
  add_executable(${APP_NAME} WIN32 ${APP_ALL_SRCS})

  if(DEFINED IS_ZEPHYROS_BUILD)
    add_dependencies(${APP_NAME} Zephyros)
    target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
  else()
    target_include_directories(${APP_NAME} PRIVATE ${ZEPHYROS_DIR} ${ZEPHYROS_DIR}/src ${ZEPHYROS_DIR}/lib/cef)
  endif()

  set(CEF_STANDARD_LIBS
    winhttp.lib
#    dnsapi.lib
    version.lib
#    msimg32.lib
    ws2_32.lib
#    usp10.lib
#    psapi.lib
#    dbghelp.lib
#    winmm.lib
    shlwapi.lib
    kernel32.lib
    gdi32.lib
#    winspool.lib
    comdlg32.lib
#    advapi32.lib
    shell32.lib
#    ole32.lib
#    oleaut32.lib
    user32.lib
#    uuid.lib
#    odbc32.lib
#    odbccp32.lib
#    delayimp.lib
#    comctl32.lib
#    rpcrt4.lib
#    opengl32.lib
#    glu32.lib
    gdiplus.lib
    iphlpapi.lib
    crypt32.lib
    uxtheme.lib
  )

  if(USE_WINSPARKLE)
    set(CEF_STANDARD_LIBS ${CEF_STANDARD_LIBS} "${ZEPHYROS_DIR}/lib/winsparkle/\$(Platform)/WinSparkle.lib")
    COPY_FILES(${APP_NAME} ${WINSPARKLE_BINARY_FILES} "${ZEPHYROS_DIR}/lib/winsparkle/x64" "${CEF_TARGET_OUT_DIR}")
  else()
    set(APP_ALL_SRCS ${APP_ALL_SRCS} "${ZEPHYROS_DIR}/src/stubs_win.cpp")
    target_compile_definitions(${APP_NAME} PRIVATE -DNO_WINSPARKLE)
  endif()

  if(USE_CRASHRPT)
    set(CEF_STANDARD_LIBS ${CEF_STANDARD_LIBS} "${ZEPHYROS_DIR}/lib/CrashRpt/\$(Platform)/CrashRpt1402.lib")
    COPY_FILES(${APP_NAME} ${CRASHREPORTER_BINARY_FILES} "${ZEPHYROS_DIR}/lib/CrashRpt/x64" "${CEF_TARGET_OUT_DIR}")
  else()
    set(APP_ALL_SRCS ${APP_ALL_SRCS} "${ZEPHYROS_DIR}/src/stubs_win.cpp")
    target_compile_definitions(${APP_NAME} PRIVATE -DNO_CRASHRPT)
  endif()

  set(CEF_RESOURCE_DIR "${ZEPHYROS_DIR}/lib/cef/DLL/libcef/windows/x64/Resources/")
  set(CEF_BINARY_DIR "${ZEPHYROS_DIR}/lib/cef/DLL/libcef/windows/x64/Release/")

  target_compile_definitions(${APP_NAME} PRIVATE -DUSE_CEF ${PLATFORM})
  target_link_libraries(${APP_NAME} PRIVATE ${LIB_ZEPHYROS_CEF} ${LIB_DLL_WRAPPER} ${CEF_BINARY_DIR}/libcef.lib ${CEF_STANDARD_LIBS})

  # add the custom manifest files to the executable
#  ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}/resources/win" "${CEF_TARGET}")

  # copy CEF binary and resource files to the target output directory
  COPY_FILES(${APP_NAME} "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES(${APP_NAME} "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

elseif(OS_LINUX)
  # find required libraries and update compiler/linker variables
  set(CEF_STANDARD_LIBS X11)
  FIND_LINUX_LIBRARIES("gmodule-2.0 gtk+-3.0 gthread-2.0 openssl libcurl")

  set(CEF_RESOURCE_DIR "${ZEPHYROS_DIR}/lib/cef/DLL/libcef/linux/x86_64/Resources/")
  set(CEF_BINARY_DIR "${ZEPHYROS_DIR}/lib/cef/DLL/libcef/linux/x86_64/Release/")

  add_executable(${APP_NAME} ${APP_ALL_SRCS})

  if(DEFINED IS_ZEPHYROS_BUILD)
    add_dependencies(${APP_NAME} Zephyros)
    target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
    target_link_libraries(${APP_NAME} PRIVATE Zephyros libcef_dll_wrapper ${CEF_BINARY_DIR}/libcef.so ${CEF_STANDARD_LIBS})
  else()
    target_include_directories(${APP_NAME} PRIVATE ${ZEPHYROS_DIR} ${ZEPHYROS_DIR}/src ${ZEPHYROS_DIR}/lib/cef)
  endif()

  target_link_libraries(${APP_NAME} PRIVATE ${LIB_ZEPHYROS_CEF} ${LIB_DLL_WRAPPER} ${CEF_BINARY_DIR}/libcef.so ${CEF_STANDARD_LIBS})
  target_compile_definitions(${APP_NAME} PRIVATE -DUSE_CEF ${PLATFORM})

  # set rpath so that libraries can be placed next to the executable
  set_target_properties(${APP_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR}
  )

  # copy CEF binary and resource files to the target output directory
  COPY_FILES(${APP_NAME} "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES(${APP_NAME} "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

  # copy cefclient resource files to the target output directory
  COPY_FILES(${APP_NAME} "${WEBAPP}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()