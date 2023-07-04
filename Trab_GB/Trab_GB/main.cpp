#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <filesystem>
#include "Shader.h"
#include "Camera.h"
#include "PerlinNoise.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
const unsigned int NUM_PATCH_PTS = 4;
const int MAP_WIDTH = 5000;
const int MAP_HEIGHT = 5000;
float SCALE = 1.0f;
float PERSISTENCE = 1.5f;
int OCTAVES = 5;

glm::vec3 lightPos(0.0f, 20.0f, 0.0f);
// camera - give pretty starting point
Camera camera(glm::vec3(100.0f, 800.5f, 169.9f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -128.1f, -42.4f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
PerlinNoise perlin;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool isDragging = false;

std::random_device rd;  // Gera uma semente aleatória do dispositivo
std::mt19937 gen(rd());  // Cria um gerador de números aleatórios usando a semente
std::uniform_int_distribution<int> dist(1, 6);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: Terrain GPU", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
   if (glewInit() != GLEW_OK) {
        std::cout << "Failed no init GLEW." << std::endl;
        return EXIT_FAILURE;
    }

    GLint maxTessLevel;
    glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
    std::cout << "Max available tess level: " << maxTessLevel << std::endl;

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader tessHeightMapShader("8.3.gpuheight.vs", "8.3.gpuheight.fs",
        "8.3.gpuheight.tcs", "8.3.gpuheight.tes");

    Shader gNormalShader("geometryShader/9.3.normal_visualization.vs", "geometryShader/9.3.normal_visualization.fs", 
        nullptr, nullptr, "geometryShader/9.3.normal_visualization.gs");

    std::vector<float> vertices;

    unsigned rez = 40;
    for (unsigned i = 0; i <= rez - 1; i++)
    {
        for (unsigned j = 0; j <= rez - 1; j++)
        {
            vertices.push_back(-MAP_WIDTH / 2.0f + MAP_WIDTH * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-MAP_HEIGHT / 2.0f + MAP_HEIGHT * j / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-MAP_WIDTH / 2.0f + MAP_WIDTH * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-MAP_HEIGHT / 2.0f + MAP_HEIGHT * j / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-MAP_WIDTH / 2.0f + MAP_WIDTH * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-MAP_HEIGHT / 2.0f + MAP_HEIGHT * (j + 1) / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v

            vertices.push_back(-MAP_WIDTH / 2.0f + MAP_WIDTH * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-MAP_HEIGHT / 2.0f + MAP_HEIGHT * (j + 1) / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v
        }
    }
    std::cout << "Loaded " << rez * rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez * rez * 4 << " vertices in vertex shader" << std::endl;
    // first, configure the cube's VAO (and terrainVBO)
    unsigned int terrainVAO, terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // POSITION ATTRIBUTE
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // TEXCOORD ATTRIBUTE
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);


    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        tessHeightMapShader.use();
        tessHeightMapShader.setFloat("scale", SCALE);
        tessHeightMapShader.setFloat("persistence", PERSISTENCE);
        tessHeightMapShader.setInt("octaves", OCTAVES);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        tessHeightMapShader.setMat4("projection", projection);
        tessHeightMapShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        tessHeightMapShader.setMat4("model", model);

        // render the terrain
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS * rez * rez);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        SCALE += 0.001;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        SCALE -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        PERSISTENCE += 0.001;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        PERSISTENCE -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        OCTAVES = dist(gen);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever a key event occurs, this callback is called
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        default:
            break;
        }
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}