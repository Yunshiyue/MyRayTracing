#pragma once

#include "Walnut/Image.h"
#include <memory>
#include "glm/glm.hpp"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"


class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
	};
public:
	Renderer() = default;

	void Render(const Scene& scene, const Camera& camera);
	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return finalImage_; }
	Settings& GetSettings() { return settings_; }

	void ResetFrameIndex() { frameIndex_ = 1; }

private:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPostion;
		glm::vec3 WorldNormal;

		int ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y);  // RayGen

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistace, int objectIndex);
	HitPayload Miss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> finalImage_;
	uint32_t* imageData_ = nullptr;
	glm::vec4* accumulationData_ = nullptr;

	const Scene* activeScene_ = nullptr;
	const Camera* activeCamera_ = nullptr;

	uint32_t frameIndex_ = 1;
	Settings settings_;

	std::vector<uint32_t> imageHorizontalIter_, imageVerticalIter_;
};