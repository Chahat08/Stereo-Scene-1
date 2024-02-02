#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <random>
#include <vector>

#include "Shader.h"
#include "VertexData.h"
#include "Constants.h"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

int CURSOR_XPOS = INT_MIN;
int CURSOR_YPOS = INT_MIN;

int NUM_CUBES = 16;

const float IPD = 0.25f;

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
	for (int i = 0; i < NUM_CUBES; ++i) {
		glm::mat4 model(1.0f);
		model = glm::translate(model, positions[i]);
		model = glm::rotate(model, (float)glfwGetTime(), axes[i]);

		shader.setUniformMatrix4float("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 12 * 6);
	}
}

void createViewMatrix(Shader& shader, float ipd, bool rightEye = false) {
	const float radius = 10.0f;
	float camX = sin(glfwGetTime()) * radius;
	float camZ = cos(glfwGetTime()) * radius;
	glm::mat4 view(1.0f);
	view = glm::lookAt(
		glm::vec3((rightEye ? camX += ipd / 2.0 : camX -= ipd / 2.0), 0.0f, camZ),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	shader.setUniformMatrix4float("view", view);
}

void createProjectionMatrix(Shader& shader, float near = 0.1f, float far = 100.0f, float fovDeg = 45.0f) {
	glm::mat4 projection(1.0f);

	projection = glm::perspective(
		glm::radians(45.0f),
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
		0.1f,
		100.0f
	);

	shader.setUniformMatrix4float("projection", projection);
}

void createAllTransformations(Shader& shader, float ipd, float near, float far, float fovDeg, std::vector<glm::vec3> positions, std::vector<glm::vec3> axes) {
	const float radius = 30.0f;
	float camX = sin(glfwGetTime()) * radius;
	float camZ = cos(glfwGetTime()) * radius;


	float fovby2 = glm::radians(fovDeg / 2.0f);

	float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
		left = -near * aspect, right = -left, top = near * tan(fovby2), bottom = -top;

	glm::mat4 view(1.0f);
	glm::mat4 projection(1.0f);

	// LEFT EYE
	glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = glm::lookAt(
		glm::vec3((camX -= ipd / 2.0), 0.0f, camZ),
		glm::vec3((-ipd / 2.0), 0.0f, -near),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	shader.setUniformMatrix4float("view", view);

	projection = glm::frustum(
		-(near * aspect * tan(fovby2) - ipd / 2.0f), // left
		(near * aspect * tan(fovby2) + ipd / 2.0f), // right
		bottom,
		top,
		near,
		far
	);
	shader.setUniformMatrix4float("projection", projection);

	createModelMatrices(shader, positions, axes);
	glFlush();

	// RIGHT EYE
	glDrawBuffer(GL_BACK_RIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = glm::lookAt(
		glm::vec3((camX += ipd / 2.0), 0.0f, camZ),
		glm::vec3((ipd / 2.0), 0.0f, -near),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	shader.setUniformMatrix4float("view", view);

	projection = glm::frustum(
		-(near * aspect * tan(fovby2) + ipd / 2.0f), // left
		(near * aspect * tan(fovby2) - ipd / 2.0f), // right
		bottom,
		top,
		near,
		far
	);
	shader.setUniformMatrix4float("projection", projection);

	createModelMatrices(shader, positions, axes);
	glFlush();
}

int main() {
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

	SCREEN_WIDTH = mode->width;
	SCREEN_HEIGHT = mode->height;

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Title", monitor, NULL);
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

	glm::mat4 view(1.0f);
	view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0f));

	int frame = 0;
	float lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0.0f, 0.027f, 0.212f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glBindVertexArray(VAO);

		if (frame++ % 2) 
			createViewMatrix(shader, IPD, false);
		else 
			createViewMatrix(shader, IPD, true);

		createProjectionMatrix(shader, 1.0f, 100.0f, 45.0f);
		createModelMatrices(shader, positions, axes);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetTime() - lastTime > 1.0f) {
			std::cout << "Current FPS: " << frame << std::endl;
			frame = 0;
			lastTime += 1.0;
		}
		//frame++;
	}

	glfwTerminate();

	return 0;
}