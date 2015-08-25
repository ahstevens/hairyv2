#include "Study.h"
#include <glm/gtc/type_ptr.hpp>

// Initialize class variables
Study* Study::instance = NULL;

// Returns singleton Study instance
Study* Study::getInstance( GLFWwindow* window )
{
    if ( !instance )
        instance = new Study( window );
    return instance;
}

Study::Study( GLFWwindow* window )
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	this->window = window;
	firstMouse = true;
	lastX  =  width  / 2.0f;
    lastY  =  height / 2.0f;
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;  	// Time of last frame

	draw_normals = 0;

	camera = Camera();
	light = Light(glm::vec3(1.0, 1.0, 1.0));

	// initialize key array
	for (int i = 0; i < 1024; ++i)
		keys[i] = 0;
}

Study::~Study()
{
}

void Study::init(GLfloat width_mm, GLfloat height_mm, GLfloat dist_mm)
{
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


    // Build and compile our shader program
    Shader lightingShader("materials.vs", "materials.frag");
    Shader normalShader("normals.vs", "normals.frag", "normals.gs");

	camera = Camera(glm::vec3(0.0f, 0.0f, dist_mm), width_mm, height_mm, dist_mm, dist_mm, 1000.0f);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	std::cout << "Generating trial..." << std::endl;
	trial = Trial(10, 10, 1.0f, 0.25f);
	trial.init();
	std::cout << "Trial generated" << std::endl;

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = (GLfloat) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();

        // Clear the colorbuffer
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Use cooresponding shader when setting uniforms/drawing objects
        lightingShader.Use();

		glm::vec4 lightPos = light.getPosition();
		glUniform4f(glGetUniformLocation(lightingShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z, lightPos.w);
		glm::vec3 cameraPos = camera.getPosition();
		glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        // Set lights properties
		//glm::vec3 lightColor;
		//lightColor.x = 1.0f; //sin(glfwGetTime() * 2.0f);
		//lightColor.y = 1.0f; //sin(glfwGetTime() * 0.7f);
		//lightColor.z = 1.0f; //sin(glfwGetTime() * 1.3f);
				
		//s->setColor(sin(glfwGetTime() * 2.0f), sin(glfwGetTime() * 0.7f), sin(glfwGetTime() * 1.3f));
		//light.setPosition(cos(glfwGetTime()), sin(glfwGetTime()), 1.0f);

		//light.setColor( 1.0f, 1.0f, 1.0f );
		glm::vec3 ambientColor = light.getAmbientColor();
		glm::vec3 diffuseColor = light.getDiffuseColor();
        glm::vec3 specularColor = light.getSpecularColor(); 
        glUniform3f(glGetUniformLocation(lightingShader.Program, "light.ambient"),  ambientColor.r, ambientColor.g, ambientColor.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "light.diffuse"),  diffuseColor.r, diffuseColor.g, diffuseColor.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "light.specular"), specularColor.r, specularColor.g, specularColor.b);

        // Create camera transformations
        glm::mat4 view = camera.getViewMatrix();
		//view = glm::translate(view, glm::vec3(-width_mm*0.5f, -height_mm*0.5f, 0));
		glm::mat4 projection = camera.getProjectionMatrix();
        // Get the uniform locations
        GLint viewLoc  = glGetUniformLocation(lightingShader.Program,  "view");
        GLint projLoc  = glGetUniformLocation(lightingShader.Program,  "projection");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		//s->redraw(lightingShader);

		//if(draw_normals)
		//{
		//	normalShader.Use();
		//	glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
		//	glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//	// And draw model again, this time only drawing normal vectors using the geometry shaders (on top of previous model)
		//	s->redraw(normalShader);
		//}

		trial.display(lightingShader);

		if(draw_normals)
		{
			normalShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			// And draw model again, this time only drawing normal vectors using the geometry shaders (on top of previous model)
			trial.display(normalShader);
		}


        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
}

void Study::begin()
{

}

void Study::next()
{

}

void Study::end()
{

}


void Study::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	getInstance(window)->key_process(window, key, scancode, action, mode);
}

// Is called whenever a key is pressed/released via GLFW
void Study::key_process(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
			if (keys[GLFW_KEY_N])
				draw_normals = abs(draw_normals - 1);
			if (keys[GLFW_KEY_R]) {
				trial = Trial(67, 50, 1.0f, 0.25f);
				trial.init();
			}
		}
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void Study::do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.processKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.processKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.processKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.processKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_K])
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (keys[GLFW_KEY_L])
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void Study::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	getInstance(window)->mouse_process(window, xpos, ypos);
}

void Study::mouse_process(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (GLfloat) xpos;
        lastY = (GLfloat) ypos;
        firstMouse = false;
    }

    GLfloat xoffset = (GLfloat) xpos - lastX;
    GLfloat yoffset = lastY - (GLfloat) ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = (GLfloat) xpos;
    lastY = (GLfloat) ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void Study::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	getInstance(window)->scroll_process(window, xoffset, yoffset);
}

void Study::scroll_process(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll((GLfloat) yoffset);
}