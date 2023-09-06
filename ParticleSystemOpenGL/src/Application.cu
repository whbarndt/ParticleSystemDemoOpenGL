#pragma once

/*
    Structure of Includes:
    - System/Language
    - Vendor/External Libs
    - Same Directory/Self Files
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/vec1.hpp"
#include "glm/gtc/constants.hpp"
#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Renderer.h"
#include "Shader.h"
#include "ParticleSystem.cuh"

#define GImGui MyImGuiTLS

// FIX: Structure of Arrays - Array of Structs - Memory access for threads better ?

// Update Particle Function - Will run on the GPU
__global__ void UpdateParticle(ParticleSystem::Particle* new_particles, float timestep)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;

    if (new_particles[i].life_rem <= 0.0f)
    {
        new_particles[i].alive = false;
    }
    new_particles[i].life_rem -= timestep;
    new_particles[i].vel.y = new_particles[i].vel.y + new_particles[i].acc * timestep;
    new_particles[i].pos += new_particles[i].pos + new_particles[i].vel * timestep;
}

int main()
{
    GLFWwindow* window;

    /* Initialize the library */
    glfwSetErrorCallback(GLFW_error);
    if (!glfwInit())
        return -1;

    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Particle Demo Scene", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    //Checks to make sure glew is initialized
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    //Gets version of OpenGL used
    std::cout << glGetString(GL_VERSION) << std::endl;

    //Defining my MVP matricies
    //Setting everything to normalized device coordinates for ease of numbers used
    glm::mat4 proj = glm::ortho(-3.0f , 3.0f, -3.0f, 3.0f, -1.0f, 1.0f);
    //Not chaning the view matrix
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    //Settting the model in the middle of the screen
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 mvp = proj * view * model;

    //Geometry of the Particles
    //Vertex Array
    float vertices[] = 
    {
        -0.15f, -0.25f, 0.0f,
         0.15f, -0.25f, 0.0f,
         0.15f,  0.25f, 0.0f,
        -0.15f,  0.25f, 0.0f
    };

    //Index Array
    unsigned int indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };
    
    //Buffer setup
    //Creating and binding the VAO
    unsigned int vao;
    GLCall(glCreateVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    //Creating and binding the VBO
    unsigned int geomVB, geomIB;
    GLCall(glCreateBuffers(1, &geomVB));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, geomVB));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    //Defining what the vertices consist of
    GLCall(glEnableVertexArrayAttrib(geomVB, 0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0));

    //Creating and binding the IBO
    GLCall(glCreateBuffers(1, &geomIB));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomIB));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    //Setup for Renderer
    Renderer renderer;
    renderer.EnableBlend();

    //Setup for Particle System
    unsigned int num_particles = 10000;
    ParticleSystem particlesystem(num_particles);
    int particles_per_frame = 1;
    particlesystem.setParticleShader("res/shaders/particle.shader");
    
    // CUDA program setup - Goal: 1 thread per particle
    int blocks = num_particles/100;
    int threads_per_block = particlesystem.getContainerSize() / blocks;

    // Using cudaMalloc and cudaMemcpy
    int pool_byte_size = particlesystem.getContainerSize() * sizeof(ParticleSystem::Particle) + 1;
    //cudaDeviceSetLimit(cudaLimitMallocHeapSize, pool_byte_size);
    // Pointer to Vector particle container
    ParticleSystem::Particle* cpu_bin = &particlesystem.particle_container[0];

    // Initialize gpu container
    ParticleSystem::Particle* gpu_bin;
    cudaMalloc((void**) &gpu_bin, pool_byte_size);

    // Using thrust library
    //thrust::device_vector<ParticleSystem::Particle> gpu_particle_container = particlesystem.getParticleContainer();
    //thrust::device_vector<ParticleSystem::Particle> gpu_particle_container(particlesystem.getParticleContainer().begin(), particlesystem.getParticleContainer().end());

    //Initializing particle properties
    ParticleProperties particleproperties;
    particleproperties.col_beg = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
    particleproperties.col_end = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
    particleproperties.size_beg = 0.25f;
    particleproperties.size_var = 0.05f;
    particleproperties.size_end = 0.0f;
    particleproperties.life = 10.0f;
    particleproperties.vel = { 0.0f, 0.7f };
    particleproperties.vel_var = { .1f, .05f };
    particleproperties.pos = { 0.0f, 0.0f };
    particleproperties.acc = -0.05f;

    // Setup Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGui::SetCurrentContext(MyImGuiTLS);  // ???????
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark(); 

    // Initializes times
    float time = (float)glfwGetTime();
    float lastframetime = 0;

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        //Render here

        //Operations to receieve my deltatime
        time = (float)glfwGetTime();
        float timestep = time - lastframetime;
        lastframetime = time;

        //Calls glClear(GL_COLOR_BUFFER_BIT)
        renderer.Clear();
        
        // Dear ImGui Call New Frame 
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        /// Particle System Loops ///

        // Emit Particle(s) Function (per frame)
        for (int i = 0; i < particles_per_frame; i++)
            particlesystem.Emit(particleproperties);

        //particlesystem.Update(timestep);

        /// CUDA ///
        
        // Multithreaded Physics Update Function 
        cudaMemcpy(gpu_bin, cpu_bin, pool_byte_size, cudaMemcpyHostToDevice);
        UpdateParticle<<<blocks,threads_per_block>>>(gpu_bin, timestep);
        cudaDeviceSynchronize();
        cudaMemcpy(cpu_bin, gpu_bin, pool_byte_size, cudaMemcpyDeviceToHost);

        // CUDA Error Checking
        cudaError_t error = cudaPeekAtLastError(); // check if kernel launch worked
        if (error == 0) error = cudaDeviceSynchronize(); // wait for GPU threads to finish
        if (error != 0) printf("CUDA Error %s (%d)", cudaGetErrorString(error), (int)error);
        
        // Render Update Function
        particlesystem.Render(mvp, vao);
        
        // Dear ImGui Active Components
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::DragFloat("Life Time", &particleproperties.life, 0.1f, 0.0f, 1000.0f);
        ImGui::DragInt("Particles Per Frame", &particles_per_frame, 1, 0, 50);

        // Dear ImGui Render Loop
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Swap front and back buffers
        glfwSwapBuffers(window);
        // Poll for and process events
        glfwPollEvents();
    }

    // cudaMalloc cleanup
    cudaFree(gpu_bin);

    //Night-Night time functions
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}