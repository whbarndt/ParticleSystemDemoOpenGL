#pragma once

#include "Renderer.h"
#include <vector>

struct ParticleProperties
{
	glm::vec2 pos;
	glm::vec2 vel, vel_var;
	float acc;
	glm::vec4 col_beg, col_end;
	float size_beg, size_end, size_var;
	float life = 1.0f;
};

class ParticleSystem
{
public:
	
	ParticleSystem();
	Shader getParticleShader();
	void setParticleShader(std::string path);
	int getContainerSize();
	//int findAliveParticleIndex();
	void Update(float time_step);
	void Render(glm::mat4 VP, unsigned int vao);
	//void ImGuiRender();
	void Emit(const ParticleProperties& particle_properties);
	
private:
	
	struct Particle
	{
		glm::vec2 pos;
		glm::vec2 vel;
		float acc;
		glm::vec4 col_beg, col_end;
		float size_beg, size_end;

		float life = 1.0f;
		float life_rem = 0.0f;

		bool alive = false;
	};

	std::vector<Particle> particle_container;
	unsigned int container_size = 10000;
	unsigned int container_index = 9999;
	unsigned int lastUsedParticle = 0;
	unsigned int geometry = 0; 
	Shader particle_shader;
	unsigned int particle_shader_VP, particle_shader_T, particle_shader_Col = 0;
};