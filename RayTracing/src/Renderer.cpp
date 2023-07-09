#include "Renderer.h"

#include "Walnut/Random.h"
#include <execution>

namespace Utils
{
	static uint32_t ConverToRGBA(const glm::vec4& color)
	{
		uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
		uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
		uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
		uint8_t a = static_cast<uint8_t>(color.a * 255.0f);
		return (a << 24) | (b << 16) | (g << 8) | r;
	}
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	activeScene_ = &scene;
	activeCamera_ = &camera;

	if (frameIndex_ == 1)
	{
		memset(accumulationData_, 0, sizeof(glm::vec4) * finalImage_->GetHeight() * finalImage_->GetWidth());
	}

#define MT 1
#if MT
	std::for_each(std::execution::par, imageVerticalIter_.begin(), imageVerticalIter_.end(), [this](uint32_t y)
		{
			std::for_each(std::execution::par, imageHorizontalIter_.begin(), imageHorizontalIter_.end(), [this, y](uint32_t x)
				{
					auto color = PerPixel(x, y);
					accumulationData_[x + y * finalImage_->GetWidth()] += color;

					glm::vec4 accumulatedColor = accumulationData_[x + y * finalImage_->GetWidth()];
					accumulatedColor /= static_cast<float>(frameIndex_);

					color = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					imageData_[x + y * finalImage_->GetWidth()] = Utils::ConverToRGBA(color);
				});
		});
#else
	// y写在外层，cache快
	for (uint32_t y = 0; y < finalImage_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < finalImage_->GetWidth(); x++)
		{
			auto color = PerPixel(x, y);
			accumulationData_[x + y * finalImage_->GetWidth()] += color;

			glm::vec4 accumulatedColor = accumulationData_[x + y * finalImage_->GetWidth()];
			accumulatedColor /= static_cast<float>(frameIndex_);

			color = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			imageData_[x + y * finalImage_->GetWidth()] = Utils::ConverToRGBA(color);
		}
	}
#endif // MT

	finalImage_->SetData(imageData_);

	if (settings_.Accumulate)
	{
		frameIndex_++;
	}
	else
	{
		frameIndex_ = 1;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (finalImage_)
	{
		if (finalImage_->GetWidth() == width && finalImage_->GetHeight())
		{
			return;
		}

		finalImage_->Resize(width, height);
	}
	else
	{
		finalImage_ = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] imageData_;
	imageData_ = new uint32_t[width * height];

	delete[] accumulationData_;
	accumulationData_ = new glm::vec4[width * height];

	ResetFrameIndex();

	imageHorizontalIter_.resize(width);
	imageVerticalIter_.resize(height);
	for (uint32_t i = 0; i < width; i++)
	{
		imageHorizontalIter_[i] = i;
	}
	for (uint32_t i = 0; i < height; i++)
	{
		imageVerticalIter_[i] = i;
	}
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// t = hit distance
	// r = radius


	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();
	
	for (size_t i = 0; i < activeScene_->Spheres.size(); i++)
	{
		const auto& sphere = activeScene_->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0)	// 没有交点
		{
			continue;
		}

		//float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);  // 最近点
		if (closestT > 0.0f && closestT < hitDistance)		// 最近点不能在相机后面
		{
			hitDistance = closestT;
			closestSphere = static_cast<int>(i);
		}
	}

	if (closestSphere < 0)
	{
		return Miss(ray);
	}
	
	return ClosestHit(ray, hitDistance, closestSphere);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)  // RayGen
{
	Ray ray;
	ray.Origin = activeCamera_->GetPosition();
	ray.Direction = activeCamera_->GetRayDirections()[x + y * finalImage_->GetWidth()];

	glm::vec3 light(0.0f);	// 最终颜色
	glm::vec3 contribution(1.0f);

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.6, 0.7, 0.9);
			//glm::vec3 skyColor = glm::vec3(0.0, 0.0, 0.0);
			//light += skyColor * contribution;
			break;
		}

		//glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0, -1.0, -1.0));
		//float lightIntensity = glm::max(glm::dot(-lightDir, payload.WorldNormal), 0.0f);	// 漫反射

		const Sphere& sphere = activeScene_->Spheres[payload.ObjectIndex];
		const Material& material = activeScene_->Materials[sphere.MaterialIndex];
		/*glm::vec3 sphereColor = material.Albedo;
		sphereColor = material.Albedo * lightIntensity;*/
		//light += material.Albedo * contribution;

		contribution *= material.Albedo;
		light += material.GetEmmision();
		// 反射，改变光的方向
		ray.Origin = payload.WorldPostion + payload.WorldNormal * 0.0001f;
		/*ray.Direction = glm::reflect(ray.Direction, 
			payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5, 0.5));*/
		ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
	}

	return { light, 1.0 };
}



Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = activeScene_->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;		// 相对移动，把球位移到原点
	payload.WorldPostion = origin + ray.Direction * hitDistance;	// a + bt
	payload.WorldNormal = glm::normalize(payload.WorldPostion);		// 球中心在原点
	payload.WorldPostion += closestSphere.Position;	// 加上偏移，还原位置
	
	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}