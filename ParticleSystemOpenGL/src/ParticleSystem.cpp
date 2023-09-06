#include "glm/gtc/constants.hpp"
#include "glm/gtx/compatibility.hpp"

#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
	container_size = 10000;
	container_index = 9999;
	particle_container.resize(container_size);
}

ParticleSystem::ParticleSystem(int container_size)
{
	particle_container.resize(container_size);
	this->container_size = container_size;
	container_index = container_size - 1;
}

Shader ParticleSystem::getParticleShader()
{
	return particle_shader;
}

void ParticleSystem::setParticleShader(std::string path)
{
	particle_shader.SetShader(path);
}

int ParticleSystem::getContainerSize()
{
	return container_size;
}

void ParticleSystem::Update(float timestep)
{
	for (auto& particle : particle_container)
	{
		if (!particle.alive)
			continue;

		if (particle.life_rem <= 0.0f)
		{
			particle.alive = false;
			continue;
		}

		particle.life_rem -= timestep;
		particle.vel.y += particle.acc * timestep;
		particle.pos += particle.vel * timestep;
	}
}

void ParticleSystem::Render(glm::mat4 VP, unsigned int vao)
{	
	particle_shader.Bind();
	particle_shader.SetUniformMat4fv("uni_MVP", VP);

	for (auto& particle : particle_container)
	{
		if (!particle.alive)
			continue;

		//Fade away particles
		float life = particle.life_rem / particle.life;
		//Lerps between the begining color and end color and changes its color based upon it
		glm::vec4 color = glm::lerp(particle.col_end, particle.col_beg, life);
		color.a = color.a * life;
		//Lerps between the sizes so the particles go to zero as they die
		float size = glm::lerp(particle.size_end, particle.size_beg, life);

		// Render
		glm::mat4 transform = glm::translate(glm::mat4(10.0f), { particle.pos.x, particle.pos.y, 0.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });
		particle_shader.SetUniformMat4fv("uni_Transform", transform);
		particle_shader.SetUniform4fv("uni_Color", color);
	
		GLCall(glBindVertexArray(vao));
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
	}
}

void ParticleSystem::Emit(const ParticleProperties& particle_properties)
{
	Particle& particle = particle_container[container_index];
	particle.alive = true;
	particle.pos = particle_properties.pos;

	//Setting Velocity
	particle.vel = particle_properties.vel;
	particle.vel.x += particle_properties.vel_var.x * ((std::rand() % 10) + -5);
	particle.vel.y += particle_properties.vel_var.y * ((std::rand() % 10) + -5);

	//Setting Acceleration
	particle.acc = particle_properties.acc;

	//Setting Color
	particle.col_beg = particle_properties.col_beg;
	particle.col_end = particle_properties.col_end;

	//Setting Life
	particle.life = particle_properties.life;
	particle.life_rem = particle_properties.life;

	//Setting Size
	particle.size_beg = particle_properties.size_beg + particle_properties.size_var * (std::rand() % 5);
	particle.size_end = particle_properties.size_end;

	lastUsedParticle = container_index;
	container_index = --container_index % container_size;
}