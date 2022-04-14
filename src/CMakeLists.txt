# /src/

include(C:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(common.cmake)

# Proto file
get_filename_component(mps_proto "../protos/multiplayer_scene.proto" ABSOLUTE)
get_filename_component(mps_proto_path "${mps_proto}" PATH)

# Generated sources
set(mps_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/multiplayer_scene.pb.cc")
set(mps_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/multiplayer_scene.pb.h")
set(mps_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/multiplayer_scene.grpc.pb.cc")
set(mps_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/multiplayer_scene.grpc.pb.h")

add_custom_command(
      OUTPUT "${mps_proto_srcs}" "${mps_proto_hdrs}" "${mps_grpc_srcs}" "${mps_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${mps_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${mps_proto}"
      DEPENDS "${mps_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# mps_grpc_proto
add_library(mps_grpc_proto
  ${mps_grpc_srcs}
  ${mps_grpc_hdrs}
  ${mps_proto_srcs}
  ${mps_proto_hdrs})
target_link_libraries(mps_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# multiplayer_scene_helper
add_library(multiplayer_scene_helper
  "helper.h"
  "helper.cc")
target_link_libraries(multiplayer_scene_helper
  mps_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_executable(multiplayer_scene_server "server/multiplayer_scene_server.cc")
target_link_libraries(multiplayer_scene_server mps_grpc_proto multiplayer_scene_helper ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

# Make symbols for anything you need from visualisations here.
add_library(client_graphics_imports
	"${visualisations_SOURCE_DIR}/visualisations/graphics/Texture.h"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/Texture.cpp"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/Mesh.h"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/Mesh.cpp"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/MeshGenerator.h"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/MeshGenerator.cpp"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/GeometryTools.h"
	"${visualisations_SOURCE_DIR}/visualisations/graphics/GeometryTools.cpp"
	
	"${visualisations_SOURCE_DIR}/visualisations/general/Constants.h"
	)
target_link_libraries(client_graphics_imports glfw glad)
target_include_directories(client_graphics_imports PUBLIC ${stb_SOURCE_DIR} ${glm_SOURCE_DIR} ${visualisations_SOURCE_DIR})

add_executable(multiplayer_scene_client "client/multiplayer_scene_client.h" "client/graphics.h" "client/graphics.cpp" "client/main.cpp")
target_include_directories(multiplayer_scene_client PUBLIC ${stb_SOURCE_DIR} ${visualisations_SOURCE_DIR} ${glm_SOURCE_DIR})
# TODO: link visualisations in.
target_link_libraries(multiplayer_scene_client mps_grpc_proto multiplayer_scene_helper glfw glad client_graphics_imports ${Boost_LIBRARIES} ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

