#include "System.h"

System::System()
{
}


System::~System()
{
}

int System::GLFWInit()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(WIDTH, HEIGHT, "TrabGA", nullptr, nullptr);

	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed no init GLEW." << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	return EXIT_SUCCESS;

}

int System::OpenGLSetup()
{

	glEnable(GL_BLEND);	// Enables blending ( glBlendFunc )
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	return EXIT_SUCCESS;
}

int System::SystemSetup()
{
	cout << "deuSETUP?";
	coreShader = Shader("Shaders/Core/core.vert", "Shaders/Core/core.frag");
	coreShader.Use();

	return EXIT_SUCCESS;
}

void System::Run()
{
	cout << "deuRUN?";

	coreShader.Use();


	OBJ modelo;
	LeitorOBJ leitorOBJ;

	modelo.carregarArquivo("cube.obj");


	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	Camera camera(cameraPosition, cameraTarget, cameraUp);

	glm::mat4 viewMatrix = camera.GetViewMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(viewMatrix));

	coreShader.Use();

	// Criação do buffer de vértices
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, modelo.getVertices().size() * sizeof(Vertex), &modelo.getVertices()[0], GL_STATIC_DRAW);

	// Criação do buffer de índices
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelo.getIndices().size() * sizeof(unsigned int), &modelo.getIndices()[0], GL_STATIC_DRAW);

	// Criação do Vertex Array Object (VAO)
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);


	while (!glfwWindowShouldClose(window)) 
	{

		glfwPollEvents();

#pragma region Input Handling

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

#pragma endregion

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		coreShader.Use();

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, modelo.getIndices().size(), GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
	}


}

void System::Finish()
{
	coreShader.Delete();

	glfwTerminate();
}
