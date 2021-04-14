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
    glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    //Not chaning the view
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    //Settting the model in the middle of the screen
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 mvp = proj * view * model;

    //Setup for Particle System
    ParticleSystem particlesystem;
    ParticleProperties particleproperties;

    //Geometry of the Particles
    //Vertex Array
    float g_vertex_buffer_data[] =
    {
        -0.15f, -0.25f, 0.0f,
         0.15f, -0.25f, 0.0f,
         0.15f,  0.25f, 0.0f,
        -0.15f,  0.25f, 0.0f
    };

    //Buffer setup
    //Creating and binding the VAO
    unsigned int vao;
    GLCall(glCreateVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    GLuint billboard_vertex_buffer;
    glCreateBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    GLuint particles_position_buffer;
    glCreateBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, particlesystem.getContainerSize() * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    
    // The VBO containing the colors of the particles
    GLuint particles_color_buffer;
    glCreateBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, particlesystem.getContainerSize() * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    //Defining what the vertices consist of
    //GLCall(glEnableVertexArrayAttrib(geomVB, 0));
    //GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0));

    //Setup for Renderer
    Renderer renderer;
    renderer.EnableBlend();

    
    unsigned int particles_per_frame = 5;
    particlesystem.setParticleShader("res/shaders/particle.shader");

    //Initializing particle properties here
    particleproperties.col_beg = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
    particleproperties.col_end = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
    particleproperties.size_beg = 0.25f;
    particleproperties.size_var = 0.05f;
    particleproperties.size_end = 0.0f;
    particleproperties.life = 1.0f;
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
        //ImGui::Text("Hello World");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //ImGui::ColorEdit4("Birth Color", glm::value_ptr(particleproperties.col_beg));
        //ImGui::ColorEdit4("Death Color", glm::value_ptr(particleproperties.col_end));
        ImGui::DragFloat("Life Time", &particleproperties.life, 0.1f, 0.0f, 1000.0f);

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

/*
{
    //ImGui variables
    glm::vec3 translationA(400, 200, 0);
    glm::vec3 translationB(600, 200, 0);

    float r = 0.0f;
    float increment = 0.05f;
    \\\\\\\\\\\\\\\\\\\\\\\\\ Included outside renderloop ^^

    glm::mat4 model = glm::translate(glm::mat4(1.0f), translationA);
    glm::mat4 mvp = proj * view * model;
    shader.Bind();
    shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);
    shader.SetUniformMat4f("u_MVP", mvp);

    renderer.Draw(va, ib, shader);


    if (r > 1.0f)
        increment = -0.05f;
    else if (r < 0.0f)
        increment = 0.05f;

    r += increment;

    //ImGui
    {
        ImGui::SliderFloat3("Translation A", &translationA.x, 0.0f, 960.0f);
        ImGui::SliderFloat3("Translation B", &translationB.x, 0.0f, 960.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
}
    ////Mutiple test stuff

    test::Test* currentTest = nullptr;
    test::TestMenu* testmenu = new test::TestMenu(currentTest);
    currentTest = testmenu;

    testmenu->RegisterTest<test::TestClearColor>("Clear Color");

    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        if (currentTest)
        {
            currentTest->OnUpdate(0.0f);
            currentTest->OnRender();
            ImGui::Begin("Test");
            if (currentTest != testmenu && ImGui::Button("<-"))
            {
                delete currentTest;
                currentTest = testmenu;
            }

            currentTest->OnImGuiRender();
            ImGui::End();

        }

    delete currentTest;
    if (currentTest != testmenu)
        delete testmenu;
*/