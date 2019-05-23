cmake_minimum_required (VERSION 3.8)

find_package(glew REQUIRED)
find_package(glut REQUIRED)
find_package(glm REQUIRED)

include_directories("${GLEW_INCLUDE_DIR}")
include_directories("${GLUT_INCLUDE_DIR}")
include_directories("$ENV{OPENVR_HOME}/headers")
include_directories("$ENV{GLM_HOME}/include")
include_directories("$ENV{DCMTK_HOME}/include")

link_directories("$ENV{DCMTK_HOME}/lib/Debug/")

add_compile_definitions(GLEW_STATIC)

add_executabl(CDVis "CDVis.cpp"
	"Pipeline/Texture.cpp"
	"Pipeline/Shader.cpp"
	"Pipeline/Mesh.cpp"
	"Scene/Object.cpp"
	"Scene/VRDevice.cpp"
	"Scene/Camera.cpp"
	"Scene/Volume.cpp"
	"Scene/MeshRenderer.cpp"
	"Scene/VRInteractable.cpp"
	"Util/ImageLoader.cpp"
	"Util/lodepng.cpp"
	"Util/FileBrowser.cpp")

target_link_libraries(CMIP "${GLEW_LIBRARIES}" "${GLUT_LIBRARIES}")
target_link_libraries(CMIP "ofstd.lib" "oflog.lib" "dcmdata.lib" "dcmimgle.lib")

if (WIN32)
	add_compile_definitions(WINDOWS)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
	target_link_libraries(CMIP "opengl32.lib" "ws2_32.lib" "wsock32.lib" "shlwapi.lib" "iphlpapi.lib" "netapi32.lib" "propsys.lib")
	target_link_libraries(CMIP "$ENV{OPENVR_HOME}/lib/win64/openvr_api.lib")
	configure_file("$ENV{GLUT_HOME}/bin/x64/freeglut.dll" "freeglut.dll" COPYONLY)
	configure_file("$ENV{OPENVR_HOME}/bin/win64/openvr_api.dll" "openvr_api.dll" COPYONLY)
	configure_file("$ENV{OPENVR_HOME}/bin/win64/openvr_api.pdb" "openvr_api.pdb" COPYONLY)
endif (WIN32)

configure_file("Shaders/blit.vert" "Shaders/blit.vert" COPYONLY)
configure_file("Shaders/blit.frag" "Shaders/blit.frag" COPYONLY)
configure_file("Shaders/solid.vert" "Shaders/solid.vert" COPYONLY)
configure_file("Shaders/solid.frag" "Shaders/solid.frag" COPYONLY)
configure_file("Shaders/volume.glsl" "Shaders/volume.glsl" COPYONLY)
configure_file("Shaders/volume.vert" "Shaders/volume.vert" COPYONLY)
configure_file("Shaders/volume.frag" "Shaders/volume.frag" COPYONLY)
configure_file("Shaders/textured.vert" "Shaders/textured.vert" COPYONLY)
configure_file("Shaders/textured.frag" "Shaders/textured.frag" COPYONLY)