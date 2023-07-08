#include "Renderer.h"

#include "Walnut/Random.h"


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

void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();
	// y写在外层，cache快
	for (uint32_t y = 0; y < finalImage_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < finalImage_->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * finalImage_->GetWidth()];

			auto color = TraceRay(ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			imageData_[x + y * finalImage_->GetWidth()] = Utils::ConverToRGBA(color);
		}
	}

	finalImage_->SetData(imageData_);
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

}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// t = hit distance
	// r = radius

	float radius = 0.5f;
	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(ray.Origin, ray.Direction);
	float c = glm::dot(ray.Origin, ray.Origin) - radius * radius;

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0)
	{
		return {0.0, 0.0, 0.0, 1.0};
	}
	glm::vec3 sphereColor(1, 0, 1);

	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);  // 较小

	
	glm::vec3 hitPosition = ray.Origin + ray.Direction * closestT;

	glm::vec3 normal = glm::normalize(hitPosition);

	glm::vec3 lightDir(-1.0, -1.0, -1.0);

	float d = glm::max(glm::dot(-lightDir, normal), 0.0f);	// 漫反射
	sphereColor *= d;
	return { sphereColor, 1.0f };
	
	//return 0xff000000 | (g << 8) | r;
}
