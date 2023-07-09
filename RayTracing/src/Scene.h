#pragma once
#include "glm/glm.hpp"
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metallilc = 0.0f;

	glm::vec3 EmmisionColor{ 0.0f };
	float EmmisionPower = 0.0f;

	glm::vec3 GetEmmision() const { return EmmisionColor * EmmisionPower; }
};

struct Sphere
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f};
	float Radius = 0.5f;

	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};

