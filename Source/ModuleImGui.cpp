#include "Globals.h"
#include "Application.h"
#include "ModuleImGui.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleResources.h"
#include "ModuleWindow.h"
#include "ModuleTexture.h"
#include "ModuleCamera.h"
#include "CubeMap.h"
#include "SDL/include/SDL.h"
#include "GL/glew.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"
#include <imgui_internal.h>

ModuleImGui::ModuleImGui()
{
}

ModuleImGui::~ModuleImGui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	for (std::list<char*>::iterator it = Items.begin(); it != Items.end(); ++it)
	{
		free(*it);
	}

	delete[] license;
}

bool ModuleImGui::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplSDL2_InitForOpenGL(App->window->GetWindow(), App->renderer->GetContext());
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();

	inspector = true;
	hierarchy = true;
	resources = true;
	config = false;
	console = true;

	autoScroll = true;

	FILE* file = nullptr;
	fopen_s(&file, LICENSE_DIR, "rb");

	if (file)
	{
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		license = (char*)malloc(size + 1);
		fseek(file, 0, SEEK_SET);
		fread(license, 1, size, file);
		license[size] = 0;
		fclose(file);
	}

	return true;
}

void ModuleImGui::ToolbarUI()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuHeight));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarSize));
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoSavedSettings //| ImGuiWindowFlags_NoBackground
		;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::Begin("TOOLBAR", NULL, window_flags);
	ImGui::PopStyleVar(1);

	App->scene->DrawImGuiToolBar();
	ImGui::SameLine();

	ImGui::Spacing(); ImGui::SameLine();
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::Spacing(); ImGui::SameLine();

	Time::DrawImGuiToolBar();

	ImGui::End();
}

update_status ModuleImGui::PreUpdate()
{
	OPTICK_CATEGORY("ModuleImGui::PreUpdate", Optick::Category::UI);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);

	//dockerspace
	constexpr ImGuiWindowFlags dockspace_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + toolbarSize));// +menuHeight));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - toolbarSize));// -menuHeight));
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Dockspace", nullptr, dockspace_flags);
	ImGui::PopStyleVar(3);

	dockSpaceId = ImGui::GetID("DockspaceID");

	ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Github"))
				ShellExecute(0, 0, GITHUB, 0, 0, SW_SHOW);
			ImGui::MenuItem("About", NULL, &about);
			ImGui::MenuItem("Config", NULL, &config);
			if (ImGui::MenuItem("Quit"))
				return UPDATE_STOP;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Console", NULL, &console);
			ImGui::MenuItem("Inspector", NULL, &inspector);
			ImGui::MenuItem("Hierarchy (in progress)", NULL, &hierarchy);
			ImGui::MenuItem("Resources (in progress)", NULL, &resources);
			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();

	// Save off menu bar height for later.
	menuHeight = ImGui::GetCurrentWindow()->MenuBarHeight();

	ImGui::End();

	ToolbarUI();

	return UPDATE_CONTINUE;
}

update_status ModuleImGui::Update()
{
	OPTICK_CATEGORY("ModuleImGui::Update", Optick::Category::UI);

	if (console)
		Console(console);

	if (about)
		About(about);

	if (config)
	{
		if (ImGui::Begin("Config", &config))
		{
			Time::DrawImGui();
			App->camera->DrawImGui();
			App->window->DrawImGui();
			App->renderer->DrawImGui();
			App->scene->DrawImGui();
			SoftwareAndHardware();
		}
		ImGui::End();
	}

	if (resources)
	{
		if (ImGui::Begin("Resources", &resources))
		{
			App->resources->DrawImGuiResources();
		}
		ImGui::End();
	}

	if (inspector)
	{
		if (ImGui::Begin("Inspector", &inspector))
		{
			App->scene->DrawImGuiInspector();
		}
		ImGui::End();
	}

	if (hierarchy)
	{
		if (ImGui::Begin("Hierarchy", &hierarchy))
		{
			App->scene->DrawImGuiHierarchy();
		}
		ImGui::End();
	}

	return UPDATE_CONTINUE;
}

update_status ModuleImGui::PostUpdate()
{
	OPTICK_CATEGORY("ModuleImGui::PostUpdate", Optick::Category::UI);

	return UPDATE_CONTINUE;
}

void ModuleImGui::WrapMouseInWindow()
{
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	float2 viewportSize = float2(ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y);
	SDL_Window* window = App->window->GetWindow();

	if (mouseX > viewportSize.x)
		SDL_WarpMouseInWindow(window, 0, mouseY);
	else if (mouseX < 0)
		SDL_WarpMouseInWindow(window, viewportSize.x - 1, mouseY);
	else if (mouseY > viewportSize.y)
		SDL_WarpMouseInWindow(window, mouseX, 0);
	else if (mouseY < 0)
		SDL_WarpMouseInWindow(window, mouseX, viewportSize.y - 1);
}

void ModuleImGui::About(bool& show)
{
	if (ImGui::Begin("About", &show))
	{
		ImGui::Text("Engine name:");
		ImGui::Indent();
		ImGui::Text(TITLE);
		ImGui::Unindent();

		ImGui::Text("Authors:");
		ImGui::Indent();
		ImGui::Text("miquelmiro3 & bernatgb");
		ImGui::Unindent();

		ImGui::Text("Description:");
		ImGui::Indent();
		ImGui::Text("This is OurEngine");
		ImGui::Unindent();

		if (ImGui::Button("Github"))
			ShellExecute(0, 0, GITHUB, 0, 0, SW_SHOW);
		ImGui::SameLine();
		ImGui::Text(GITHUB);

		ImGui::Separator();

		ImGui::TextWrapped("License");

		ImGui::Indent();
		ImGui::TextWrapped(license);
		ImGui::Unindent();
	}
	ImGui::End();
}

void ModuleImGui::Console(bool& show)
{
	if (ImGui::Begin("Console", &show))
	{
		if (ImGui::SmallButton("Clear"))
		{
			for (std::list<char*>::iterator it = Items.begin(); it != Items.end(); ++it)
				free(*it);
			Items.clear();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &autoScroll);

		ImGui::Separator();

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -10), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (std::list<char*>::iterator it = Items.begin(); it != Items.end(); ++it)
		{
			ImVec4 color;
			bool has_color = false;
			if (strstr((*it), "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
			if (has_color)
				ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted((*it));
			if (has_color)
				ImGui::PopStyleColor();
		}

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(0.0f);

		ImGui::EndChild();

	}
	ImGui::End();
}

void ModuleImGui::SoftwareAndHardware()
{
	if (ImGui::CollapsingHeader("About Software & Hardware"))
	{
		ImGui::TextWrapped("Libraries");

		ImGui::BulletText("Assimp v 143");
		ImGui::BulletText("DevIL v 1.8.0");
		ImGui::BulletText("Glew v 2.1.0");
		ImGui::BulletText("ImGui v 1.86 WIP");
		ImGui::BulletText("ImGuiFileDialog v 0.6.3");
		ImGui::BulletText("ImGuizmo v 1.83");
		ImGui::BulletText("MathGeoLib v 1.5");
		ImGui::BulletText("RapidJSON v 1.1.0");
		ImGui::BulletText("Optick v 1.3.2.0");
		ImGui::BulletText("SDL v 2.0.16");

		ImGui::Separator();

		ImGui::TextWrapped("Hardware");

		SDL_version version;
		SDL_GetVersion(&version);
		int vram_free, vram_budget;
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &vram_free);
		glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &vram_budget);
		float vram_budget_mb = vram_budget / 1024.0f;
		float vram_free_mb = vram_free / 1024.0f;
		float vram_usage_mb = vram_budget_mb - vram_free_mb;

		//ImGui::Text("SDL Version: %d.%d.%d", version.major, version.minor, version.patch);
		ImGui::BulletText("CPUs: %d", SDL_GetCPUCount());
		ImGui::BulletText("System RAM: %.1f Gb", SDL_GetSystemRAM() / 1024.0f);
		ImGui::BulletText("GPU: %s", (unsigned char*)glGetString(GL_RENDERER));
		ImGui::BulletText("Brand: %s", (unsigned char*)glGetString(GL_VENDOR));
		ImGui::BulletText("VRAM Budget: %.1f Mb", vram_budget_mb);
		ImGui::BulletText("Vram Usage: %.1f Mb", vram_usage_mb);
		ImGui::BulletText("Vram Avaliable: %.1f Mb", vram_free_mb);
	}
}
