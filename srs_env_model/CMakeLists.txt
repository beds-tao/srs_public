cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries

#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()
#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
rosbuild_genmsg()
#uncomment if you have defined services
rosbuild_gensrv()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# BUT env. model server
set( SERVER_SOURCES src/but_server/but_server.cpp
                    src/but_server/server_tools.cpp
                    src/but_server/octonode.cpp
                    src/but_server/plugins/cmap_plugin.cpp
                    src/but_server/plugins/octomap_plugin.cpp
                    src/but_server/plugins/point_cloud_plugin.cpp
                    src/but_server/plugins/collision_object_plugin.cpp
                    src/but_server/plugins/map2d_plugin.cpp
                    src/but_server/plugins/imarkers_plugin.cpp
                    src/but_server/plugins/marker_array_plugin.cpp
                    src/but_server/plugins/limited_point_cloud_plugin.cpp
                    src/but_server/plugins/objtree_plugin.cpp
                    src/but_server/plugins/old_imarkers_plugin.cpp
                    src/but_server/plugins/octomap_plugin_tools/testing_oriented_box.cpp
                    src/but_server/plugins/octomap_plugin_tools/testing_planes.cpp
                    src/but_server/plugins/octomap_plugin_tools/testing_polymesh.cpp
                    )

set( OBJTREE_SOURCE_FILES src/but_server/objtree/bbox.cpp
                          src/but_server/objtree/gbbox.cpp
                          src/but_server/objtree/filter.cpp
                          src/but_server/objtree/node.cpp
                          src/but_server/objtree/object.cpp
                          src/but_server/objtree/octree.cpp
                          src/but_server/objtree/plane.cpp
                          src/but_server/objtree/history.cpp
                          )

# Objtree library
set( OBJTREE_LIB_NAME but_objtree )
rosbuild_add_library( ${OBJTREE_LIB_NAME} ${OBJTREE_SOURCE_FILES} )

# Add servernode executable
rosbuild_add_executable( but_server_node ${SERVER_SOURCES} src/nodes/server_node.cpp )
target_link_libraries( but_server_node ${OCTOMAP_LIBRARIES} ${OBJTREE_LIB_NAME} )

include_directories( include/but_server )

