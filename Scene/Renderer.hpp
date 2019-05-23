#pragma once

class Camera;

class Renderer {
public:
	unsigned int mRenderQueue;
	virtual void Draw(Camera& camera) = 0;
};