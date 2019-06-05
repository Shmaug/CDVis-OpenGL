#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <openvr.h>
#pragma warning(pop)

#include <unordered_map>

#ifdef WINDOWS
#include <Windows.h>
#include <gl/wglew.h>
#endif

#include "Scene/VRDevice.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Volume.hpp"
#include "Scene/MeshRenderer.hpp"
#include "Pipeline/AssetDatabase.hpp"
#include "Pipeline/Shader.hpp"
#include "Pipeline/Mesh.hpp"
#include "Pipeline/Texture.hpp"
#include "Util/FileBrowser.hpp"
#include "Util/ImageLoader.hpp"
#include "Util/Util.hpp"

using namespace std;
using namespace glm;

// glfw functions
void Resize(GLFWwindow*, int, int);
void Key(GLFWwindow*, int, int, int, int);
void Mouse(GLFWwindow*, int, int, int);
void MouseWheel(GLFWwindow*, double, double);
void MouseMove(GLFWwindow*, double, double);

// game functions
void InitScene();
void Update();
void Render();
void Cleanup();

GLFWwindow* gWindow;
ivec2 gWindowSize(1600, 900);

std::unordered_map<int, bool> gKeys;
bool gMouse[5];
vec2 gMousePos;

vr::IVRRenderModels* vrRenderModelInterface;
vr::IVRSystem* gHmd;
bool vrEnable;

bool gGizmoDraw = false;

shared_ptr<Camera> gCamera;
shared_ptr<Camera> gLeftEye;
shared_ptr<Camera> gRightEye;
vector<shared_ptr<Volume>> gVolumes;

vector<shared_ptr<VRDevice>> vrDevices;
unordered_map<string, shared_ptr<Mesh>> vrMeshes;
unordered_map<vr::TextureID_t, shared_ptr<Texture>> vrTextures;
vr::TrackedDevicePose_t vrTrackedDevices[vr::k_unMaxTrackedDeviceCount];

shared_ptr<Mesh> gScreenQuadMesh;

vector<shared_ptr<Object>> gScene;

void Error(int error, const char* desc) {
	printf("GLFW error %d: %s\n", error, desc);
}

int main(int argc, char** argv) {
	glfwSetErrorCallback(Error);

	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	gWindow = glfwCreateWindow(gWindowSize.x, gWindowSize.y, "CDVis", nullptr, nullptr);
	if (!gWindow) {
		printf("Failed to create window\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(gWindow);
	glfwSwapInterval(0);

	#pragma region print some info
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
	#pragma endregion

	glfwSetKeyCallback(gWindow, Key);
	glfwSetMouseButtonCallback(gWindow, Mouse);
	glfwSetCursorPosCallback(gWindow, MouseMove);
	glfwSetScrollCallback(gWindow, MouseWheel);
	glfwSetFramebufferSizeCallback(gWindow, Resize);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialize GLEW!\n");
		exit(EXIT_FAILURE);
	}

	memset(gMouse, 0, 5 * sizeof(bool));

	InitScene();

	while (!glfwWindowShouldClose(gWindow)) {
		if (gHmd) vr::VRCompositor()->WaitGetPoses(vrTrackedDevices, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		Update();
		Render();

		glfwSwapBuffers(gWindow);
		glfwPollEvents();
	}

	Cleanup();

	glfwDestroyWindow(gWindow);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

bool InitVR() {
	vr::EVRInitError eError = vr::VRInitError_None;
	gHmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
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
	gHmd->GetRecommendedRenderTargetSize(&w, &h);

	vec3 pos;
	quat rot;
	float l, r, t, b;

	// I have no clue why the eyes have to be swapped here

	VR2GL(gHmd->GetEyeToHeadTransform(vr::Eye_Right), pos, rot, true);
	gHmd->GetProjectionRaw(vr::Eye_Right, &l, &r, &t, &b);

	gLeftEye = shared_ptr<Camera>(new Camera());
	gLeftEye->Far(25.f);
	gLeftEye->LocalPosition(pos);
	gLeftEye->LocalRotation(rot);
	gLeftEye->PixelWidth(w);
	gLeftEye->PixelHeight(h);
	gLeftEye->PerspectiveBounds(vec4(l, r, -b, -t));
	gLeftEye->SampleCount(4);
	gScene.push_back(gLeftEye);

	VR2GL(gHmd->GetEyeToHeadTransform(vr::Eye_Left), pos, rot, true);
	gHmd->GetProjectionRaw(vr::Eye_Left, &l, &r, &t, &b);

	gRightEye = shared_ptr<Camera>(new Camera());
	gRightEye->Far(25.f);
	gRightEye->LocalPosition(pos);
	gRightEye->LocalRotation(rot);
	gRightEye->PixelWidth(w);
	gRightEye->PixelHeight(h);
	gRightEye->PerspectiveBounds(vec4(l, r, -b, -t));
	gRightEye->SampleCount(4);
	gScene.push_back(gRightEye);

	vrDevices.reserve(vr::k_unMaxTrackedDeviceCount);
	for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		vrDevices.push_back(nullptr);

	vrEnable = true;
	return true;
}

void InitScene() {
	AssetDatabase::LoadAssets();

	#pragma region screen quad mesh
	gScreenQuadMesh = shared_ptr<Mesh>(new Mesh());
	gScreenQuadMesh->BindVAO();

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

	gScreenQuadMesh->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, quadVertices, GL_STATIC_DRAW);

	gScreenQuadMesh->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, quadIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	gScreenQuadMesh->ElementCount(6);
	#pragma endregion

	auto v = shared_ptr<Volume>(new Volume());
	v->LocalScale(.5f, .5f, .5f);
	gVolumes.push_back(v);
	gScene.push_back(v);

	gCamera = shared_ptr<Camera>(new Camera());
	gCamera->SampleCount(4);
	gCamera->PixelWidth(gWindowSize.x);
	gCamera->PixelHeight(gWindowSize.y);
	gCamera->LocalPosition(0.f, 0.f, -1.25f);
	gScene.push_back(gCamera);

	if (!InitVR()) {
		printf("Failed to initialize OpenVR\n");
		gHmd = nullptr;
		vrEnable = false;
	}
}

void Cleanup() {
	AssetDatabase::Cleanup();

	gCamera.reset();
	gScene.clear();
	gScreenQuadMesh.reset();
	gVolumes.clear();
	vrTextures.clear();
	vrMeshes.clear();
	vrDevices.clear();
	if (gHmd) vr::VR_Shutdown();
}

#pragma region input callbacks
void MouseWheel(GLFWwindow* window, double x, double y) {
	gCamera->LocalPosition(gCamera->LocalPosition() + gCamera->WorldRotation() * vec3(0.f, 0.f, (float)y * .03f));
}
void MouseMove(GLFWwindow* window, double x, double y) {
	gMousePos.x = (float)x;
	gMousePos.y = (float)y;
}
void Mouse(GLFWwindow* window, int btn, int action, int mods) {
	gMouse[btn] = action == GLFW_PRESS;
}
void Key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		gKeys[key] = true;
		switch (key) {
		case GLFW_KEY_O: {
			#ifdef WINDOWS
			string folder = BrowseFolder(GetForegroundWindow());
			#endif
			if (folder.empty()) break;

			vec3 size;
			gVolumes[0]->Texture(ImageLoader::LoadVolume(folder, size));
			gVolumes[0]->LocalScale(size);

			break;
		}
		case GLFW_KEY_V:
			vrEnable = !vrEnable;
			break;
		case GLFW_KEY_G:
			gVolumes[0]->DisplaySampleCount(!gVolumes[0]->DisplaySampleCount());
			break;
		case GLFW_KEY_F:
			gGizmoDraw = !gGizmoDraw;
			break;
		}
	}
	else if (action == GLFW_RELEASE)
		gKeys[key] = false;
}
#pragma endregion

void Resize(GLFWwindow* window, int w, int h) {
	gWindowSize.x = w;
	gWindowSize.y = h;
	gCamera->PixelWidth(w);
	gCamera->PixelHeight(h);
}

void GetVRRenderModel(unsigned int index, MeshRenderer* renderer) {
	unsigned int len = gHmd->GetStringTrackedDeviceProperty(index, vr::TrackedDeviceProperty::Prop_RenderModelName_String, NULL, 0);
	char* name = new char[len];
	gHmd->GetStringTrackedDeviceProperty(index, vr::TrackedDeviceProperty::Prop_RenderModelName_String, name, len);

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

	vector<GLuint> indexBuffer((size_t)renderModel->unTriangleCount * (size_t)3);
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

	shared_ptr<Texture> texture = shared_ptr<Texture>(new Texture(renderModelDiffuse->unWidth, renderModelDiffuse->unHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, (void*)renderModelDiffuse->rubTextureMapData));
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
	vrDevices[index]->Shader(AssetDatabase::gTexturedShader);
	GetVRRenderModel(index, vrDevices[index].get());

	gScene.push_back(vrDevices[index]);
}

void DrawScene(Camera& camera, bool clear) {
	camera.Set();
	if (clear) {
		glClearColor(.25f, .25f, .25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	for (const auto& r : gScene)
		r->Draw(camera);
}
void DrawSceneGizmo(Camera& camera, bool clear) {
	camera.Set();
	if (clear) {
		glClearColor(.25f, .25f, .25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	for (const auto& r : gScene)
		if (&(*r) != &camera)
			r->DrawGizmo(camera);
}

void Update() {
	#pragma region Timekeeping
	static std::chrono::high_resolution_clock timer;
	static auto lastTime = timer.now();

	auto n = timer.now();
	double deltaTime = (n - lastTime).count() * 1e-9;
	lastTime = n;

	static unsigned int fc = 0;
	static double ft = 0.0;
	ft += deltaTime;
	fc++;
	if (ft > 1.0) {
		ft -= 1.0;
		printf("FPS: %u\n", fc);
		fc = 0;
	}
	#pragma endregion

	#pragma region PC controls
	static vec2 mouseLast;
	vec2 md = gMousePos - mouseLast;
	mouseLast = gMousePos;

	if (gMouse[GLFW_MOUSE_BUTTON_LEFT]) {
		vec3 axis;

		if (gKeys[GLFW_KEY_LEFT_SHIFT]) {
			vec2 v(gMousePos.x - gWindowSize.x / 2, gMousePos.y - gWindowSize.y / 2);
			v = vec2(-v.y, v.x);
			axis = gCamera->WorldRotation() * vec3(0.f, 0.f, 1.f) * dot(v, md);
		} else {
			axis = gCamera->WorldRotation() * vec3(0.f, 1.f, 0.f) * md.x + gCamera->WorldRotation() * vec3(1.f, 0.f, 0.f) * md.y;
		}

		if (length(axis) > .01f) {
			quat delta = angleAxis(length(md) * .003f, -normalize(axis));
			gVolumes[0]->LocalRotation(delta * gVolumes[0]->LocalRotation());
		}
	}

	if (gMouse[GLFW_MOUSE_BUTTON_RIGHT]) {
		vec3 mv(0.f);
		if (gKeys[GLFW_KEY_W]) mv += vec3(0.f, 0.f, 1.f);
		if (gKeys[GLFW_KEY_S]) mv -= vec3(0.f, 0.f, 1.f);
		if (gKeys[GLFW_KEY_A]) mv -= vec3(1.f, 0.f, 0.f);
		if (gKeys[GLFW_KEY_D]) mv += vec3(1.f, 0.f, 0.f);
		mv = gCamera->WorldRotation() * mv;
		if (gKeys[GLFW_KEY_LEFT_SHIFT]) mv *= 10.f;
		gCamera->LocalPosition(gCamera->LocalPosition() + mv * (float)deltaTime * .25f);

		static vec3 euler;
		euler += vec3(md.y, md.x, 0.f) * .0025f;
		euler.x = std::clamp(euler.x, -pi<float>() * .5f, pi<float>() * .5f);
		gCamera->LocalRotation(quat(euler));
	}

	if (gKeys[GLFW_KEY_Z]) gVolumes[0]->Threshold(gVolumes[0]->Threshold() - (float)deltaTime * .2f);
	if (gKeys[GLFW_KEY_X]) gVolumes[0]->Threshold(gVolumes[0]->Threshold() + (float)deltaTime * .2f);

	if (gKeys[GLFW_KEY_K]) gVolumes[0]->Exposure(gVolumes[0]->Exposure() - (float)deltaTime);
	if (gKeys[GLFW_KEY_L]) gVolumes[0]->Exposure(gVolumes[0]->Exposure() + (float)deltaTime);

	if (gKeys[GLFW_KEY_N]) gVolumes[0]->Density(gVolumes[0]->Density() - (float)deltaTime * .5f);
	if (gKeys[GLFW_KEY_M]) gVolumes[0]->Density(gVolumes[0]->Density() + (float)deltaTime * .5f);

	if (gKeys[GLFW_KEY_H]) gVolumes[0]->StepSize(gVolumes[0]->StepSize() - (float)deltaTime * .0002f);
	if (gKeys[GLFW_KEY_J]) gVolumes[0]->StepSize(gVolumes[0]->StepSize() + (float)deltaTime * .0002f);
	#pragma endregion
	
	#pragma region VR Controls
	static vector<shared_ptr<VRDevice>> trackedControllers;
	if (gHmd) {
		// Process SteamVR events (controller added/removed/etc)
		vr::VREvent_t event;
		while (gHmd->PollNextEvent(&event, sizeof(event))) {}

		// Gather input from tracked controllers and convert to VRController class

		bool drawControllers = !gHmd->IsSteamVRDrawingControllers();
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
			vrDevices[i]->UpdateDevice(gScene, gHmd, vrTrackedDevices[i]);

			// Process device input
			switch (gHmd->GetTrackedDeviceClass(i)) {
			case vr::TrackedDeviceClass_Controller:
				trackedControllers.push_back(vrDevices[i]);
				break;

			case vr::TrackedDeviceClass_HMD:
				vrDevices[i]->mVisible = !vrEnable; // don't draw the headset mesh when in VR

				gRightEye->Parent(vrDevices[i]);
				gLeftEye->Parent(vrDevices[i]);
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

	sort(gScene.begin(), gScene.end(), [](const shared_ptr<Object>& a, const shared_ptr<Object>& b) {
		return a->RenderQueue() < b->RenderQueue();
	});

	if (vrEnable && gHmd) {
		if (gGizmoDraw) {
			DrawSceneGizmo(*gLeftEye, true);
			DrawSceneGizmo(*gRightEye, true);
		}

		DrawScene(*gLeftEye, !gGizmoDraw);
		DrawScene(*gRightEye, !gGizmoDraw);

		gLeftEye->Resolve();
		gRightEye->Resolve();
	} else {
		if (gGizmoDraw) DrawSceneGizmo(*gCamera, true);
		DrawScene(*gCamera, !gGizmoDraw);
		gCamera->Resolve();
	}

	// draw camera texture
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, gWindowSize.x, gWindowSize.y);
	
	GLuint bp = AssetDatabase::gBlitShader->Use();
	Shader::Uniform(bp, "FrameTexture", 0);
	glActiveTexture(GL_TEXTURE0);

	gScreenQuadMesh->BindVAO();
	if (vrEnable && gHmd) {
		Shader::Uniform(bp, "ScaleOffset", vec4(.5f, 1.f, -.5f, 0.f));
		glBindTexture(GL_TEXTURE_2D, gLeftEye->ResolveColorBuffer());
		glDrawElements(GL_TRIANGLES, gScreenQuadMesh->ElementCount(), GL_UNSIGNED_INT, nullptr);

		Shader::Uniform(bp, "ScaleOffset", vec4(.5f, 1.f, .5f, 0.f));
		glBindTexture(GL_TEXTURE_2D, gRightEye->ResolveColorBuffer());
		glDrawElements(GL_TRIANGLES, gScreenQuadMesh->ElementCount(), GL_UNSIGNED_INT, nullptr);

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)gLeftEye->ResolveColorBuffer(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)gRightEye->ResolveColorBuffer(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	} else {
		Shader::Uniform(bp, "ScaleOffset", vec4(1.f, 1.f, 0.f, 0.f));
		glBindTexture(GL_TEXTURE_2D, gCamera->ResolveColorBuffer());
		glDrawElements(GL_TRIANGLES, gScreenQuadMesh->ElementCount(), GL_UNSIGNED_INT, nullptr);
	}
}