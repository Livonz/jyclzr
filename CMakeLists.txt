cmake_minimum_required(VERSION 3.14)
project(jyengine VERSION 1.0.0 LANGUAGES C OBJC)

set(CMAKE_C_STANDARD 11)

# iOS settings
if(IOS)
    set(CMAKE_SYSTEM_NAME iOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
    set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
    if(NOT DEFINED CMAKE_OSX_SYSROOT)
        set(CMAKE_OSX_SYSROOT iphoneos)
    endif()
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-" CACHE STRING "")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO)
endif()

# Lua source (embedded)
set(LUA_DIR ${CMAKE_SOURCE_DIR}/third_party/lua)
file(GLOB LUA_SOURCES ${LUA_DIR}/*.c)
list(FILTER LUA_SOURCES EXCLUDE REGEX "luac\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX "lua\\.c$")

# Engine source files
set(ENGINE_SOURCES
    src/jymain.c
    src/sdlfun.c
    src/luafun.c
    src/mainmap.c
    src/piccache.c
    src/charset.c
)

# iOS app source files
if(IOS)
    list(APPEND ENGINE_SOURCES
        ios/jyengine/main.m
        ios/jyengine/AppDelegate.m
        ios/jyengine/JYViewController.m
    )
endif()

# Find SDL2 - try multiple methods
if(DEFINED SDL2_LIBRARY AND DEFINED SDL2_INCLUDE_DIR)
    # Manual specification (used in CI)
    set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
    set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR})
    set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
    set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})
else()
    # Try find_package
    find_package(SDL2 QUIET)
    find_package(SDL2_image QUIET)
    find_package(SDL2_mixer QUIET)
    find_package(SDL2_ttf QUIET)
endif()

# Create executable
if(IOS)
    add_executable(jyengine MACOSX_BUNDLE
        ${ENGINE_SOURCES}
        ${LUA_SOURCES}
    )
    set_target_properties(jyengine PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.jyengine.ios"
        MACOSX_BUNDLE_BUNDLE_NAME "JY Engine"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/ios/Info.plist"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.jyengine.ios"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_APPLICATION_EXTENSION_API_ONLY NO
    )
else()
    add_executable(jyengine
        ${ENGINE_SOURCES}
        ${LUA_SOURCES}
    )
endif()

# Include directories
target_include_directories(jyengine PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${LUA_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(jyengine PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

# iOS frameworks
if(IOS)
    target_link_libraries(jyengine PRIVATE
        "-framework UIKit"
        "-framework Foundation"
        "-framework CoreGraphics"
        "-framework QuartzCore"
    )
    set_target_properties(jyengine PROPERTIES
        INSTALL_RPATH "@executable_path/Frameworks"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

target_compile_definitions(jyengine PRIVATE
    $<$<BOOL:${IOS}>:IOS_BUILD>
)

if(NOT MSVC)
    target_compile_options(jyengine PRIVATE -Wall -Wno-unused-variable -Wno-unused-function)
endif()

install(TARGETS jyengine
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
