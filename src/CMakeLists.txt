###############################################################################
# Source files
###############################################################################

# Zephyros common sources

# - public headers
set(ZEPHYROS__SRCS
	zephyros.h
	native_extensions.h
	jsbridge.h
	zephyros_strings.h
	licensing.h
)
source_group(Public\ Headers FILES ${ZEPHYROS__SRCS})

set(ZEPHYROS__PUBLICHEADERS_SRCS
	${ZEPHYROS__SRCS}
	base/ZPYAppDelegate.h
	components/ZPYMenuItem.h
)

# - base
set(ZEPHYROS__BASE_SRCS
	base/zephyros_impl.cpp
	base/app.h
	base/app.cpp
	base/jsbridge.cpp
	base/logging.h
	base/types.h
)
set(ZEPHYROS__BASE_SRCS_MACOSX
	base/app_mac.mm
	base/ZPYAppDelegate.h
	base/ZPYAppDelegate.mm
	base/ZPYMenuHandler.h
	base/ZPYMenuHandler.mm
	base/ZPYTouchBarHandler.h
	base/ZPYTouchBarHandler.mm
)
set(ZEPHYROS__BASE_SRCS_WINDOWS
	base/app_win.cpp
)
set(ZEPHYROS__BASE_SRCS_LINUX
	base/app_linux.cpp
)
APPEND_PLATFORM_SOURCES(ZEPHYROS__BASE_SRCS)
source_group(Base FILES ${ZEPHYROS__BASE_SRCS})

# - licensing (base)
set(ZEPHYROS__BASE_LICENSING_SRCS
	base/licensing.cpp
)
set(ZEPHYROS__BASE_LICENSING_SRCS_MACOSX
	base/licensing_mac.mm
)
set(ZEPHYROS__BASE_LICENSING_SRCS_WINDOWS
	base/licensing_win.cpp
)
set(ZEPHYROS__BASE_LICENSING_SRCS_LINUX
	base/licensing_linux.cpp
)
APPEND_PLATFORM_SOURCES(ZEPHYROS__BASE_LICENSING_SRCS)
source_group(Base FILES ${ZEPHYROS__BASE_LICENSING_SRCS})

# - native extensions
set(ZEPHYROS__NATIVEEXT_SRCS
	native_extensions/native_extensions.cpp
	native_extensions/browser.cpp
	native_extensions/browser.h
	native_extensions/custom_url_manager.cpp
	native_extensions/custom_url_manager.h
	native_extensions/error.cpp
	native_extensions/error.h
	native_extensions/file_util.h
	native_extensions/file_watcher.cpp
	native_extensions/file_watcher.h
	native_extensions/network_util.h
	native_extensions/os_util.h
	native_extensions/pageimage.h
	native_extensions/path.cpp
	native_extensions/path.h
	native_extensions/updater.h
)
set(ZEPHYROS__NATIVEEXT_SRCS_MACOSX
	native_extensions/browser_mac.mm
	native_extensions/error_mac.mm
	native_extensions/file_util_mac.mm
	native_extensions/file_watcher_mac.mm
	native_extensions/image_util_mac.h
	native_extensions/image_util_mac.mm
	native_extensions/network_util_mac.mm
)
set(ZEPHYROS__NATIVEEXT_SRCS_WINDOWS
	native_extensions/browser_win.cpp
	native_extensions/error_win.cpp
	native_extensions/file_util_win.cpp
	native_extensions/file_watcher_win.cpp
	native_extensions/image_util_win.h
	native_extensions/image_util_win.cpp
	native_extensions/network_util_win.cpp
	native_extensions/os_util_win.cpp
	native_extensions/pageimage_win.cpp
	native_extensions/process_manager.h # TODO: rename to win
	native_extensions/process_manager_win.cpp
	native_extensions/updater_win.cpp
)
set(ZEPHYROS__NATIVEEXT_SRCS_LINUX
	native_extensions/browser_linux.cpp
	native_extensions/error_linux.cpp
	native_extensions/file_util_linux.cpp
	native_extensions/file_watcher_linux.cpp
	native_extensions/image_util_linux.h
	native_extensions/image_util_linux.cpp
	native_extensions/network_util_linux.cpp
	native_extensions/os_util_linux.cpp
	native_extensions/pageimage_linux.cpp
	native_extensions/updater_linux.cpp
)
set(ZEPHYROS__NATIVEEXT_IMPL_SRCS)
set(ZEPHYROS__NATIVEEXT_STUB_SRCS)
set(ZEPHYROS__NATIVEEXT_IMPL_SRCS_MACOSX
	native_extensions/os_util_mac.mm
	native_extensions/pageimage_mac.mm
	native_extensions/updater_mac.mm
)
set(ZEPHYROS__NATIVEEXT_STUB_SRCS_MACOSX
	native_extensions/os_util_mac_stub.mm
	native_extensions/pageimage_mac_stub.mm
	native_extensions/updater_mac_stub.mm
)
APPEND_PLATFORM_SOURCES(ZEPHYROS__NATIVEEXT_SRCS)
APPEND_PLATFORM_SOURCES(ZEPHYROS__NATIVEEXT_IMPL_SRCS)
APPEND_PLATFORM_SOURCES(ZEPHYROS__NATIVEEXT_STUB_SRCS)
source_group(Native\ Extensions FILES
	${ZEPHYROS__NATIVEEXT_SRCS}
	${ZEPHYROS__NATIVEEXT_IMPL_SRCS}
	${ZEPHYROS__NATIVEEXT_STUB_SRCS}
)

# - components
set(ZEPHYROS__COMPONENTS_SRCS)
set(ZEPHYROS__COMPONENTS_SRCS_MACOSX
	components/ZPYMenuItem.h
	components/ZPYMenuItem.mm
)
set(ZEPHYROS__COMPONENTS_SRCS_WINDOWS
	components/dialog_win.cpp
	components/dialog_win.h
)
set(ZEPHYROS__COMPONENTS_SRCS_LINUX
)
APPEND_PLATFORM_SOURCES(ZEPHYROS__COMPONENTS_SRCS)
source_group(Components FILES ${ZEPHYROS__COMPONENTS_SRCS})

# - licensing (components)
set(ZEPHYROS__COMPONENTS_LICENSING_SRCS)
set(ZEPHYROS__COMPONENTS_LICENSING_SRCS_MACOSX
	components/LicenseCheckWindowController.h
	components/LicenseCheckWindowController.mm
	res/mac/LicenseCheckWindowController.xib
)
APPEND_PLATFORM_SOURCES(ZEPHYROS__COMPONENTS_LICENSING_SRCS)
source_group(Components FILES ${ZEPHYROS__COMPONENTS_LICENSING_SRCS})

# - utilities
set(ZEPHYROS__UTILITIES_SRCS
	util/string_util.cpp
	util/string_util.h
	util/picojson.h
	util/base32.cpp
	util/base32.h
	util/base64.cpp
	util/base64.h
	util/MurmurHash3.cpp
	util/MurmurHash3.h
)
set(ZEPHYROS__UTILITIES_SRCS_WINDOWS
	util/dataobject.cpp
	util/dataobject.h
	util/dropsource.cpp
	util/dropsource.h
	util/enumformat.cpp
	util/enumformat.h
)
set(ZEPHYROS__UTILITIES_SRCS_MACOSX
	util/EMKeychainItem.h
	util/EMKeychainItem.m
	util/GetPrimaryMACAddress.cpp
	util/GetPrimaryMACAddress.h
	util/NSData+Base64.h
	util/NSData+Base64.mm
)
APPEND_PLATFORM_SOURCES(ZEPHYROS__UTILITIES_SRCS)
source_group(Utilities FILES ${ZEPHYROS__UTILITIES_SRCS})


set(ZEPHYROS_COMMON_SRCS
	${ZEPHYROS__SRCS}
	${ZEPHYROS__BASE_SRCS}
	${ZEPHYROS__NATIVEEXT_SRCS}
	${ZEPHYROS__COMPONENTS_SRCS}
	${ZEPHYROS__UTILITIES_SRCS}
)

# Zephyros CEF sources
set(ZEPHYROS_CEF_BASE_SRCS
	base/cef/cef_app.cpp
	base/cef/client_app.cpp
	base/cef/client_app.h
	base/cef/client_handler.cpp
	base/cef/client_handler.h
	base/cef/extension_handler.cpp
	base/cef/extension_handler.h
	base/cef/local_scheme_handler.cpp
	base/cef/local_scheme_handler.h
	base/cef/mime_types.cpp
	base/cef/mime_types.h
	base/cef/resource_util.h
	base/cef/v8_util.cpp
	base/cef/v8_util.h
	jsbridge_v8.h
)
set(ZEPHYROS_CEF_BASE_SRCS_MACOSX
	base/cef/resource_util_mac.mm
)
set(ZEPHYROS_CEF_BASE_SRCS_WINDOWS
	base/cef/resource_util_win.cpp
)
set(ZEPHYROS_CEF_BASE_SRCS_LINUX
	base/cef/resource_util_linux.cpp
)
APPEND_PLATFORM_SOURCES(ZEPHYROS_CEF_BASE_SRCS)
source_group(Base FILES ${ZEPHYROS_CEF_BASE_SRCS})

set(ZEPHYROS_CEF_BASE_IMPL_SRCS)
set(ZEPHYROS_CEF_BASE_IMPL_SRCS_MACOSX
	base/cef/zephyros_cef_mac.mm
	base/cef/client_handler_mac.mm
	base/cef/ZPYCEFAppDelegate.h
	base/cef/ZPYCEFAppDelegate.mm
	base/cef/ZPYWindowDelegate.h
	base/cef/ZPYWindowDelegate.mm
)
set(ZEPHYROS_CEF_BASE_IMPL_SRCS_WINDOWS
	base/cef/zephyros_cef_win.h
	base/cef/zephyros_cef_win.cpp
	base/cef/client_handler_win.cpp
)
set(ZEPHYROS_CEF_BASE_IMPL_SRCS_LINUX
	base/cef/zephyros_cef_linux.cpp
	base/cef/client_handler_linux.cpp
)
APPEND_PLATFORM_SOURCES(ZEPHYROS_CEF_BASE_IMPL_SRCS)
source_group(Base FILES ${ZEPHYROS_CEF_BASE_IMPL_SRCS})

set(ZEPHYROS_CEF_SRCS
	${ZEPHYROS_COMMON_SRCS}
	${ZEPHYROS__BASE_LICENSING_SRCS}
	${ZEPHYROS__NATIVEEXT_IMPL_SRCS}
	${ZEPHYROS__COMPONENTS_LICENSING_SRCS}
	${ZEPHYROS_CEF_BASE_SRCS}
	${ZEPHYROS_CEF_BASE_IMPL_SRCS}
)

# Zephyros CEF Helper sources
set(ZEPHYROS_CEFHELPER_BASE_SRCS
	base/cef/zephyros_cef_mac_stub.mm
	base/cef/client_handler_mac_stub.mm
	base/licensing_stub.cpp
)
source_group(Base FILES ${ZEPHYROS_CEFHELPER_BASE_SRCS})
set(ZEPHYROS_CEFHELPER_SRCS
	${ZEPHYROS_COMMON_SRCS}
	${ZEPHYROS__NATIVEEXT_STUB_SRCS}
	${ZEPHYROS_CEF_BASE_SRCS}
	${ZEPHYROS_CEFHELPER_BASE_SRCS}
)

# Zephyros WebView sources
set(ZEPHYROS_WEBVIEW_BASE_SRCS
	base/webview/zephyros_webview.mm
	jsbridge_webview.h
	base/webview/jsbridge_webview.cpp
	base/webview/webview_extension.mm
	base/webview/ZPYWebViewAppDelegate.h
	base/webview/ZPYWebViewAppDelegate.mm
	res/mac/MainWindow.xib
)
source_group(Base FILES ${ZEPHYROS_WEBVIEW_BASE_SRCS})
set(ZEPHYROS_WEBVIEW_COMPONENTS_SRCS
	components/ZPYWebView.h
	components/ZPYWebView.mm
)
source_group(Components FILES ${ZEPHYROS_WEBVIEW_COMPONENTS_SRCS})
set(ZEPHYROS_WEBVIEW_SRCS
	${ZEPHYROS_COMMON_SRCS}
	${ZEPHYROS__BASE_LICENSING_SRCS}
	${ZEPHYROS__NATIVEEXT_IMPL_SRCS}
	${ZEPHYROS__COMPONENTS_LICENSING_SRCS}
	${ZEPHYROS_WEBVIEW_BASE_SRCS}
	${ZEPHYROS_WEBVIEW_COMPONENTS_SRCS}
)

# Zephyros AppStore sources
set(ZEPHYROS_APPSTORE_SRCS
	${ZEPHYROS_COMMON_SRCS}
	${ZEPHYROS__NATIVEEXT_IMPL_SRCS}
	${ZEPHYROS_WEBVIEW_BASE_SRCS}
	${ZEPHYROS_WEBVIEW_COMPONENTS_SRCS}
)


###############################################################################
# Configuration
###############################################################################

if(OS_MACOSX)
	set(CMAKE_MACOSX_RPATH 1)

	find_library(LIB_APPKIT AppKit)
	find_library(LIB_COCOA Cocoa)
	find_library(LIB_WEBKIT WebKit)
	find_library(LIB_JAVASCRIPTCORE JavaScriptCore)
	find_library(LIB_IOKIT IOKit)
	find_library(LIB_SYSTEMCONFIGURATION SystemConfiguration)
	find_library(LIB_SECURITY Security)
	find_library(LIB_SPARKLE Sparkle PATHS ${CMAKE_SOURCE_DIR}/lib)
	find_library(LIB_CEF "Chromium Embedded Framework" PATHS ${CMAKE_SOURCE_DIR}/lib/cef/DLL/libcef/mac)

	set(DEFAULT_LIBS ${LIB_APPKIT} ${LIB_COCOA} ${LIB_WEBKIT} ${LIB_IOKIT} ${LIB_SYSTEMCONFIGURATION} ${LIB_SECURITY})
endif()

SET_CEF_TARGET_OUT_DIR()
set(ZEPHYROS_BUILD_DIR ${CEF_TARGET_OUT_DIR} PARENT_SCOPE)

if(OS_MACOSX)
	# CEF configuration
	add_library(Zephyros_CEF SHARED ${ZEPHYROS_CEF_SRCS})
	add_dependencies(Zephyros_CEF libcef_dll_wrapper)
	target_include_directories(Zephyros_CEF PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
	target_compile_definitions(Zephyros_CEF PRIVATE -DUSE_CEF ${PLATFORM})
	target_link_libraries(Zephyros_CEF PUBLIC ${DEFAULT_LIBS} ${LIB_SPARKLE} ${LIB_CEF} libcef_dll_wrapper)
	#set_source_files_properties()
	#set_property(SOURCE zephyros.h
	#  PROPERTY MACOSX_PACKAGE_LOCATION Headers
	#  )
	set_target_properties(Zephyros_CEF PROPERTIES
		FRAMEWORK TRUE
		PUBLIC_HEADER "${ZEPHYROS__PUBLICHEADERS_SRCS};jsbridge_v8.h"
		RESOURCE "res/mac/LicenseCheckWindowController.xib"
		INSTALL_NAME_DIR "@rpath"
	)
	EMBED_FRAMEWORK(Zephyros_CEF ${LIB_SPARKLE})

	# CEF Helper configuration
	add_library(Zephyros_CEF_Helper SHARED ${ZEPHYROS_CEFHELPER_SRCS})
	add_dependencies(Zephyros_CEF_Helper libcef_dll_wrapper)
	target_include_directories(Zephyros_CEF_Helper PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
	target_compile_definitions(Zephyros_CEF_Helper PRIVATE -DUSE_CEF ${PLATFORM})
	target_link_libraries(Zephyros_CEF_Helper PUBLIC ${DEFAULT_LIBS} ${LIB_SPARKLE} ${LIB_CEF} libcef_dll_wrapper)
	set_target_properties(Zephyros_CEF_Helper PROPERTIES
		FRAMEWORK TRUE
		PUBLIC_HEADER "${ZEPHYROS__PUBLICHEADERS_SRCS};jsbridge_v8.h"
		INSTALL_NAME_DIR "@rpath"
	)

	set_target_properties(Zephyros_CEF
     	PROPERTIES
    	XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Frameworks"
 	)
 	#set_target_properties(Zephyros_CEF_Helper
    # 	PROPERTIES
    #	XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Frameworks"
 	#)

	# WebView configuration
	add_library(Zephyros_WebView SHARED ${ZEPHYROS_WEBVIEW_SRCS})
	target_include_directories(Zephyros_WebView PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/base/webview)
	target_compile_definitions(Zephyros_WebView PUBLIC -DUSE_WEBVIEW ${PLATFORM})
	target_link_libraries(Zephyros_WebView PUBLIC ${DEFAULT_LIBS} ${LIB_JAVASCRIPTCORE} ${LIB_SPARKLE})
	set_target_properties(Zephyros_WebView PROPERTIES
		FRAMEWORK TRUE
		PUBLIC_HEADER "${ZEPHYROS__PUBLICHEADERS_SRCS};jsbridge_webview.h;components/ZPYWebView.h;components/ZPYWebViewAppDelegate.h"
		RESOURCE "res/mac/MainWindow.xib;res/mac/LicenseCheckWindowController.xib"
		INSTALL_NAME_DIR "@rpath"
	)
	EMBED_FRAMEWORK(Zephyros_WebView ${LIB_SPARKLE})

	# AppStore configuration
	add_library(Zephyros_AppStore SHARED ${ZEPHYROS_APPSTORE_SRCS})
	target_include_directories(Zephyros_AppStore PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/base/webview)
	target_compile_definitions(Zephyros_AppStore PRIVATE -DUSE_WEBVIEW -DAPPSTORE ${PLATFORM})
	target_link_libraries(Zephyros_AppStore PRIVATE ${DEFAULT_LIBS} ${LIB_JAVASCRIPTCORE})
	set_target_properties(Zephyros_AppStore PROPERTIES
		FRAMEWORK TRUE
		PUBLIC_HEADER "${ZEPHYROS__PUBLICHEADERS_SRCS};jsbridge_webview.h;components/ZPYWebView.h;components/ZPYWebViewAppDelegate.h"
		RESOURCE "res/mac/MainWindow.xib"
		INSTALL_NAME_DIR "@rpath"
	)
elseif(OS_WINDOWS)
	add_library(Zephyros STATIC ${ZEPHYROS_CEF_SRCS})
	target_include_directories(Zephyros PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
	target_compile_definitions(Zephyros PRIVATE -DUSE_CEF ${PLATFORM})
elseif(OS_LINUX)
	# find required libraries and update compiler/linker variables
	FIND_LINUX_LIBRARIES("gmodule-2.0 gtk+-3.0 gthread-2.0")

	add_library(Zephyros STATIC ${ZEPHYROS_CEF_SRCS})
	target_include_directories(Zephyros PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib/cef)
	target_compile_definitions(Zephyros PRIVATE -DUSE_CEF ${PLATFORM})
endif()
