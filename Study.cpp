#include "Study.h"
#include <glm/gtc/type_ptr.hpp>
#include "Slice.h"

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
	int width_px, height_px;
	glfwGetWindowSize(window, &width_px, &height_px);
	this->window = window;
	firstMouse = true;
	lastX  =  width_px  / 2.0f;
    lastY  =  height_px / 2.0f;
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;  	// Time of last frame

	draw_halos = cycle_light = draw_probe = 0;

	lengthMultiplier = thicknessMultiplier = directionalGeomScale = 1.f;
	haloSize = 0.5f;
	hedgehogOffset = 0.f;

	polhemus = Polhemus::getInstance();

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
	windowWidth = width_mm;
	windowHeight = height_mm;
	eyeDistance = dist_mm;

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


    // Build and compile our shader programs
	//Shader lightingShader("materials.vert", "materials.frag");
	Shader lightingShader("materials_new.vert", "materials_new.frag");
	Shader haloShader("halo.vert", "halo.frag");
	Shader hogShader("hedgehogs.vert", "hedgehogs.frag");
    Shader normalShader("normals.vert", "normals.frag", "normals.geom");

	Slice::Seed seed;
	seed.x = seed.y = 0.f;
	seed.dx = -1.f;
	seed.dy = 1.f;
	seed.dz = -1.f;

	Slice probe;
	probe.setShader(&lightingShader);
	probe.addSeed(seed);
	probe.renderPT(16);

	// set camera at eye position; far clipping plane is 1 meter behind screen
	glm::vec3 eyePos( 0.f, 0.f, eyeDistance );
	camera = Camera( eyePos, windowWidth, windowHeight, eyeDistance, eyeDistance + 2000.0f );

	generateTrial(Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_BLINN);

	// Build transformation matrix to position slices in middle of clipping volume and scale to fill screen
	float temp = (dist_mm + 1000.f) / dist_mm;
	glm::mat4 translate_mat = glm::translate( glm::mat4( 1.f ), glm::vec3( 0.0f, 0.0f, -1000.0f ) ); // Identity matrix
	glm::mat4 scale_mat     = glm::scale( glm::mat4( 1.f ), glm::vec3( temp, temp, temp ) ); // Identity matrix
	glm::mat4 xform_mat     = translate_mat * scale_mat;


    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = (GLfloat) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		polhemus->update();

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();

		// Create camera transformations
		glm::mat4 view = camera.getViewMatrix();
		view = view * xform_mat;
		glm::mat4 projection = camera.getProjectionMatrix();
		// Get the uniform locations
						
		if(cycle_light) light.setPosition(cos(glfwGetTime()), sin(glfwGetTime()), 1.0f);
		else light.setPosition( 1.f, 1.f, 1.f );

		if (trial.getRenderMode() == Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_BLINN ||
			trial.getRenderMode() == Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_PHONG ||
			trial.getRenderMode() == Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_MAXIMUM_PHONG)
		{
			lightingShader.Off();
			trial.passThroughPVMatrix((float*)glm::value_ptr(projection), 
									  (float*)glm::value_ptr(view));
			trial.display();
		}
		else if (trial.getRenderMode() == Trial::RenderMode::TRIAL_RENDER_SHADOWED_HEDGEHOGS)
		{
			// Clear the colorbuffer
			glClearColor(0.325f, 0.486f, 0.812f, 1.0f);
			//glClearColor(0.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			// Use cooresponding shader when setting uniforms/drawing objects
			hogShader.Use();
			trial.setShader(&hogShader);

			// Get uniform matrix locations in shader
			GLint viewLoc = glGetUniformLocation(hogShader.Program, "view");
			GLint projLoc = glGetUniformLocation(hogShader.Program, "projection");
			// Pass the matrices to the shader
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

			// Pass light and camera positions to shader
			glm::vec4 lightPos = light.getPosition();
			glUniform4f(glGetUniformLocation(hogShader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z, lightPos.w);
			glm::vec3 cameraPos = camera.getPosition();
			glUniform3f(glGetUniformLocation(hogShader.Program, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

			glm::vec3 ambientColor = light.getAmbientColor();
			glm::vec3 diffuseColor = light.getDiffuseColor();
			glm::vec3 specularColor = light.getSpecularColor();
			glUniform3f(glGetUniformLocation(hogShader.Program, "light.ambient"), ambientColor.r, ambientColor.g, ambientColor.b);
			glUniform3f(glGetUniformLocation(hogShader.Program, "light.diffuse"), diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(glGetUniformLocation(hogShader.Program, "light.specular"), specularColor.r, specularColor.g, specularColor.b);



			glUniform1f(glGetUniformLocation(hogShader.Program, "lengthMult"), lengthMultiplier);
			glUniform1f(glGetUniformLocation(hogShader.Program, "thicknessMult"), thicknessMultiplier);
			glUniform1f(glGetUniformLocation(hogShader.Program, "directionalGeomScale"), directionalGeomScale);
			
			glUniform1f(glGetUniformLocation(hogShader.Program, "offset"), (-trial.getShadowOffset())*lengthMultiplier + hedgehogOffset);

			glUniform1i(glGetUniformLocation(hogShader.Program, "doShadows"), true);

			trial.display();

			glUniform1i(glGetUniformLocation(hogShader.Program, "doShadows"), false);

			trial.display();
		}
		else
		{
			// Clear the colorbuffer
			glClearColor(0.325f, 0.486f, 0.812f, 1.0f);
			//glClearColor(0.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			// Use cooresponding shader when setting uniforms/drawing objects
			lightingShader.Use();

			
			// Get uniform matrix locations in shader
			GLint viewLoc  = glGetUniformLocation(lightingShader.Program,  "view");
			GLint projLoc  = glGetUniformLocation(lightingShader.Program,  "projection");
			// Pass the matrices to the shader
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

			// Pass light and camera positions to shader
			glm::vec4 lightPos = light.getPosition();
			glUniform4f(glGetUniformLocation(lightingShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z, lightPos.w);
			glm::vec3 cameraPos = camera.getPosition();
			glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);


			glm::vec3 ambientColor = light.getAmbientColor();
			glm::vec3 diffuseColor = light.getDiffuseColor();
			glm::vec3 specularColor = light.getSpecularColor(); 
			glUniform3f(glGetUniformLocation(lightingShader.Program, "light.ambient"),  ambientColor.r, ambientColor.g, ambientColor.b);
			glUniform3f(glGetUniformLocation(lightingShader.Program, "light.diffuse"),  diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(glGetUniformLocation(lightingShader.Program, "light.specular"), specularColor.r, specularColor.g, specularColor.b);


		
			glUniform1f(glGetUniformLocation(lightingShader.Program, "lengthMult"), lengthMultiplier);
			glUniform1f(glGetUniformLocation(lightingShader.Program, "thicknessMult"), thicknessMultiplier);
			glUniform1f(glGetUniformLocation(lightingShader.Program, "directionalGeomScale"), directionalGeomScale);

			if(draw_halos)
			{
				// reverse the vertex winding order
				glFrontFace( GL_CW );
				trial.setShader(&haloShader);
				haloShader.Use();
				glUniform1f(glGetUniformLocation(haloShader.Program, "lengthMult"), lengthMultiplier);
				glUniform1f(glGetUniformLocation(haloShader.Program, "thicknessMult"), thicknessMultiplier);
				glUniform1f(glGetUniformLocation(haloShader.Program, "directionalGeomScale"), directionalGeomScale);
				glUniform1f(glGetUniformLocation(haloShader.Program, "haloSize"), haloSize);
				glUniformMatrix4fv(glGetUniformLocation(haloShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(haloShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				// Draw model using the halo shader (regular model will be drawn on top)
				trial.display();
				// reset vertex winding order
				glFrontFace( GL_CCW );
			}
			
			if (draw_probe)
			{
				lightingShader.Use();
				probe.clearSeeds();
				glm::vec3 o = normalize(polhemus->getVector());
				seed.dx = o.x;
				seed.dy = o.y;
				seed.dz = o.z;
				probe.addSeed(seed);
				probe.renderPT(16);
				//probe.updateOrientation(normalize(polhemus->getVector()));
				//probe.updateOrientation(glm::vec3(cos(glfwGetTime()), sin(glfwGetTime()), 1.f));
				glUniform1f(glGetUniformLocation(lightingShader.Program, "lengthMult"), 50.f);
				glUniform1f(glGetUniformLocation(lightingShader.Program, "thicknessMult"), 10.f);
				glUniform1f(glGetUniformLocation(lightingShader.Program, "directionalGeomScale"), 7.5f);
				probe.redraw();
			}
			else
			{
				lightingShader.Use();
				trial.setShader(&lightingShader);
				trial.display();
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
			if (keys[GLFW_KEY_F])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_TUBES_PLAIN);
			if (keys[GLFW_KEY_G])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_TUBES_RINGED);
			if (keys[GLFW_KEY_H])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_SHADOWED_HEDGEHOGS);
			if (keys[GLFW_KEY_I])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_LINES_PLAIN);
			if (keys[GLFW_KEY_L])
				cycle_light = abs(cycle_light - 1);
			if (keys[GLFW_KEY_M])
				draw_halos = abs(draw_halos - 1);
			if (keys[GLFW_KEY_P])
				polhemus->printInfo();
			if (keys[GLFW_KEY_R])
				generateTrial(trial.getRenderMode());
			if (keys[GLFW_KEY_T])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_BLINN);
			if (keys[GLFW_KEY_U])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_MAXIMUM_PHONG);
			if (keys[GLFW_KEY_Y])
				trial.setRenderMode(Trial::RenderMode::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_PHONG);
			if (keys[GLFW_KEY_BACKSPACE])
			{
				thicknessMultiplier = lengthMultiplier = directionalGeomScale = 1.f;
				haloSize = 0.5f;
				hedgehogOffset = 5.f;
			}
			if (keys[GLFW_KEY_INSERT])
				draw_probe = abs(draw_probe - 1);
			if (keys[GLFW_KEY_HOME])
			{
				glm::vec3 eyePos(0.f, 0.f, eyeDistance);
				camera = Camera(eyePos, windowWidth, windowHeight, eyeDistance, eyeDistance + 2000.0f);
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

	// Study parameter modifiers
	if (keys[GLFW_KEY_MINUS])
		lengthMultiplier -= (lengthMultiplier > 0.1f) ? 0.1f : 0.f;
	if (keys[GLFW_KEY_EQUAL])
		lengthMultiplier += 0.1f;
	if (keys[GLFW_KEY_LEFT_BRACKET])
		thicknessMultiplier -= (thicknessMultiplier > 0.01f) ? 0.01f : 0.f;
	if (keys[GLFW_KEY_RIGHT_BRACKET])
		thicknessMultiplier += 0.01f;
	if (keys[GLFW_KEY_SEMICOLON])
		directionalGeomScale -= (directionalGeomScale > 0.01f) ? 0.01f : 0.f;
	if (keys[GLFW_KEY_APOSTROPHE])
		directionalGeomScale += 0.01f;
	if (keys[GLFW_KEY_COMMA])
		haloSize -= (haloSize > 0.01f) ? 0.01f : 0.f;
	if (keys[GLFW_KEY_PERIOD])
		haloSize += 0.01f;
	if (keys[GLFW_KEY_PAGE_DOWN])
		hedgehogOffset -= (hedgehogOffset > 0.1f) ? 0.1f : 0.f;
	if (keys[GLFW_KEY_PAGE_UP])
		hedgehogOffset += 0.1f;
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

void Study::generateTrial(Trial::RenderMode renderMode)
{
	std::cout << "Generating trial for " << windowWidth << " x " << windowHeight << "mm screen..." << std::endl;
	trial = Trial(windowWidth, windowHeight, 0.1f, 0.25f);
	trial.init();
	trial.setRenderMode(renderMode);
	std::cout << "Trial generated" << std::endl;
}