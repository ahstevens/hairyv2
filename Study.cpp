#include "Study.h"
#include <glm/gtc/type_ptr.hpp>

// Initialize class variables
Study* Study::instance = NULL;

// Returns singleton Study instance
Study* Study::getInstance()
{
    if ( !instance )
        instance = new Study();
    return instance;
}

Study::Study()
{
	firstMouse = true;
	lastX  =  WIDTH  / 2.0f;
    lastY  =  HEIGHT / 2.0f;
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;  	// Time of last frame

	draw_normals = 0;

	camera = Camera(glm::vec3(0,0,565));
	light = Light(glm::vec3(1.0, 1.0, 1.0));

	// initialize key array
	for (int i = 0; i < 1024; ++i)
		keys[i] = 0;
}

Study::~Study()
{
}

void Study::init()
{
	// Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Choose the iPad retina display for full screen, if present
	int count, widthMM, heightMM;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	GLboolean retinaDisplayPresent;
	for(int i = 0; i < count; ++i)
	{
		glfwGetMonitorPhysicalSize(monitors[i], &widthMM, &heightMM);
		std::cout << "Monitor " << i << ": " << widthMM << "mm x " << heightMM << "mm" << std::endl;
		if(widthMM == 722 && heightMM == 542)
		{
			retinaDisplayPresent = true;
			monitor = monitors[i];
			break;
		}
	}

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow((GLint) WIDTH, (GLint) HEIGHT, "Hairy Slices", monitor, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

	 // Define the viewport dimensions
    glViewport(0, 0, (GLsizei) WIDTH, (GLsizei) HEIGHT);

    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


    // Build and compile our shader program
    Shader lightingShader("materials.vs", "materials.frag");
    Shader normalShader("normals.vs", "normals.frag", "normals.gs");

	std::cout << "Generating trial..." << std::endl;
	Trial trial(67, 50, 1.0f, 0.25f);
	std::cout << "Trial generated" << std::endl;

	objs = trial.getObjects();

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

		std::vector<SweepSurface>::iterator it;
		for(it = objs.begin(); it != objs.end(); it++)
		{
			(*it).redraw(lightingShader);

			if(draw_normals)
			{
				normalShader.Use();
				glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				// And draw model again, this time only drawing normal vectors using the geometry shaders (on top of previous model)
				(*it).redraw(normalShader);
			}
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
	getInstance()->key_process(window, key, scancode, action, mode);
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
			if (keys[GLFW_KEY_R])
				objs = Trial(67, 50, 1.0f, 0.25f).getObjects();
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
	getInstance()->mouse_process(window, xpos, ypos);
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
	getInstance()->scroll_process(window, xoffset, yoffset);
}

void Study::scroll_process(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll((GLfloat) yoffset);
}