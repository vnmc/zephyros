###############################################################################
# Source files
###############################################################################

# the folder containing the webapp
set(WEBAPP app)

# the native sources
set(APP_SRCS
  src/app.cpp
  src/stdafx.h
)
#APPEND_PLATFORM_SOURCES(APP_SRCS)

# app resources
set(APP_RESOURCES)
set(APP_RESOURCES_MACOSX
  res/mac/Base.lproj/MainMenu.xib
  res/mac/en.lproj/Localizable.strings
  res/mac/Images.xcassets
  ${WEBAPP}
)
APPEND_PLATFORM_SOURCES(APP_RESOURCES)

set(APP_SRCS
  ${APP_SRCS}
  ${APP_RESOURCES}
  ${APP_WEBAPP}
)


###############################################################################
# Configuration
###############################################################################

if(OS_WINDOWS)
  set(PLATFORM -DOS_WIN)
elseif(OS_MACOSX)
  set(PLATFORM -DOS_MACOSX)
elseif(OS_LINUX)
  set(PLATFORM -DOS_LINUX)
endif()

#message(STATUS "---- ${DIR_ZEPHYROS_BIN}")


if(OS_MACOSX)
  find_library(LIB_APPKIT AppKit)

#  find_library(LIB_ZEPHYROS_CEF Zephyros_CEF PATHS ${DIR_ZEPHYROS_BIN})
#  find_library(LIB_ZEPHYROS_CEFHELPER Zephyros_CEF_Helper PATHS ${DIR_ZEPHYROS_BIN})

#  find_library(LIB_ZEPHYROS_WEBVIEW Zephyros_WebView PATHS ${DIR_ZEPHYROS_BIN})
  find_library(LIB_ZEPHYROS_WEBVIEW Zephyros_WebView PATHS /Users/christen/Projects/zephyros/build/src/Debug)
#  find_library(LIB_ZEPHYROS_WEBVIEW Zephyros_WebView PATHS "/Users/christen/Library/Developer/Xcode/DerivedData/Zephyros-cphdxfikckfikiesmgfffoplkisk/Build/Products/Debug")

#  find_library(LIB_ZEPHYROS_APPSTORE Zephyros_AppStore PATHS ${DIR_ZEPHYROS_BIN})

  find_library(LIB_CEF "Chromium Embedded Framework" PATHS ${DIR_ZEPHYROS_LIB})
endif()

set(LIB_ZEPHYROS_CEF Zephyros_CEF)
set(LIB_ZEPHYROS_CEFHELPER "Zephyros_CEF_Helper")

SET_CEF_TARGET_OUT_DIR()


if(OS_MACOSX)
  # CEF Helper App
  add_executable("App_CEF Helper" MACOSX_BUNDLE ${APP_SRCS} ../../src/base/cef/process_helper_mac.mm)
  add_dependencies("App_CEF Helper" Zephyros_CEF_Helper)
  target_include_directories("App_CEF Helper" PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
  target_compile_definitions("App_CEF Helper" PRIVATE -DUSE_CEF ${PLATFORM})
  target_link_libraries("App_CEF Helper" PRIVATE ${LIB_APPKIT} ${LIB_ZEPHYROS_CEFHELPER} ${LIB_CEF})
  set_target_properties("App_CEF Helper" PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/mac/Info-CEF-Helper.plist
  )
  EMBED_FRAMEWORK("App_CEF Helper" ${DIR_ZEPHYROS_BIN}/Zephyros_CEF_Helper.framework)
  FIX_MACOSX_HELPER_FRAMEWORK_LINK("App_CEF Helper" ${CEF_TARGET_OUT_DIR})

  # CEF App
  add_executable(App_CEF MACOSX_BUNDLE ${APP_SRCS})
  #target_include_directories(App_CEF PRIVATE ${DIR_ZEPHYROS})
  add_dependencies(App_CEF "App_CEF Helper" Zephyros_CEF)
  target_include_directories(App_CEF PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
  target_compile_definitions(App_CEF PRIVATE -DUSE_CEF ${PLATFORM})
  target_link_libraries(App_CEF PRIVATE ${LIB_APPKIT} ${LIB_ZEPHYROS_CEF} ${LIB_CEF})
  set_target_properties(App_CEF PROPERTIES
    RESOURCE "${APP_RESOURCES}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/mac/Info-CEF.plist
  )
  EMBED_FRAMEWORK(App_CEF ${DIR_ZEPHYROS_BIN}/Zephyros_CEF.framework)
  EMBED_FRAMEWORK(App_CEF ${LIB_CEF})
  EMBED_HELPERS(App_CEF "App_CEF Helper")
  FIX_MACOSX_MAIN_FRAMEWORK_LINK(App_CEF ${CEF_TARGET_OUT_DIR})

  # WebView App
  add_executable(App_WebView MACOSX_BUNDLE ${APP_SRCS})
  add_dependencies(App_WebView Zephyros_WebView)
  target_include_directories(App_WebView PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/base/webview)
  target_compile_definitions(App_WebView PUBLIC -DUSE_WEBVIEW ${PLATFORM})
  target_link_libraries(App_WebView PUBLIC ${LIB_APPKIT} Zephyros_WebView)
  set_target_properties(App_WebView PROPERTIES
    RESOURCE "${APP_RESOURCES}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/mac/Info-WebView.plist
  )
  EMBED_FRAMEWORK(App_WebView ${DIR_ZEPHYROS_BIN}/Zephyros_WebView.framework)

  # AppStore App
  add_executable(App_AppStore MACOSX_BUNDLE ${APP_SRCS})
  add_dependencies(App_AppStore Zephyros_AppStore)
  target_include_directories(App_AppStore PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/base/webview)
  target_compile_definitions(App_AppStore PUBLIC -DUSE_WEBVIEW -DAPPSTORE ${PLATFORM})
  target_link_libraries(App_AppStore PUBLIC ${LIB_APPKIT} Zephyros_AppStore)
  set_target_properties(App_AppStore PROPERTIES
    RESOURCE "${APP_RESOURCES}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/mac/Info-AppStore.plist
  )
  EMBED_FRAMEWORK(App_AppStore ${DIR_ZEPHYROS_BIN}/Zephyros_AppStore.framework)
elseif(OS_WINDOWS)
elseif(OS_LINUX)
  # find required libraries and update compiler/linker variables
  set(CEF_STANDARD_LIBS X11)
  FIND_LINUX_LIBRARIES("gmodule-2.0 gtk+-3.0 gthread-2.0")

  set(CEF_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/cef/DLL/libcef/linux-x86_64/Resources/")
  set(CEF_BINARY_DIR "${CMAKE_SOURCE_DIR}/lib/cef/DLL/libcef/linux-x86_64/Release/")

  add_executable(App ${APP_SRCS})
  add_dependencies(App Zephyros)
  target_include_directories(App PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
  target_compile_definitions(App PRIVATE -DUSE_CEF ${PLATFORM})
  target_link_libraries(App PRIVATE Zephyros libcef_dll_wrapper ${CEF_BINARY_DIR}/libcef.so ${CEF_STANDARD_LIBS})

  # set rpath so that libraries can be placed next to the executable
  set_target_properties(App PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR}
  )


  # List of CEF binary files.
  set(CEF_BINARY_FILES
    libcef.so
    natives_blob.bin
    snapshot_blob.bin
  )

  # List of CEF resource files.
  set(CEF_RESOURCE_FILES
    cef.pak
    cef_100_percent.pak
    cef_200_percent.pak
    cef_extensions.pak
    devtools_resources.pak
    icudtl.dat
    locales
  )

  # copy CEF binary and resource files to the target output directory
  COPY_FILES(App "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES(App "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

  # copy cefclient resource files to the target output directory
  COPY_FILES(App "${WEBAPP}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()
