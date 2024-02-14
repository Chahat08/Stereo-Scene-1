#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <random>
#include <vector>
#include <string>

#include "Shader.h"
#include "VertexData.h"
#include "Constants.h"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

int SCREEN_DIMENSION_WIDTH = 800;
float SCREEN_DIMENSION_HEIGHT = 0.3556f;

float FOV = 0.0f;

int CURSOR_XPOS = INT_MIN;
int CURSOR_YPOS = INT_MIN;

int NUM_CUBES = 16;

const float IPD = 0.5f; // 0.065
glm::vec3 HEAD_POSITION = glm::vec3(0.0, 0.0, -0.75); // let the head be 0.75 m from the screen
const bool RENDER_TOP_HALF = true;

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	if (CURSOR_XPOS == INT_MIN) {
		// set initial values
		CURSOR_XPOS = xpos;
		CURSOR_YPOS = ypos;
	}

	if (CURSOR_XPOS != xpos || CURSOR_YPOS != ypos)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void createModelMatrices(Shader& shader, std::vector<glm::vec3> positions, std::vector<glm::vec3> axes) {
	for (int i = 0; i < 1; ++i) {
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0, 0.0, 10.0));
		//model = glm::rotate(model, (float)glfwGetTime(), axes[i]);

		shader.setUniformMatrix4float("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 12 * 6);
	}
}

float camX = 0.0f, camZ = 0.0f;
void createViewMatrix(Shader& shader, float ipd, bool rightEye = false) {
	const float radius = 100.0f;
	camX = sin(glfwGetTime()) * radius + (rightEye ? ipd / 2.0 : -ipd / 2.0);
	camZ = cos(glfwGetTime()) * radius;
	glm::mat4 view(1.0f);
	view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	shader.setUniformMatrix4float("view", view);
}

void createHeadMatrix(Shader& shader, glm::vec3 translation, const float rotationAngle, glm::vec3 rotationAxis) {
	glm::mat4 head(1.0f);

	//head = glm::lookAt(HEAD_POSITION, 
	//	glm::vec3(camX, 0.0f, camZ), 
	//	glm::vec3(0.0f, 1.0f, 0.0f));

	shader.setUniformMatrix4float("head", head);
}

void createProjectionMatrix(Shader& shader, float near = 0.5f, float far = 100.0f, float fovDeg = 45.0f) {
	glm::mat4 projection(1.0f);

	float fov = glm::radians(2 * atan(SCREEN_DIMENSION_HEIGHT / (2 * 1.0f)));
	projection = glm::perspective(
		fov,
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
		0.1f,
		100.0f
	);

	std::cout << fov <<", "<<SCREEN_HEIGHT << std::endl;

	//projection = glm::frustum(

	//);


	shader.setUniformMatrix4float("projection", projection);
}

void createAllTransformationsAndEnableQuadBuffer(Shader& shader, float ipd, float near, float far, float fovDeg, std::vector<glm::vec3> positions, std::vector<glm::vec3> axes) {
	createProjectionMatrix(shader, near, far, fovDeg);

	// LEFT EYE
	glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	createViewMatrix(shader, ipd, false);
	createHeadMatrix(shader, HEAD_POSITION, 0.0, glm::vec3(0.0, 0.0, 0.0));
	createModelMatrices(shader, positions, axes);

	glFlush();

	// RIGHT EYE
	glDrawBuffer(GL_BACK_RIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	createViewMatrix(shader, ipd, true);
	createHeadMatrix(shader, HEAD_POSITION, 0.0, glm::vec3(0.0, 0.0, 0.0));
	createModelMatrices(shader, positions, axes);

	glFlush();
}

int main(int argc, char* argv[]) {

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_DECORATED, NULL); // to remove border and titlebar

	glfwWindowHint(GLFW_STEREO, GLFW_TRUE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

/*	SCREEN_WIDTH = mode->width; 
	SCREEN_HEIGHT = mode->height;*/	
	
	SCREEN_WIDTH = std::stoi(argv[1]);
	SCREEN_HEIGHT = std::stoi(argv[2]);
	SCREEN_DIMENSION_HEIGHT = std::stof(argv[3]);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Title", NULL, NULL);
	//GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Title", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // hide the cursor


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	std::cout << "vendor: " << vendor << std::endl;
	std::cout << "renderer: " << renderer << std::endl;

	/*if(RENDER_TOP_HALF) glViewport(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
	else glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2);*/

	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);

	// creating shaders
	Shader shader("VertexShaderSource.vert", "FragmentShaderSource.frag");

	// creating and sending vertex data
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositionsAndColors), &vertexPositionsAndColors, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);

	int width, height, nrChannels;
	unsigned char* texImageData = stbi_load("block.png", &width, &height, &nrChannels, 0);

	// MAKE MOVING CUBE SCENE

	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_real_distribution<> distribution_xy(-5.0, 5.0);
	std::uniform_real_distribution<> distribution_z(-5.0, 5.0);
	std::uniform_real_distribution<> distribution_axes(-2, 2);

	std::vector<glm::vec3> positions, axes;

	for (int i = 0; i < NUM_CUBES; ++i) {
		positions.push_back(glm::vec3(distribution_xy(engine), distribution_xy(engine), distribution_z(engine)));
		axes.push_back(glm::vec3(distribution_axes(engine), distribution_axes(engine), distribution_axes(engine)));
	}

	int frame = 0;
	float lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glBindVertexArray(VAO);

		createAllTransformationsAndEnableQuadBuffer(shader, IPD, 0.1f, 100.0f, 45.0, positions, axes);

		glfwSwapBuffers(window);
		glfwPollEvents();

		frame++;
		if (glfwGetTime() - lastTime > 1.0f) {
			std::cout << "Current FPS: " << frame << std::endl;
			frame = 0;
			lastTime += 1.0;
		}
	}

	glfwTerminate();

	return 0;
}