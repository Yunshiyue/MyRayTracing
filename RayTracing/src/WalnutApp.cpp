#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"
#include "Camera.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: camera_(45.0f, 0.1f, 100.0f)
	{ 
		Material& pinkSphere = scene_.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.0f;
		pinkSphere.Metallilc = 0.0f;

		Material& blueSphere = scene_.Materials.emplace_back();
		blueSphere.Albedo = { 0.2f, 0.3f, 1.0f };
		blueSphere.Roughness = 0.3f;
		blueSphere.Metallilc = 0.0f;

		Material& orangeSphere = scene_.Materials.emplace_back();
		orangeSphere.Albedo = { 0.8f, 0.5f, 0.2f };
		orangeSphere.Roughness = 0.1f;
		orangeSphere.Metallilc = 0.0f;
		orangeSphere.EmmisionColor = orangeSphere.Albedo;
		orangeSphere.EmmisionPower = 2.0f;

		{
			Sphere sphere;
			sphere.Position = { 0.0, 0.0, 0.0 };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;
			scene_.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.5, -101.0, 0.0 };
			sphere.Radius = 100.0f;
			sphere.MaterialIndex = 1;
			scene_.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 3.0, 0.0, 0.0 };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 2;
			scene_.Spheres.push_back(sphere);
		}
	}


	void OnUpdate(float ts) override
	{
		if (camera_.OnUpdate(ts))
		{
			renderer_.ResetFrameIndex();
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last Render: %.3fms", lastRenderTime_);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Checkbox("Accumulate", &renderer_.GetSettings().Accumulate);
		if (ImGui::Button("Reset"))
		{
			renderer_.ResetFrameIndex();
		}
		ImGui::End();

		ImGui::Begin("Scene");
		for (size_t i = 0; i < scene_.Spheres.size(); i++)
		{
			ImGui::PushID(i);
			Sphere& sphere = scene_.Spheres[i];
			ImGui::DragFloat3("Postion", glm::value_ptr(sphere.Position), 1.0);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0, static_cast<int>(scene_.Materials.size() - 1));
			ImGui::PopID();
		}

		for (size_t i = 0; i < scene_.Materials.size(); i++)
		{
			ImGui::PushID(i);
			Material& mat = scene_.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(mat.Albedo), 1.0);
			ImGui::DragFloat("Roughness", &mat.Roughness, 0.1, 0.0, 1.0);
			ImGui::DragFloat("Metallic", &mat.Metallilc, 0.1, 0.0, 1.0);
			ImGui::ColorEdit3("Emmision Color", glm::value_ptr(mat.EmmisionColor), 1.0);
			ImGui::DragFloat("Emmision Power", &mat.EmmisionPower, 0.1, 0.0, FLT_MAX);
			ImGui::PopID();
		}
		
		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		viewportWidth_ = ImGui::GetContentRegionAvail().x;
		viewportHeight_ = ImGui::GetContentRegionAvail().y;

		auto image = renderer_.GetFinalImage();
		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(), 
				{ static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight()) },
				ImVec2(0, 1), ImVec2(1, 0) );
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
		//ImGui::ShowDemoWindow();
	}

	void Render()
	{
		Timer timer;

		// renderer resize
		renderer_.OnResize(viewportWidth_, viewportHeight_);
		camera_.OnResize(viewportWidth_, viewportHeight_);  // cache ray directions
		// renderer render
		renderer_.Render(scene_, camera_);
		

		lastRenderTime_ = timer.ElapsedMillis();
	}
private:
	uint32_t viewportWidth_;
	uint32_t viewportHeight_;
	Renderer renderer_;
	Camera camera_;
	Scene scene_;
	float lastRenderTime_;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}