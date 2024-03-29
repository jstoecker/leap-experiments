cmake_minimum_required(VERSION 2.8.12)
project(leap-experiments)

# Directory for cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# output the executable in the build directory (not Debug/Release subdir)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

set(DATA_SHADERS
    shaders/color.vert
    shaders/color.frag
    shaders/mask_volume.vert
    shaders/mask_volume.frag
    shaders/texture_2D.vert
    shaders/texture_2D.frag
)

set(DATA_FONTS
    fonts/menlo18.bmp
    fonts/menlo18.dat
)

set(DATA_IMAGES
	images/carry.jpg
	images/fist.jpg
	images/thumb.jpg
	images/l_closed.jpg
	images/l_open.jpg
	images/palms_face.jpg
	images/point.jpg
	images/point2.jpg
	images/three.jpg
	images/v_closed.jpg
	images/v_open.jpg
	images/pinch.jpg
)

foreach(item IN LISTS DATA_SHADERS DATA_FONTS DATA_IMAGES)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${item}" 
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/${item}"
        DEPENDS   "${CMAKE_CURRENT_SOURCE_DIR}/${item}"
    ) 
endforeach()
add_custom_target(SHADER_COPY ALL DEPENDS ${DATA_SHADERS} ${DATA_FONTS} ${DATA_IMAGES})


# source group naming for IDEs
source_group("fonts" FILES ${DATA_FONTS})
source_group("images" FILES ${DATA_IMAGES})
source_group("shaders" FILES ${DATA_SHADERS})
file(GLOB SOURCE_GL src/gl/*.cpp src/gl/*.c src/gl/*.h)
source_group("gl" FILES ${SOURCE_GL})
file(GLOB SOURCE_GL_GEOM src/gl/geom/*.cpp src/gl/geom/*.h)
source_group("gl\\geom" FILES ${SOURCE_GL_GEOM})
file(GLOB SOURCE_GL_UTIL src/gl/util/*.cpp src/gl/util/*.h)
source_group("gl\\util" FILES ${SOURCE_GL_UTIL})
file(GLOB SOURCE_GL_MATH src/gl/math/*.cpp src/gl/math/*.h)
source_group("gl\\math" FILES ${SOURCE_GL_MATH})
file(GLOB SOURCE_MAIN src/*.cpp src/*.h)
source_group("" FILES ${SOURCE_MAIN})
file(GLOB SOURCE_POSES src/poses/*.cpp src/poses/*.h)
source_group("poses" FILES ${SOURCE_POSES})
file(GLOB SOURCE_EXPERIMENTS src/experiments/*.cpp src/experiments/*.h)
source_group("experiments" FILES ${SOURCE_EXPERIMENTS})
file(GLOB SOURCE_UTIL src/util/*.cpp src/util/*.h src/util/*.c)
source_group("util" FILES ${SOURCE_UTIL})

set(SOURCE_AND_RESOURCES
    ${SOURCE_MAIN}
    ${SOURCE_GL}
    ${SOURCE_GL_GEOM}
    ${SOURCE_GL_UTIL}
    ${SOURCE_GL_MATH}
    ${SOURCE_POSES}
    ${SOURCE_EXPERIMENTS}
    ${SOURCE_UTIL}
    ${DATA_SHADERS}
    ${DATA_FONTS}
    ${DATA_IMAGES}
)

# set root of source directory for finding headers
include_directories(src)

# Find the libraries (must have LEAP_DIR and possibly GLFW_DIR env variables)
find_package(GLFW CONFIG REQUIRED)
include_directories(${GLFW_INCLUDE_DIR})
find_package(Leap REQUIRED)
include_directories(${LEAP_INCLUDE_DIR})

message(STATUS ${LEAP_INCLUDE_DIR})

if (APPLE)

    add_executable(${PROJECT_NAME} ${SOURCE_AND_RESOURCES})
    #add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_AND_RESOURCES})

    # Copy resources to the right place in the app bundle
#    SET_SOURCE_FILES_PROPERTIES(
#        ${DATA_SHADERS} 
#        PROPERTIES 
#        MACOSX_PACKAGE_LOCATION
#        Resources/shaders
#    )
#    SET_SOURCE_FILES_PROPERTIES(
#        ${DATA_FONTS} 
#        PROPERTIES 
#        MACOSX_PACKAGE_LOCATION
#        Resources/fonts
#    )

    # for C++11
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")

    # GLFW needs OpenGL, Cocoa, IOKit, and CoreVideo frameworks linked
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)

    # Link all the libraries
    target_link_libraries(
        ${PROJECT_NAME}
        ${GLFW_LIBRARY}
        ${OPENGL_LIBRARY}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${LEAP_LIBRARY}
    )

    # Copy libLeap.dylib to output directory
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy
        ${LEAP_LIBRARY}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

endif(APPLE)

if (WIN32)
    add_executable(${PROJECT_NAME} ${SOURCE_AND_RESOURCES})

	# is this still needed? check
	add_definitions(-DGLEW_STATIC)

    target_link_libraries(
        ${PROJECT_NAME}
		opengl32
        ${GLFW_LIBRARY}
    )

    # use Leap.lib for RELEASE or Leapd.lib for DEBUG
    target_link_libraries (${PROJECT_NAME} optimized ${LEAP_LIB_RELEASE} debug ${LEAP_LIB_DEBUG})

    # copy Leap.dll for RELEASE or Leapd.dll for DEBUG into output directory
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy
        "$<$<CONFIG:debug>:${LEAP_DLL_DEBUG}>$<$<CONFIG:release>:${LEAP_DLL_RELEASE}>"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif(WIN32)

# Ensure shaders are copied whenever modified
add_dependencies(${PROJECT_NAME} SHADER_COPY)
