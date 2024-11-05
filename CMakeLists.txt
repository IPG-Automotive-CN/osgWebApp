CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Path to OpenSceneGraph
SET(EXT_PROJ_DIR "${CMAKE_SOURCE_DIR}/../..")

# Specify critical OpenSceneGraph build variables.
SET(BUILD_OSG_APPLICATIONS NO CACHE BOOL "Do not build applications")
SET(EGL_LIBRARY "GL" CACHE STRING "Suppress linkage error")
SET(OSG_GL1_AVAILABLE OFF CACHE BOOL "Unavailable under Emscripten")
SET(OSG_GL2_AVAILABLE OFF CACHE BOOL "Unavailable under Emscripten")
SET(OSG_GLES2_AVAILABLE ON CACHE BOOL "GLES2 is what Emscripten uses")
SET(OSG_WINDOWING_SYSTEM "None" CACHE STRING "Since we use SDL2 for OpenGL context creation, we don't have a windowing system")
SET(DYNAMIC_OPENTHREADS OFF CACHE BOOL "Link OpenThreads statically")
SET(DYNAMIC_OPENSCENEGRAPH OFF CACHE BOOL "Link OpenSceneGraph statically")
set(JPEG_FOUND TRUE)

# Prevent CMake error during configuration.
SET(_OPENTHREADS_ATOMIC_USE_GCC_BUILTINS_EXITCODE "0" CACHE STRING "Prevent cfg error")

# Reference SDL2 during build process.
# We use SDL2 to do the following:
# * OpenGL functions' address retrieval
# * OpenGL graphics context creation
SET(USE_SDL2 "-s USE_LIBJPEG=1 -s USE_ZLIB=1 -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=256MB ")
SET(SKIP_UNDEFINED_SYMBOLS "-s ERROR_ON_UNDEFINED_SYMBOLS=0")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${USE_SDL2} ${SKIP_UNDEFINED_SYMBOLS}" CACHE STRING "Use SDL2" FORCE)
# Build and reference OpenSceneGraph.
SET(OSG_SOURCE_DIR "${EXT_PROJ_DIR}/OpenSceneGraph")
SET(OSG_BUILD_DIR "${EXT_PROJ_DIR}/OpenSceneGraph/build/Emscripten")
FILE(MAKE_DIRECTORY ${OSG_BUILD_DIR})
ADD_SUBDIRECTORY(${OSG_SOURCE_DIR} ${OSG_BUILD_DIR})
INCLUDE_DIRECTORIES(${OSG_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${OSG_BUILD_DIR}/include)
LINK_DIRECTORIES(${OSG_BUILD_DIR}/lib)

# Build sample.
INCLUDE_DIRECTORIES(src-gen)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
ADD_EXECUTABLE(sample-ems ${SOURCES})

# Make Emscripten generate ready-to-open HTML page.
SET(CMAKE_EXECUTABLE_SUFFIX ".html")


# Get the absolute path to the res directory
set(RES_DIR "${CMAKE_SOURCE_DIR}/res")

# Set the preload string
set(PRELOAD_STRING "--preload-file ${RES_DIR}@res")

# Set the link flags
set_target_properties(sample-ems PROPERTIES LINK_FLAGS "${PRELOAD_STRING} -lwebsocket.js")
# Make Emscripten preload the resource directory.
# SET_TARGET_PROPERTIES(sample-ems PROPERTIES LINK_FLAGS "--preload-file box.osgt")
# Copy the resource to the build directory.
CONFIGURE_FILE(box.osgt box.osgt COPYONLY)
# Libraries must be linked in the specified order.
# Otherwise you may get unsatisified linker errors.
TARGET_LINK_LIBRARIES(
    sample-ems
    osgViewer
    osgDB
    # osgDB plugins start.
    osgdb_osg
    osgdb_serializers_osgsim
    osgdb_serializers_osg
    osgdb_jpeg
    # osgDB plugins end.
    osgGA
    osgText
    osgUtil
    osg
    OpenThreads
)
