#include "ModuleCamera.h"

#include "Application.h"
#include "ModuleInput.h"
#include "SDL.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

ModuleCamera::ModuleCamera()
{
}

ModuleCamera::~ModuleCamera()
{
}

void ModuleCamera::ViewProjectionMatrix()
{
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
	frustum.SetViewPlaneDistances(zNear, zFar);
	frustum.SetVerticalFovAndAspectRatio(verticalFov, aspect);
	//TODO: setHoritzontal

	frustum.SetPos(eye);
	frustum.SetFront(rotationMatrix.WorldZ());
	frustum.SetUp(rotationMatrix.WorldY());

	view = float4x4(frustum.ViewMatrix());
	proj = frustum.ProjectionMatrix();
}

bool ModuleCamera::Init()
{
	eye = float3(0.0f, 4.0f, 8.0f);
	target = float3(0.0f, 0.0f, 0.0f);
	rotationMatrix = float3x3::FromEulerXYZ(DEGTORAD * -30.0f, DEGTORAD * 180.0f, 0.0f);

	//view = float4x4::LookAt(float3(0.0f, 4.0f, 8.0f), float3(0.0f, 0.0f, 0.0f), float3::unitY, float3::unitY);
	//LookAt(camera forward(right), target dir, local up, world up)

	aspect = SCREEN_WIDTH / SCREEN_HEIGHT;

	ViewProjectionMatrix();

	return true;
}

update_status ModuleCamera::Update()
{
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT))
	{
		int deltaX, deltaY;
		App->input->GetMouseMotion(deltaX, deltaY);

		/*if (App->input->GetKey(SDL_SCANCODE_W) || App->input->GetKey(SDL_SCANCODE_S) ||
			App->input->GetKey(SDL_SCANCODE_A) || App->input->GetKey(SDL_SCANCODE_D) ||
			App->input->GetKey(SDL_SCANCODE_Q) || App->input->GetKey(SDL_SCANCODE_E))
		{
			deltaX *= -1;
			deltaY *= -1;
		}*/

		if (App->input->GetKey(SDL_SCANCODE_LALT))
		{
			eye += rotationMatrix.WorldZ() * deltaY * App->GetDeltaTime() * mouseSpeedForRotation;
		}
		else
		{
			float3x3 rotationDeltaMatrix =
				float3x3::RotateAxisAngle(rotationMatrix.WorldX(), deltaY * App->GetDeltaTime() * mouseSpeedForRotation * DEGTORAD) *
				float3x3::RotateAxisAngle(float3::unitY, -deltaX * App->GetDeltaTime() * mouseSpeedForRotation * DEGTORAD);

			rotationMatrix = rotationDeltaMatrix * rotationMatrix;

			float multiplier = 1.0f;
			if (App->input->GetKey(SDL_SCANCODE_LSHIFT))
				multiplier = 3.0f;

			if (App->input->GetKey(SDL_SCANCODE_W))
				eye += rotationMatrix.WorldZ() * App->GetDeltaTime() * speed * multiplier;
			if (App->input->GetKey(SDL_SCANCODE_S))
				eye -= rotationMatrix.WorldZ() * App->GetDeltaTime() * speed * multiplier;
			if (App->input->GetKey(SDL_SCANCODE_A))
				eye += rotationMatrix.WorldX() * App->GetDeltaTime() * speed * multiplier;
			if (App->input->GetKey(SDL_SCANCODE_D))
				eye -= rotationMatrix.WorldX() * App->GetDeltaTime() * speed * multiplier;
			if (App->input->GetKey(SDL_SCANCODE_Q))
				eye += rotationMatrix.WorldY() * App->GetDeltaTime() * speed * multiplier;
			if (App->input->GetKey(SDL_SCANCODE_E))
				eye -= rotationMatrix.WorldY() * App->GetDeltaTime() * speed * multiplier;
		}

		ViewProjectionMatrix();
	}
	else if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE))
	{
		int deltaX, deltaY;
		App->input->GetMouseMotion(deltaX, deltaY);

		eye += rotationMatrix.WorldX() * deltaX * App->GetDeltaTime() * mouseSpeedForMovement +
			rotationMatrix.WorldY() * deltaY * App->GetDeltaTime() * mouseSpeedForMovement;

		ViewProjectionMatrix();
	}
	else if (App->input->MouseWheel()) 
	{
		int wheelX, wheelY;
		App->input->GetMouseWheel(wheelX, wheelY);

		eye += rotationMatrix.WorldZ() * wheelY * App->GetDeltaTime() * speed;

		ViewProjectionMatrix();
	}
	else if (App->input->GetKey(SDL_SCANCODE_O)) 
	{
		eye = float3::zero;

		ViewProjectionMatrix();
	}

	return UPDATE_CONTINUE;
}

void ModuleCamera::WindowResized(unsigned width, unsigned height)
{
	aspect = (float)width / (float)height;

	if (aspect >= 1.0f)
		verticalFov = DEGTORAD * initialVerticalFov;
	else
		verticalFov = math::Atan(math::Tan(DEGTORAD * initialVerticalFov) / aspect);

	ViewProjectionMatrix();
}

void ModuleCamera::DrawImGui()
{
	ImGui::Text("Variables");
	if (ImGui::DragFloat("Vertical FOV", &initialVerticalFov, 1.0f, 10.0f, 160.0f, "%.2f")) {
		if (aspect >= 1)
			verticalFov = DEGTORAD * initialVerticalFov;
		else
			verticalFov = math::Atan(math::Tan(DEGTORAD * initialVerticalFov) / aspect);
	}
	ImGui::DragFloat("Z-near", &zNear, 1.0f, 0.1f, 5.0f, "%.2f");
	ImGui::DragFloat("Z-far", &zFar, 5.0f, 6.0f, 400.0f, "%.2f");
	
	ImGui::Text("Vectors");
	ImGui::DragFloat("Eye-X", &eye.x, 1.0f, -25.0f, 25.0f, "%.2f");
	ImGui::DragFloat("Eye-Y", &eye.y, 1.0f, -25.0f, 25.0f, "%.2f");
	ImGui::DragFloat("Eye-Z", &eye.z, 1.0f, -25.0f, 25.0f, "%.2f");
	ImGui::Separator();

	ViewProjectionMatrix();
}
