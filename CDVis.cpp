#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>

#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <openvr.h>

#ifdef WINDOWS
#include <Windows.h>
#include <gl/wglew.h>
#endif

#include "Scene/VRDevice.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Volume.hpp"
#include "Scene/MeshRenderer.hpp"
#include "Pipeline/Shader.hpp"
#include "Pipeline/Mesh.hpp"
#include "Pipeline/Texture.hpp"
#include "Util/FileBrowser.hpp"
#include "Util/ImageLoader.hpp"

using namespace std;
using namespace glm;

// glut functions
void Resize(int, int);
void Render();
void Idle();
void KeyUp(unsigned char, int, int);
void KeyDown(unsigned char, int, int);
void Mouse(int, int, int, int);
void MouseMove(int, int);
void MouseDrag(int, int);
void MouseWheel(int, int, int, int);

void InitScene();
void Cleanup();

int windowWidth = 1600;
int windowHeight = 900;

bool keys[0xFF];
int mouse[5];
ivec2 mousePos;
float camZ = -1.25f;

vr::IVRRenderModels* vrRenderModelInterface;
vr::IVRSystem* hmd;
bool vrEnable;

shared_ptr<Camera> camera;
vector<shared_ptr<Volume>> volumes;

vector<shared_ptr<VRDevice>> vrDevices;
unordered_map<string, shared_ptr<Mesh>> vrMeshes;
unordered_map<vr::TextureID_t, shared_ptr<Texture>> vrTextures;
vr::TrackedDevicePose_t vrTrackedDevices[vr::k_unMaxTrackedDeviceCount];

shared_ptr<Shader> blitShader;
shared_ptr<Shader> texturedShader;
shared_ptr<Shader> solidShader;
shared_ptr<Mesh> screenQuad;

shared_ptr<Mesh> box;

vector<shared_ptr<Renderer>> scene;

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(windowWidth, windowHeight);

	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);

	int glutHandle = glutCreateWindow("CMIP");
	if (glutHandle < 1) {
		printf("Failed to create window\n");
		exit(EXIT_FAILURE);
	}

	// disable vsync for higher framerates
	#ifdef WINDOWS
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	wglSwapIntervalEXT(0);
	#endif

	// print some info
	if (const GLubyte* renderer = glGetString(GL_RENDERER))
		printf("OpenGL Renderer: %s\n", renderer);
	else {
		printf("Failed to get OpenGL renderer\n");
		exit(EXIT_FAILURE);
	}

	if (const GLubyte* version = glGetString(GL_VERSION))
		printf("OpenGL Version: %s\n", version);
	else {
		printf("Failed to get OpenGL version\n");
		exit(EXIT_FAILURE);
	}

	glutReshapeFunc(Resize);
	glutDisplayFunc(Render);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutIdleFunc(Idle);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseDrag);
	glutPassiveMotionFunc(MouseMove);
	glutMouseWheelFunc(MouseWheel);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialize GLEW!\n");
		exit(EXIT_FAILURE);
	}

	memset(keys, 0, 0xFF * sizeof(bool));
	memset(mouse, GLUT_UP, 5 * sizeof(int));

	InitScene();
	
	glutMainLoop();

	Cleanup();

	exit(EXIT_SUCCESS);
}

bool InitVR() {
	vr::EVRInitError eError = vr::VRInitError_None;
	hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None) {
		printf("Failed to initialize VR: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return false;
	}

	vrRenderModelInterface = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!vrRenderModelInterface) {
		vr::VR_Shutdown();
		printf("Failed to get render model interface: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return false;
	}

	if (!vr::VRCompositor()) {
		printf("Failed to initialize compositor.\n");
		return false;
	}

	unsigned int w, h;
	hmd->GetRecommendedRenderTargetSize(&w, &h);

	vrDevices.reserve(vr::k_unMaxTrackedDeviceCount);
	for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		vrDevices.push_back(nullptr);

	vrEnable = true;
	return true;
}

void InitScene() {
	blitShader = shared_ptr<Shader>(new Shader());
	blitShader->AddShaderFile(GL_VERTEX_SHADER, "Shaders/blit.vert");
	blitShader->AddShaderFile(GL_FRAGMENT_SHADER, "Shaders/blit.frag");
	blitShader->CompileAndLink();

	solidShader = shared_ptr<Shader>(new Shader());
	solidShader->AddShaderFile(GL_VERTEX_SHADER, "Shaders/solid.vert");
	solidShader->AddShaderFile(GL_FRAGMENT_SHADER, "Shaders/solid.frag");
	solidShader->CompileAndLink();

	texturedShader = shared_ptr<Shader>(new Shader());
	texturedShader->AddShaderFile(GL_VERTEX_SHADER, "Shaders/textured.vert");
	texturedShader->AddShaderFile(GL_FRAGMENT_SHADER, "Shaders/textured.frag");
	texturedShader->CompileAndLink();

	#pragma region screen quad
	screenQuad = shared_ptr<Mesh>(new Mesh());
	screenQuad->BindVAO();

	static const GLfloat quadVertices[8] {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f,
	};
	static const GLuint quadIndices[6] {
		0, 1, 2,
		3, 2, 1,
	};

	screenQuad->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, quadVertices, GL_STATIC_DRAW);

	screenQuad->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, quadIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	screenQuad->ElementCount(6);
	#pragma endregion

	box = shared_ptr<Mesh>(new Mesh());
	box->BindVAO();

	static const vec3 boxVertices[8]{
		{-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
		{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}
	};
	static const GLuint boxIndices[36]{
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		4, 5, 1, 1, 0, 4,
		3, 2, 6, 6, 7, 3
	};

	box->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, boxVertices, GL_STATIC_DRAW);

	box->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 36, boxIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (GLvoid*)0);

	box->ElementCount(36);
	glBindVertexArray(0);

	camera = shared_ptr<Camera>(new Camera());
	
	auto v = shared_ptr<Volume>(new Volume());
	v->LocalScale(.5f, .5f, .5f);
	volumes.push_back(v);
	scene.push_back(v);

	camera->PixelWidth(windowWidth);
	camera->PixelHeight(windowHeight);
	camera->LocalPosition(0.f, 0.f, -1.25f);

	if (!InitVR()) {
		printf("Failed to initialize OpenVR\n");
		hmd = nullptr;
		vrEnable = false;
	}
}
void Cleanup() {
	camera.reset();
	blitShader.reset();
	screenQuad.reset();
	volumes.clear();
	vrTextures.clear();
	vrMeshes.clear();
	vrDevices.clear();
	scene.clear();
	if (hmd) vr::VR_Shutdown();
}

#pragma region glut input callbacks

void MouseWheel(int wheel, int dir, int x, int y) {
	camera->LocalPosition(camera->LocalPosition() + camera->WorldRotation() * vec3(0.f, 0.f, dir * .03f));
}
void MouseDrag(int x, int y) {
	mousePos.x = x;
	mousePos.y = y;
}
void MouseMove(int x, int y){
	mousePos.x = x;
	mousePos.y = y;
}
void Mouse(int btn, int state, int x, int y) {
	mouse[btn] = state;
}
void KeyDown(unsigned char key, int x, int y) {
	keys[key] = true;

	switch (key) {
	case 'o': {
		#ifdef WINDOWS
		string folder = BrowseFolder(GetForegroundWindow());
		#endif
		if (folder.empty()) break;

		vec3 size;
		volumes[0]->Texture(ImageLoader::LoadVolume(folder, size));
		volumes[0]->LocalScale(size);

		break;
	}
	case 'v':
		vrEnable = !vrEnable;
		break;
	case 'g':
		volumes[0]->DisplaySampleCount(!volumes[0]->DisplaySampleCount());
		break;
	}
}
void KeyUp(unsigned char key, int x, int y) {
	keys[key] = false;
}

#pragma endregion

void Resize(int w, int h) {
	windowWidth = w;
	windowHeight = h;
	camera->PixelWidth(w);
	camera->PixelHeight(h);
}

void GetVRRenderModel(unsigned int index, MeshRenderer* renderer) {
	unsigned int len = hmd->GetStringTrackedDeviceProperty(index, vr::TrackedDeviceProperty::Prop_RenderModelName_String, NULL, 0);
	char* name = new char[len];
	hmd->GetStringTrackedDeviceProperty(index, vr::TrackedDeviceProperty::Prop_RenderModelName_String, name, len);

	vr::RenderModel_t* renderModel;
	vr::RenderModel_TextureMap_t* renderModelDiffuse;

	vr::EVRRenderModelError error;
	while ((error = vrRenderModelInterface->LoadRenderModel_Async(name, &renderModel)) == vr::VRRenderModelError_Loading)
		#ifdef WINDOWS
		Sleep(1);
		#else
		throw exception();
		#endif
	if (error != vr::VRRenderModelError_None) return;

	while ((error = vrRenderModelInterface->LoadTexture_Async(renderModel->diffuseTextureId, &renderModelDiffuse)) == vr::VRRenderModelError_Loading)
		#ifdef WINDOWS
		Sleep(1);
		#else
		throw exception();
		#endif

	if (error != vr::VRRenderModelError_None) {
		vrRenderModelInterface->FreeRenderModel(renderModel);
		return;
	}

	if (vrMeshes.count(name)) {
		// we've already created this mesh earlier
		renderer->Mesh(vrMeshes.at(name));
		renderer->Uniform("Texture", vrTextures.at(renderModel->diffuseTextureId));
		return;
	}

	vector<GLuint> indexBuffer(renderModel->unTriangleCount * 3);
	for (unsigned int i = 0; i < renderModel->unTriangleCount * 3; i++)
		indexBuffer[i] = renderModel->rIndexData[i];
	vector<vr::RenderModel_Vertex_t> vertexBuffer(renderModel->unVertexCount);
	memcpy(vertexBuffer.data(), renderModel->rVertexData, sizeof(vr::RenderModel_Vertex_t) * renderModel->unVertexCount);
	for (unsigned int i = 0; i < renderModel->unVertexCount; i++) {
		vertexBuffer[i].vPosition.v[2] = -vertexBuffer[i].vPosition.v[2];
		vertexBuffer[i].vNormal.v[2] = -vertexBuffer[i].vNormal.v[2];
	}

	shared_ptr<Mesh> mesh = shared_ptr<Mesh>(new Mesh());
	mesh->BindVAO();

	mesh->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vertexBuffer.size(), vertexBuffer.data(), GL_STATIC_DRAW);

	mesh->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexBuffer.size(), indexBuffer.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	mesh->ElementCount(indexBuffer.size());
	
	glBindVertexArray(0);

	shared_ptr<Texture> texture = shared_ptr<Texture>(new Texture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, renderModelDiffuse->unWidth, renderModelDiffuse->unHeight, (void*)renderModelDiffuse->rubTextureMapData));
	glBindTexture(GL_TEXTURE_2D, texture->GLTexture());
	glGenerateMipmap(GL_TEXTURE_2D);
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	vrMeshes.emplace(name, mesh);
	vrTextures.emplace(renderModel->diffuseTextureId, texture);

	renderer->Mesh(mesh);
	renderer->Uniform("Texture", texture);

	vrRenderModelInterface->FreeRenderModel(renderModel);
	vrRenderModelInterface->FreeTexture(renderModelDiffuse);

	delete[] name;
}
void VRCreateDevice(unsigned int index) {
	vrDevices[index] = shared_ptr<VRDevice>(new VRDevice(index));
	vrDevices[index]->Shader(texturedShader);
	GetVRRenderModel(index, vrDevices[index].get());

	scene.push_back(vrDevices[index]);
}

void Idle() {
	// Get poses as late as possible to maintain realtimeness
	if (hmd) vr::VRCompositor()->WaitGetPoses(vrTrackedDevices, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	static std::chrono::high_resolution_clock timer;
	static auto lastTime = timer.now();

	auto n = timer.now();
	double deltaTime = (n - lastTime).count() * 1e-9;
	lastTime = n;

	#pragma region PC controls
	static ivec2 mouseLast;
	vec2 md = mousePos - mouseLast;
	mouseLast = mousePos;

	if (mouse[GLUT_LEFT_BUTTON] == GLUT_DOWN) {
		vec3 axis;

		if (keys[' ']) {
			vec2 v(mousePos.x - windowWidth / 2, mousePos.y - windowHeight / 2);
			v = vec2(-v.y, v.x);
			axis = camera->WorldRotation() * vec3(0.f, 0.f, 1.f) * dot(v, md);
		} else {
			axis = camera->WorldRotation() * vec3(0.f, 1.f, 0.f) * md.x + camera->WorldRotation() * vec3(1.f, 0.f, 0.f) * md.y;
		}

		if (length(axis) > .01f) {
			quat delta = angleAxis(length(md) * .003f, -normalize(axis));
			volumes[0]->LocalRotation(delta * volumes[0]->LocalRotation());
		}
	}

	if (mouse[GLUT_RIGHT_BUTTON] == GLUT_DOWN) {
		vec3 mv(0.f);
		if (keys['w']) mv += vec3(0.f, 0.f, 1.f);
		if (keys['s']) mv -= vec3(0.f, 0.f, 1.f);
		if (keys['a']) mv -= vec3(1.f, 0.f, 0.f);
		if (keys['d']) mv += vec3(1.f, 0.f, 0.f);
		mv = camera->WorldRotation() * mv;
		camera->LocalPosition(camera->LocalPosition() + mv * (float)deltaTime * .25f);

		static vec3 euler;
		euler += vec3(md.y, md.x, 0.f) * .0025f;
		euler.x = clamp(euler.x, -pi<float>() * .5f, pi<float>() * .5f);
		camera->LocalRotation(quat(euler));
	}

	if (keys['z']) volumes[0]->Threshold(volumes[0]->Threshold() - (float)deltaTime * .2f);
	if (keys['x']) volumes[0]->Threshold(volumes[0]->Threshold() + (float)deltaTime * .2f);

	if (keys['k']) volumes[0]->Exposure(volumes[0]->Exposure() - (float)deltaTime);
	if (keys['l']) volumes[0]->Exposure(volumes[0]->Exposure() + (float)deltaTime);

	if (keys['n']) volumes[0]->Density(volumes[0]->Density() - (float)deltaTime * .5f);
	if (keys['m']) volumes[0]->Density(volumes[0]->Density() + (float)deltaTime * .5f);

	if (keys['h']) volumes[0]->StepSize(volumes[0]->StepSize() - (float)deltaTime * .0002f);
	if (keys['j']) volumes[0]->StepSize(volumes[0]->StepSize() + (float)deltaTime * .0002f);
	#pragma endregion
	
	#pragma region VR Controls
	static vector<shared_ptr<VRDevice>> trackedControllers;
	if (hmd) {
		// Process SteamVR events (controller added/removed/etc)
		vr::VREvent_t event;
		while (hmd->PollNextEvent(&event, sizeof(event))) {}

		// Gather input from tracked controllers and convert to VRController class

		bool drawControllers = !hmd->IsSteamVRDrawingControllers();
		trackedControllers.clear();

		for (vr::TrackedDeviceIndex_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
			if (!vrTrackedDevices[i].bPoseIsValid) {
				// don't draw or update if the device's pose isn't valid
				if (vrDevices[i]) vrDevices[i]->mTracking = false;
				continue;
			}

			if (!vrDevices[i]) VRCreateDevice(i);

			// don't draw devices if SteamVR is drawing them already (in SteamVR menu, etc)
			vrDevices[i]->mTracking = drawControllers;
			vrDevices[i]->UpdateDevice(hmd, vrTrackedDevices[i]);

			// Process device input
			switch (hmd->GetTrackedDeviceClass(i)) {
			case vr::TrackedDeviceClass_Controller:
				trackedControllers.push_back(vrDevices[i]);
				break;

			case vr::TrackedDeviceClass_HMD:
				vrDevices[i]->mVisible = !vrEnable; // don't draw the headset mesh when in VR
				//vrCamera->LocalPosition(mVRDevices[i]->DevicePosition());
				//vrCamera->LocalRotation(mVRDevices[i]->DeviceRotation());
				break;

			default:
				break;
			}
		}

		//vrInteraction->ProcessInput(scene, trackedControllers, mVolume, delta);
	}
	#pragma endregion
}
void Render() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	camera->Set();
	glClearColor(.25f, .25f, .25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sort(scene.begin(), scene.end(), [](const shared_ptr<Renderer>& a, const shared_ptr<Renderer>& b) {
		return a->mRenderQueue < b->mRenderQueue;
	});
	for (const auto& r : scene)
		r->Draw(*camera);

	// draw camera texture
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, windowWidth, windowHeight);
	
	GLuint bp = blitShader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, camera->ColorBuffer());
	Shader::Uniform(bp, "frameTexture", 0);
	
	screenQuad->BindVAO();
	glDrawElements(GL_TRIANGLES, screenQuad->ElementCount(), GL_UNSIGNED_INT, 0);

	glutSwapBuffers();
	glutPostRedisplay();

	// count fps

	static std::chrono::high_resolution_clock timer;
	static auto last = timer.now();

	static unsigned int fc = 0;
	static double ft = 0.0;

	auto n = timer.now();
	ft += (n - last).count() * 1e-9;
	last = n;

	fc++;
	if (ft > 1.0) {
		ft -= 1.0;
		printf("FPS: %u\n", fc);
		fc = 0;
	}
}