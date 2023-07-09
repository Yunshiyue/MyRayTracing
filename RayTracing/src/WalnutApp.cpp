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
		{
			Sphere sphere;
			sphere.Position = { -0.5, -1.0, 0.0 };
			sphere.Albedo = { 1.0f, 1.0f, 0.0f };
			scene_.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.5, 0.0, 0.0 };
			sphere.Albedo = { 1.0f, 0.0f, 0.0f };
			scene_.Spheres.push_back(sphere);
		}
	}


	void OnUpdate(float ts) override
	{
		camera_.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last Render: %.3fms", lastRenderTime_);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::End();

		ImGui::Begin("Scene");
		for (size_t i = 0; i < scene_.Spheres.size(); i++)
		{
			ImGui::PushID(i);
			Sphere& sphere = scene_.Spheres[i];
			ImGui::DragFloat3("Postion", glm::value_ptr(sphere.Position), 1.0);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo), 1.0);
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