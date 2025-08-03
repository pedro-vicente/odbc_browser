cmake_minimum_required(VERSION 3.31)
set(CMAKE_BUILD_TYPE Debug)
project (odbc_browser)

set(CMAKE_CXX_STANDARD 17)
message(STATUS "Source directory is " ${CMAKE_SOURCE_DIR})
message(STATUS "Build directory is " ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
  message(STATUS "Binary directory is " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
endif()

#//////////////////////////
# Linux/Mac compiling flags and needed libraries 
# build/ext/wxwidgets-3.3.1/wx-config --cppflags
# build/ext/wxwidgets-3.3.1/wx-config --libs

#//////////////////////////
# ext(ernal) dependencies 
# dependencies are dealt with:
# add_subdirectory(source_dir): Used to add a subdirectory to the build. There is also a CMakeLists.txt file in the source_dir. 
# include_directories(dir): Add the given directories to those the compiler uses to search for include files.
#//////////////////////////

#//////////////////////////
#ODBC Linux/Mac
#//////////////////////////

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(APPLE)
  message(STATUS "Mac OS build...")
endif()


find_package(ODBC REQUIRED)
message(STATUS "ODBC found: ${ODBC_FOUND}")
message(STATUS "ODBC include: ${ODBC_INCLUDE_DIRS}")
message(STATUS "ODBC library: ${ODBC_LIBRARIES}")
message(STATUS "ODBC config: ${ODBC_CONFIG}")

include_directories(${ODBC_INCLUDE_DIRS})


#//////////////////////////
# wxWidgets library locations 
#//////////////////////////

if (MSVC)
  include_directories(${CMAKE_SOURCE_DIR}/build/ext/wxwidgets-3.3.1/lib/vc_x64_lib/mswud)
  link_directories(${CMAKE_SOURCE_DIR}/build/ext/wxwidgets-3.3.1/lib/vc_x64_lib)
elseif(APPLE)
  include_directories(/usr/local/include/)
  include_directories(${CMAKE_SOURCE_DIR}/build/ext/wxwidgets-3.3.1/lib/wx/include/osx_cocoa-unicode-static-3.2)
  link_directories(${CMAKE_SOURCE_DIR}/build/ext/wxwidgets-3.3.1/lib)
  add_definitions(-DWXUSINGDLL -D__WXOSX_COCOA__ -D__WXMAC__ -D__WXOSX__)
elseif(LINUX)
  include_directories(${CMAKE_SOURCE_DIR}/build/ext/wxwidgets-3.3.1/lib/wx/include/gtk3-unicode-static-3.2)
  link_directories(${CMAKE_SOURCE_DIR}/build/ext/wxwidgets-3.3.1/lib)
  add_definitions(-D__WXGTK3__ -D__WXGTK__)
endif()

#//////////////////////////
# wxWidgets source
#//////////////////////////

include_directories(${CMAKE_SOURCE_DIR}/ext/wxwidgets-3.3.1/include)

#//////////////////////////
#sources 
#//////////////////////////

set(src ${src})
if(APPLE)
  set(src ${src} ${CMAKE_SOURCE_DIR}/ext/wxwidgets-3.3.1/src/osx/carbon/wxmac.icns)
endif()
if (MSVC)
  set(src ${src} ${CMAKE_SOURCE_DIR}/src/sample.rc)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

#//////////////////////////
# Interface
#//////////////////////////

set(src ${src} src/frame.cc)
set(src ${src} src/frame.hh)
set(src ${src} src/table.hh)
set(src ${src} src/table.cc)
set(src ${src} src/wodbc.hh)
set(src ${src} src/wodbc.cc)
set(src ${src} src/database.cc)
set(src ${src} src/database.hh)

add_executable(odbc_browser WIN32 MACOSX_BUNDLE ${src})

if (MSVC)
  set_target_properties(odbc_browser PROPERTIES LINK_FLAGS "/SubSystem:\"Windows\"" VS_DPI_AWARE "ON")
endif()

#//////////////////////////
#link with libraries
#lib_dep contains a cascade definition of all the libraries needed to link
#//////////////////////////

set(lib_dep ${lib_dep})
set(ODBC_LIBRARY ${ODBC_LIBRARY})

if (MSVC)
  set(lib_dep ${lib_dep} wxbase33ud wxmsw33ud_core wxmsw33ud_webview wxbase33ud_net wxpngd wxzlibd wxregexud wxmsw33ud_html)
  set(lib_dep ${lib_dep} wxscintillad wxmsw33ud_stc wxlexillad)
  
  set(ODBC_LIBRARY ${ODBC_LIBRARY} odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib ws2_32.lib)
endif()


if (LINUX)
  set(lib_dep ${lib_dep} wx_gtk3u_xrc-3.3 wx_gtk3u_qa-3.3 wx_baseu_net-3.3 wx_gtk3u_html-3.3 wx_gtk3u_core-3.3)
  set(lib_dep ${lib_dep} wxtiff-3.3 wx_baseu-3.3)
  set(lib_dep ${lib_dep} gtk-3 gdk-3 pangocairo-1.0 pango-1.0 harfbuzz atk-1.0 cairo-gobject cairo gdk_pixbuf-2.0 gio-2.0 gobject-2.0)
  set(lib_dep ${lib_dep} X11 glib-2.0 png jpeg z)
endif()

if(APPLE)
  set(lib_dep ${lib_dep} wx_osx_cocoau_xrc-3.3 wx_osx_cocoau_qa-3.3 wx_baseu_net-3.3 wx_osx_cocoau_html-3.3 wx_osx_cocoau_core-3.3)
  set(lib_dep ${lib_dep} wx_baseu_xml-3.3 wx_baseu-3.3 wx_osx_cocoau_webview-3.3)
  set(lib_dep ${lib_dep} wx_osx_cocoau_core-3.3 wxregexu-3.3 wxjpeg-3.3 wxpng-3.3 wxtiff-3.3 z iconv)
  set(lib_dep ${lib_dep} wx_osx_cocoau_stc-3.3)
  set(lib_dep ${lib_dep} wxscintilla-3.3 wxlexilla-3.3)
  set(ODBC_LIBRARY ${ODBC_LIBRARY} /usr/local/Cellar/msodbcsql18/18.1.2.1/lib/libmsodbcsql.18.dylib)
  find_library(COREFOUNDATION CoreFoundation)
  if (NOT COREFOUNDATION)
      message(FATAL_ERROR "CoreFoundation not found")
  endif()
  find_library(SECURITY Security)
  find_library(CARBON Carbon)
  find_library(COCOA Cocoa)
  find_library(IOKIT IOKit)
  find_library(WEBKIT WebKit)
  find_library(ANIMATION QuartzCore)
  set(lib_dep ${lib_dep} ${COREFOUNDATION} ${SECURITY} ${CARBON} ${COCOA} ${IOKIT} ${WEBKIT} ${ANIMATION})
endif()

if(UNIX)
  find_path(ODBC_INCLUDE sql.h ${find_opt})
  if(NOT ODBC_INCLUDE)
    message(FATAL_ERROR "sql.h header file not found")
  else()
    message("-- Found sql.h header file at: " ${ODBC_INCLUDE})
  endif()
  include_directories(${ODBC_INCLUDE})
  find_library(ODBC_LIBRARY NAMES odbc PATHS "/usr/local/lib")
  if(NOT ODBC_LIBRARY)
    message(FATAL_ERROR "ODBC library not found$")
  else()
    message("-- Found ODBC library at: " ${ODBC_LIBRARY})
  endif()
endif()

#//////////////////////////
#LINUX
#order of the link libraries matters; pthread dl ${ODBC_LIBRARY} must be last
#//////////////////////////

if(LINUX)
  set(lib_dep ${lib_dep} pthread dl)
  find_program(LSB_RELEASE_EXEC lsb_release)
  execute_process(COMMAND ${LSB_RELEASE_EXEC} -is OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Building in " ${LSB_RELEASE_ID_SHORT})
endif()

set(lib_dep ${lib_dep} ${ODBC_LIBRARIES})

set(lib_dep ${lib_dep} ${ODBC_LIBRARY})

target_link_libraries(odbc_browser ${lib_dep})

if(APPLE)
  set(VERSION "1.0")
  set(COPYRIGHT "Sarsat")
  if(NOT IPHONE)
    set_target_properties(odbc_browser PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/ext/wxwidgets-3.3.1/src/osx/carbon/Info.plist.in"
      RESOURCE "${CMAKE_SOURCE_DIR}/ext/wxwidgets-3.3.1/src/osx/carbon/wxmac.icns")
  endif()
  set_target_properties(odbc_browser PROPERTIES
    MACOSX_BUNDLE_ICON_FILE wxmac.icns
    MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}"
    MACOSX_BUNDLE_VERSION "${VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "${COPYRIGHT}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.wxwidgets.qms"
    )
  message(STATUS "MacOS bundle version: " ${VERSION})
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT odbc_browser)
