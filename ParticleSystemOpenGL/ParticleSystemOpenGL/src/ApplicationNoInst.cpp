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
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Renderer.h"
#include "Shader.h"
#include "ParticleSystem.h"

int main()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
    //Not chaning the view
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
    ParticleSystem particlesystem;
    ParticleProperties particleproperties;
    int particles_per_frame = 3;
    particlesystem.setParticleShader("res/shaders/particle.shader");

    //Initializing particle properties here
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

    //Setup for ImGui binding
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark(); // Setup style

    //Initializes times
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
        
        ImGui_ImplGlfwGL3_NewFrame();
        
        //Particle System loops
        particlesystem.Update(timestep);
        particlesystem.Render(mvp, vao);
        for (int i = 0; i < particles_per_frame; i++)
            particlesystem.Emit(particleproperties);
       
        //ImGui loops
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::DragFloat("Life Time", &particleproperties.life, 0.1f, 0.0f, 1000.0f);
        ImGui::DragInt("Particles Per Frame", &particles_per_frame, 1, 0, 50);

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Swap front and back buffers
        glfwSwapBuffers(window);
        // Poll for and process events
        glfwPollEvents();
    }

    //Night-Night time functions
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}