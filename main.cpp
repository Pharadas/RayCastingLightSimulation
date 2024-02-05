#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h>

#include <shader_m.h>
#include <shader_c.h>
#include <camera.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow *window);
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// texture size
const unsigned int TEXTURE_WIDTH = 150, TEXTURE_HEIGHT = 150;

// camera
Camera camera(glm::vec3(25.0f, 25.0f, 25.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing 
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

glm::vec3 rotate3D(glm::vec3 v, glm::vec2 rotation) {
    float cos_a_x = cos(rotation.y);
    float sin_a_x = sin(rotation.y);

    float cos_a_y = cos(rotation.x);
    float sin_a_y = sin(rotation.x);

    glm::vec3 temp_vec = glm::vec3(
        v.x,
        v.y * cos_a_x - v.z * sin_a_x,
        v.y * sin_a_x + v.z * cos_a_x
    );

    return glm::vec3(
        temp_vec.x * cos_a_y + temp_vec.z * sin_a_y,
        temp_vec.y,
        -temp_vec.x * sin_a_y + temp_vec.z * cos_a_y
    );
}

int main(int argc, char* argv[]) {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
	glfwSwapInterval(0);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// query limitations
	// -----------------
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}	
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;

	// build and compile shaders
	// -------------------------
	Shader screenQuad("screen_quad.vs", "screen_quad.fs");
	ComputeShader computeShader("compute_shader.comp");

	screenQuad.use();
	screenQuad.setInt("tex", 0);

	// Create texture for opengl operation
	// -----------------------------------
	unsigned int texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

    float globalTime = 0;
	// render loop
	// -----------
	int fCounter = 0;
	while (!glfwWindowShouldClose(window)) {
		// Set frame time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
        globalTime += deltaTime;

        processInput(window);

		if(fCounter > 100) {
			std::cout << "FPS: " << 1 / deltaTime << "\r";
			fCounter = 0;
		} else {
			fCounter++;
		}

		computeShader.use();
		computeShader.setFloat("t", globalTime);
        /* std::cout << globalTime; */
		computeShader.setVec2("viewportDimensions", glm::vec2(TEXTURE_WIDTH, TEXTURE_HEIGHT));
		computeShader.setVec3("position", camera.Position);
		computeShader.setVec2("rotation", glm::vec2(camera.Yaw, -camera.Pitch));

		glDispatchCompute((unsigned int) TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// render image to quad
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenQuad.use();
		
		renderQuad();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteTextures(1, &texture);
	glDeleteProgram(screenQuad.ID);
	glDeleteProgram(computeShader.ID);

	glfwTerminate();

	return EXIT_SUCCESS;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    glm::vec3 relative_vel(0, 0, 0);
    float speed = 5;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        relative_vel.z += speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        relative_vel.z -= speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        relative_vel.x -= speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        relative_vel.x += speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        relative_vel.y += speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        relative_vel.y -= speed * deltaTime;

    camera.Position += rotate3D(relative_vel, glm::vec2(camera.Yaw, -camera.Pitch));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
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
