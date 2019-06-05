cmake_minimum_required (VERSION 3.8)

include_directories("$ENV{GLEW_HOME}/include")
include_directories("$ENV{GLFW_HOME}/include")
include_directories("$ENV{OPENVR_HOME}/headers")
include_directories("$ENV{GLM_HOME}/include")
include_directories("$ENV{DCMTK_HOME}/include")

link_directories("$ENV{DCMTK_HOME}/lib/Debug/")

add_compile_definitions(GLEW_STATIC)

add_executable(CDVis "CDVis.cpp"
	"Pipeline/AssetDatabase.cpp"
	"Pipeline/Font.cpp"
	"Pipeline/Mesh.cpp"
	"Pipeline/Shader.cpp"
	"Pipeline/Texture.cpp"
	"Scene/Camera.cpp"
	"Scene/MeshRenderer.cpp"
	"Scene/Object.cpp"
	"Scene/Volume.cpp"
	"Scene/VRDevice.cpp"
	"Scene/VRInteractable.cpp"
	"Scene/VRPieMenu.cpp"
	"ThirdParty/stb_imp.cpp"
	"Util/FileBrowser.cpp"
	"Util/ImageLoader.cpp"
	"Util/Util.cpp")

target_link_libraries(CDVis "ofstd.lib" "oflog.lib" "dcmdata.lib" "dcmimgle.lib")
set_property(TARGET CDVis PROPERTY CXX_STANDARD 17)

if (WIN32)
	add_compile_definitions(WINDOWS)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
	target_link_libraries(CDVis "opengl32.lib" "ws2_32.lib" "wsock32.lib" "shlwapi.lib" "iphlpapi.lib" "netapi32.lib" "propsys.lib")
	target_link_libraries(CDVis "$ENV{GLFW_HOME}/lib-vc2019/glfw3.lib")
	target_link_libraries(CDVis "$ENV{OPENVR_HOME}/lib/win64/openvr_api.lib")
	configure_file("$ENV{OPENVR_HOME}/bin/win64/openvr_api.dll" "openvr_api.dll" COPYONLY)
	configure_file("$ENV{OPENVR_HOME}/bin/win64/openvr_api.pdb" "openvr_api.pdb" COPYONLY)

	target_link_libraries(CDVis "$ENV{GLEW_HOME}/lib/Release/x64/glew32s.lib" "$ENV{GLUT_HOME}/lib/x64/freeglut.lib")
endif (WIN32)

configure_file("Assets/dial_diffuse.png" "Assets/dial_diffuse.png" COPYONLY)
configure_file("Assets/icons.png"		"Assets/icons.png" COPYONLY)
configure_file("Assets/pen_diffuse.png"	"Assets/pen_diffuse.png" COPYONLY)
configure_file("Assets/pie_icons.png"	"Assets/pie_icons.png" COPYONLY)

configure_file("Assets/tools.fbx"		"Assets/tools.fbx" COPYONLY)

configure_file("Assets/blit.vert"		"Assets/blit.vert" COPYONLY)
configure_file("Assets/blit.frag"		"Assets/blit.frag" COPYONLY)
configure_file("Assets/pie.vert"		"Assets/pie.vert" COPYONLY)
configure_file("Assets/pie.frag"		"Assets/pie.frag" COPYONLY)
configure_file("Assets/textured.vert"	"Assets/textured.vert" COPYONLY)
configure_file("Assets/textured.frag"	"Assets/textured.frag" COPYONLY)
configure_file("Assets/volume.glsl"		"Assets/volume.glsl" COPYONLY)
configure_file("Assets/volume.vert"		"Assets/volume.vert" COPYONLY)
configure_file("Assets/volume.frag"		"Assets/volume.frag" COPYONLY)