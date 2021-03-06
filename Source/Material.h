#pragma once

#include <string>
#include <Math\float3.h>
#include "ModuleProgram.h"
#include "Texture.h"

class Material
{
public:
	Material();
	~Material();

	void DrawImGui();

	std::string m_Name = "Material";
	unsigned int m_GUID = 0;

	Texture* m_DiffuseTexture = nullptr;
	Texture* m_SpecularTexture = nullptr;
	Texture* m_NormalMap = nullptr;

	float3 m_DiffuseColor = float3(0.3f);
	float3 m_SpecularColor = float3(1.0f);

	bool m_ShininessAlpha = false;
	float m_Shininess = 0.0f;
};
