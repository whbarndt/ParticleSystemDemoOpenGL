#pragma once

#include <vector>

#include "cuda.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "thrust/host_vector.h"
#include "thrust/device_vector.h"
#include "thrust/copy.h"

#include "Renderer.h"

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
	
	// Originally Private, but needed it for CUDA
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

	ParticleSystem();
	ParticleSystem(int container_size);
	std::vector<Particle> particle_container;					// Originally Private, but needed it for CudaMemcpy
	//Particle* particle_container = new Particle[];			// Probably would work but need to restructure my functions that rely on vector methods
	//Particle* particle_container;
	Shader getParticleShader();
	void setParticleShader(std::string path);
	int getContainerSize();
	//std::vector<Particle> getParticleContainer();		 
	//void Update(float time_step);
	void Render(glm::mat4 VP, unsigned int vao);
	void Emit(const ParticleProperties& particle_properties);

private:
	//thrust::host_vector<Particle> particle_container;			// Doesn't work since it wasn't copying the contents of the Particle elements in the particle_container vector
	//std::vector<Particle> particle_container;					// Would keep it private like I had in my original program, but I need to access it directly since I memcpy the data
	unsigned int container_size = 0;
	unsigned int container_index = 0;
	unsigned int lastUsedParticle = 0;
	unsigned int geometry = 0; 
	Shader particle_shader;
	unsigned int particle_shader_VP, particle_shader_T, particle_shader_Col = 0;

};