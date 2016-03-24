#include "Study.h"
#include <glm/gtc/type_ptr.hpp>
#include <time.h> // time() for srand()
#include <random>
#include <sys/stat.h> // stat()

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

#include <ctime> // for tm struct

#define POLHEMUS_CALIBRATION glm::quat( 0.999959, -0.0267551, -0.00644445, 0.0103233 )

#define NBLOCKS 2
#define NREPLICATESPERBLOCK 5
#define NDENSITYCONDITIONS 3
#define NTHICKNESSCONDITIONS 3
#define NRENDERINGCONDITIONS 5
#define NCONDITIONS NDENSITYCONDITIONS * ( ( NRENDERINGCONDITIONS - 2 ) + ( NTHICKNESSCONDITIONS * ( NRENDERINGCONDITIONS - 3 ) ) )
#define NTRIALSPERBLOCK NREPLICATESPERBLOCK * NCONDITIONS

#define BGCOLOR glm::vec3(0.325f, 0.486f, 0.812f)
//#define BGCOLOR glm::vec3(0.f, 0.f, 0.f)

// Initialize class variables
Study* Study::instance = NULL;

bool showTargetCursor = true;

// Returns singleton Study instance
Study* Study::getInstance( GLFWwindow* window )
{
    if ( !instance )
        instance = new Study( window );
    return instance;
}

Study::Study( GLFWwindow* window ) : probe(Probe(80.f, 20.f)), trainingTarget(Probe(80.f, 20.f)), targetCursor(Target( glm::vec3(1.f, 0.f, 0.f) ) )
{
	int width_px, height_px;
	glfwGetWindowSize(window, &width_px, &height_px);
	this->window = window;
	snapshotRequested = false;
	firstMouse = true;
	lastX  =  width_px  / 2.0f;
    lastY  =  height_px / 2.0f;
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;  	// Time of last frame

	draw_halos = cycle_light = probe_training = draw_probe = show_probe_hints = old_hog = 0;
	training_target_random = target_on_top = 1;

	density = 0.1f;
	jitter = 0.25f;

	lengthMultiplier = thicknessMultiplier = directionalGeomScale = 1.f;
	haloSize = 0.5f;
	hedgehogOffset = 0.f;

	polhemus = Polhemus::getInstance();

	camera = Camera();
	light = Light(glm::vec3(1.0, 1.0, 1.0));

	// initialize key array
	for (int i = 0; i < 1024; ++i)
		keys[i] = 0;
	
	bgColor = BGCOLOR;

    // Build and compile our shader programs
	lightingShader = new Shader("materials.vert", "materials.frag");
	lineShader = new Shader("lines.vert", "lines.frag");
	haloShader = new Shader("halo.vert", "halo.frag");
	hogShader = new Shader("hedgehogs.vert", "hedgehogs.frag");
	hogShaderRev = new Shader("hedgehogs_rev.vert", "hedgehogs_rev.frag");
	targetShader = new Shader("target.vert", "target.frag");
}

Study::~Study()
{
	delete lightingShader;
	delete lineShader;
	delete haloShader;
	delete hogShader;
	delete hogShaderRev;
	delete targetShader;
}

void Study::init(std::string name, GLfloat width_mm, GLfloat height_mm, GLfloat dist_mm)
{
	prepareOutput( name );

	participant = name;
	windowWidth = width_mm;
	windowHeight = height_mm;
	eyeDistance = dist_mm;

	polhemus->setCalibration( POLHEMUS_CALIBRATION );

	if( name == std::string("demo") )
	{
		this->mode = Mode::DEMO;
		generateTrial(Trial::RenderMode::LINES_PLAIN);
	}
	else
	{
		this->mode = Mode::NONE;
		std::cout << "Press 'T' to enter training mode, or <Enter> to begin the study" << std::endl;
	}
		
	probe.setShader(lightingShader);
	probe.init();

	trainingTarget.setShader(lightingShader);
	trainingTarget.init();

	targetCursor.setSize(20.f, 20.f, 0.f);
	targetCursor.setShader(targetShader);

	// set target color and opacity. Shader must be active to set the uniform
	targetShader->Use();	
	glUniform3f(glGetUniformLocation(targetShader->Program, "col"), 1.f, 0.f, 0.f); // red
	Shader::Off(); // turn shader back off

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(3.f);

	// set camera at eye position; far clipping plane is 1 meter behind screen
	glm::vec3 eyePos( 0.f, 0.f, eyeDistance );
	camera = Camera( eyePos, windowWidth, windowHeight, eyeDistance, eyeDistance + 200000.0f );

	this->mainLoop();
}

void Study::mainLoop()
{
	// main loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		polhemus->update();

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		if (mode == Mode::DEMO) do_movement();

		if (mode != Mode::NONE && mode != Mode::PAUSED)
		{
			render();

			// Swap the screen buffers
			glfwSwapBuffers(window);

			if ( snapshotRequested )
			{
				snapshotTGA( "snapshot" );
				snapshotRequested = false;
			}
		}
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
}

void Study::render()
{	
	// Change background for Shadowed Hedgehogs
	if( trial.getRenderMode() == Trial::RenderMode::SHADOWED_HEDGEHOGS && !probe_training )
		bgColor = glm::vec3(1.f, 1.f, 1.f);
	else
		bgColor = BGCOLOR;

	// Clear the colorbuffer
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (probe_training) // probe orientation training
	{	
		// define probe colors
		glm::vec3 white = glm::vec3(1.f, 1.f, 1.f);
		glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
		glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);

		if (show_probe_hints)
		{
			glm::quat p = glm::normalize(probe.getOrientation());
			glm::quat t = glm::normalize(trainingTarget.getOrientation());
			glm::vec3 vecXp = glm::rotate(p, glm::vec3(1.f, 0.f, 0.f));
			glm::vec3 vecXt = glm::rotate(t, glm::vec3(1.f, 0.f, 0.f));
			float cosTheta = dot(vecXp, vecXt);
			float theta = glm::degrees(acos(cosTheta));

			if (theta <= 1.f) // white halo if within 1 degree
			{
				initGL(haloShader);

				glUniform1f(glGetUniformLocation(haloShader->Program, "haloSize"), 0.1f);
				glUniform3f(glGetUniformLocation(haloShader->Program, "haloColor"), 1.f, 1.f, 1.f );
				glUniform1f(glGetUniformLocation(haloShader->Program, "lengthMult"), 1.f);
				glUniform1f(glGetUniformLocation(haloShader->Program, "thicknessMult"), 1.f);
				glUniform1f(glGetUniformLocation(haloShader->Program, "directionalGeomScale"), 10.f);
				glUniform1i(glGetUniformLocation(haloShader->Program, "glyphHead"), false);

				trainingTarget.setShader(haloShader);
				
				// reverse the vertex winding order
				glFrontFace(GL_CW);
				// Draw model using the halo shader (regular model will be drawn on top)
				trainingTarget.redraw();
				// reset vertex winding order
				glFrontFace(GL_CCW);
			}
			else if (theta <= 5.f) // green probe if within 5 degrees
				trainingTarget.setColor(green);			
			else if (theta <= 15.f) // from 15 degrees to 5 degrees, gradient from yellow to green
			{
				float a = (theta - 15.f) / (5.f - 15.f);
				trainingTarget.setColor(mix(yellow, green, a));
			}
			else  if (theta <= 90.f) // from 90 degrees to 15 degrees, gradient from white to yellow
			{
				float a = (theta - 90.f) / (15.f - 90.f);
				trainingTarget.setColor(mix(white, yellow, a));
			}
			else // if greater than 90 degrees, white
				trainingTarget.setColor(white);
		}
		else // default probe color is white
			trainingTarget.setColor(white);

		initGL(lightingShader);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "lengthMult"), 1.f);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "thicknessMult"), 1.f);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "directionalGeomScale"), 10.f);

		probe.setOrientation(normalize(polhemus->getQuaternion()));

		if (draw_probe) probe.redraw();
		trainingTarget.setShader(lightingShader);
		trainingTarget.redraw();
	}
	else
	{
		switch (trial.getRenderMode())
		{
			case Trial::RenderMode::LINES_PLAIN:
				setupPL();
				break;
			case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN:
			case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG:
			case Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG:
				setupIL();

				trial.passThroughPVMatrix((float*)glm::value_ptr(camera.getProjectionMatrix()),
					(float*)glm::value_ptr(camera.getViewMatrix()));

				break;
			case Trial::RenderMode::SHADOWED_HEDGEHOGS:
				setupSH();
				break;
			case Trial::RenderMode::TUBES_PLAIN:
			case Trial::RenderMode::TUBES_RINGED:
				setupTubes();
				break;
		}

		// Display the trial slice
		trial.redraw();
			
		if (showTargetCursor)
		{
			// Now display target cursor
			initGL(targetShader);

			// Should we render the target atop everything else, or render it on the slice?
			if (target_on_top) glDisable(GL_DEPTH_TEST);

			// Turn off writing to the depth mask for transparency effects
			glDepthMask(GL_FALSE);

			// First draw target cursor as a filled object
			glUniform1f(glGetUniformLocation(targetShader->Program, "opacity"), 0.1f);
			targetCursor.redraw(); // draw target cursor		

			// Now draw target cursor outline slightly darker
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform1f(glGetUniformLocation(targetShader->Program, "opacity"), 0.25f);
			targetCursor.redraw();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// Reenable writing to the depth mask
			glDepthMask(GL_TRUE);

			// Turn the depth test back on if it was turned off previously
			if (target_on_top) glEnable(GL_DEPTH_TEST);
		}
	}
}

void Study::setupPL()
{
	this->initGL(lineShader);
	trial.setShader(lineShader);
}

void Study::setupIL()
{
	this->initGL(lightingShader);
	trial.setShader(lightingShader);	
}

void Study::setupSH()
{
	Shader *s = old_hog ? hogShader : hogShaderRev;

	initGL(s);

	glUniform1f(glGetUniformLocation(s->Program, "lengthMult"), (1 / density) / 2 / trial.getMaxLength());

	if(old_hog) glUniform1f(glGetUniformLocation(s->Program, "offset"), (-trial.getShadowOffset())*(1 / density) / 2 / trial.getMaxLength() + hedgehogOffset);
	else glUniform1f(glGetUniformLocation(s->Program, "offset"), hedgehogOffset);

	// first pass to render shadow plane
	glUniform1i(glGetUniformLocation(s->Program, "doShadows"), true);

	trial.setShader(s);

	trial.redraw();

	// second pass to render the glyph plane
	glUniform1i(glGetUniformLocation(s->Program, "doShadows"), false);
}

void Study::setupTubes()
{
	if (draw_halos)
	{
		initGL(haloShader);

		glUniform1f(glGetUniformLocation(haloShader->Program, "haloSize"), haloSize);
		glUniform3f(glGetUniformLocation(haloShader->Program, "haloColor"), 0.f, 0.f, 0.f);

		trial.setShader(haloShader);

		// reverse the vertex winding order
		glFrontFace(GL_CW);
		// Draw model using the halo shader (regular model will be drawn on top)
		trial.redraw();
		// reset vertex winding order
		glFrontFace(GL_CCW);
	}

	this->initGL(lightingShader);
	trial.setShader(lightingShader);
}

void Study::initGL(Shader *s)
{
	// Use cooresponding shader when setting uniforms/drawing objects
	s->Use();

	// Pass the matrices to the shader
	glm::mat4 view = camera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(s->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glm::mat4 projection = camera.getProjectionMatrix();
	glUniformMatrix4fv(glGetUniformLocation(s->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Pass light and camera positions to shader
	glm::vec4 lightPos = light.getPosition();
	glUniform4f(glGetUniformLocation(s->Program, "light.position"), lightPos.x, lightPos.y, lightPos.z, lightPos.w);
	glm::vec3 cameraPos = camera.getPosition();
	glUniform3f(glGetUniformLocation(s->Program, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glm::vec3 ambientColor = light.getAmbientColor();
	glm::vec3 diffuseColor = light.getDiffuseColor();
	glm::vec3 specularColor = light.getSpecularColor();
	glUniform3f(glGetUniformLocation(s->Program, "light.ambient"), ambientColor.r, ambientColor.g, ambientColor.b);
	glUniform3f(glGetUniformLocation(s->Program, "light.diffuse"), diffuseColor.r, diffuseColor.g, diffuseColor.b);
	glUniform3f(glGetUniformLocation(s->Program, "light.specular"), specularColor.r, specularColor.g, specularColor.b);

	glUniform1f(glGetUniformLocation(s->Program, "lengthMult"), lengthMultiplier);
	glUniform1f(glGetUniformLocation(s->Program, "thicknessMult"), thicknessMultiplier);
	glUniform1f(glGetUniformLocation(s->Program, "directionalGeomScale"), directionalGeomScale);
}

void Study::begin()
{
	mode = Mode::STUDY;
	probe_training = draw_probe = show_probe_hints = 0;


	srand((unsigned int)time(NULL));

	std::vector< Trial::RenderMode > renders;
	renders.push_back( Trial::RenderMode::LINES_PLAIN ); 
	renders.push_back( Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN );
	renders.push_back( Trial::RenderMode::SHADOWED_HEDGEHOGS ); 
	renders.push_back( Trial::RenderMode::TUBES_PLAIN );
	renders.push_back( Trial::RenderMode::TUBES_RINGED );

	std::vector< float > densities;
	densities.push_back( 0.4f ); 
	densities.push_back( 0.3f );
	densities.push_back( 0.2f );

	std::vector< float > thicknesses;
	thicknesses.push_back( 2.f );
	thicknesses.push_back( 1.f );
	thicknesses.push_back( 0.5f );

	std::vector< float > glyphHeads;
	glyphHeads.push_back( 1.15f );
	glyphHeads.push_back( 0.75f );
	glyphHeads.push_back( 0.5f );
	std::vector< std::vector<Condition> > block;

	for (int i = 0; i < densities.size(); ++i)
		for (int j = 0; j < renders.size(); ++j)
		{	
			std::vector<Condition> replicates;

			Trial::RenderMode rm = renders[j];
			float d = densities[i];
			float l = 1.f;
			float t = 1.f;
			float gh = 1.f;
				
			if( rm == Trial::RenderMode::LINES_PLAIN ||
				rm == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
				rm == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
				rm == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG )
				gh = 0.75;

			if( rm == Trial::RenderMode::SHADOWED_HEDGEHOGS )
			{
				t = 0.57;
				gh = 0.47;
			}

			if( rm == Trial::RenderMode::TUBES_PLAIN || rm == Trial::RenderMode::TUBES_RINGED )
			{
				for( int m = 0; m < NTHICKNESSCONDITIONS; ++m)
				{
					t = thicknesses[m];
					gh = glyphHeads[m];

					for (int k = 0; k < NREPLICATESPERBLOCK; ++k)
						replicates.push_back(Condition(rm, d, l, t, gh));					

					block.push_back( replicates );
					replicates.clear();
				}
			}
			else
			{
				for (int k = 0; k < NREPLICATESPERBLOCK; ++k)
					replicates.push_back(Condition(rm, d, l, t, gh));

				block.push_back( replicates );
			}
		}


	for (int i = 0; i < NBLOCKS; ++i)
	{
		std::random_shuffle(block.begin(), block.end());

		blocks.push_back(block);
	}

	std::cout << "Commencing study..." << std::endl;
	std::cout << std::endl;

	next();
}

void Study::next()
{	
	// get current trial block from queue
	std::vector< std::vector< Condition > > *block = &blocks.back();
	std::vector< Condition > *cond = &block->back();

	if( cond->size() == 0 )
	{
		std::cout << "Block " << NBLOCKS - blocks.size() + 1 << ": Condition " << NCONDITIONS - block->size() + 1 << " of " << NCONDITIONS << " completed!" << std::endl;
		mode = PAUSED;
		block->pop_back();

		if( block->size() == 0 )
		{
			std::cout << "Block " << NBLOCKS - blocks.size() + 1 << " of " << NBLOCKS << " completed!" << std::endl;
			bgColor = glm::vec3(0.f, 0.5f, 0.f);
			mode = PAUSED;
			blocks.pop_back();

			if( blocks.size() == 0 )
				end();
			else
				block = &blocks.back();
						
			std::cout << "Please take a short break, then press the ENTER key when ready to begin the next block." << std::endl << std::endl;
		}
		else
		{
			std::cout << "Please take a short break, then press the ENTER key when ready to begin the next condition." << std::endl << std::endl;
			bgColor = glm::vec3(0.8f, 0.596f, 0.f);
		}

		cond = &block->back();
	}
	else	
	{
		bgColor = glm::vec3(0.f, 0.f, 0.f);
	}


	Condition *repl = &cond->back();
	cond->pop_back();

	// Give blank screen immediately while processing new trial
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);


	density = repl->density;
	lengthMultiplier = repl->lengthMultiplier;
	thicknessMultiplier = repl->thicknessMultiplier;
	directionalGeomScale = repl->glyphHeadMultiplier;

	generateTrial(repl->renderMode);

	stopwatch.start();
}

void Study::end()
{
	std::cout << "Study Complete!" << std::endl;
	mode = Mode::NONE;
	
	// Tell GLFW to kill the OpenGL window and exit
	glfwTerminate();
	exit( 0 );
}


void Study::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	getInstance(window)->key_process(window, key, scancode, action, mode);
}

// Is called whenever a key is pressed/released via GLFW
void Study::key_process(GLFWwindow* window, int key, int scancode, int action, int keymode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && mode != STUDY)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
			switch (mode)
			{
			case DEMO:
				if (keys[GLFW_KEY_1])
					trial.setRenderMode(Trial::RenderMode::LINES_PLAIN);
				if (keys[GLFW_KEY_2])
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN, lengthMultiplier);
				if (keys[GLFW_KEY_3])
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG, lengthMultiplier);
				if (keys[GLFW_KEY_4])
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG, lengthMultiplier);
				if (keys[GLFW_KEY_5])
					trial.setRenderMode(Trial::RenderMode::TUBES_PLAIN);
				if (keys[GLFW_KEY_6])
					trial.setRenderMode(Trial::RenderMode::TUBES_RINGED);
				if (keys[GLFW_KEY_7])
					trial.setRenderMode(Trial::RenderMode::SHADOWED_HEDGEHOGS);
				if (keys[GLFW_KEY_L])
					cycle_light = abs(cycle_light - 1);
				if (keys[GLFW_KEY_M])
					draw_halos = abs(draw_halos - 1);
				if (keys[GLFW_KEY_T])
					showTargetCursor = !showTargetCursor;
				if (keys[GLFW_KEY_P])
				{					
					std::cout << std::endl;
					std::cout << "########################################################" << std::endl;
					std::cout << "####################### Accuracy #######################" << std::endl;
					std::cout << "Angular Error to Target: " << glm::degrees( getAngleError( getAdjustedTargetCursorOrientation(), polhemus->getQuaternion() ) ) << std::endl;					
					std::cout << "############### Calibration Axis Offsets ###############" << std::endl;
					std::cout << "                     +x: " << glm::degrees( getAngleError( vecsToQuat( glm::vec3( 1.f, 0.f, 0.f ), glm::vec3( +1.f, 0.f, 0.f ) ), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << "                     -x: " << glm::degrees( getAngleError( glm::quat( 0.f, 0.f, 1.f, 0.f ), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << "                     +y: " << glm::degrees( getAngleError( vecsToQuat( glm::vec3( 1.f, 0.f, 0.f ), glm::vec3( 0.f, +1.f, 0.f ) ), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << "                     -y: " << glm::degrees( getAngleError( vecsToQuat( glm::vec3( 1.f, 0.f, 0.f ), glm::vec3( 0.f, -1.f, 0.f ) ), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << "                     +z: " << glm::degrees( getAngleError( vecsToQuat( glm::vec3( 1.f, 0.f, 0.f ), glm::vec3( 0.f, 0.f, +1.f ) ), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << "                     -z: " << glm::degrees( getAngleError( vecsToQuat( glm::vec3( 1.f, 0.f, 0.f ), glm::vec3( 0.f, 0.f, -1.f ) ), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << "#################### Trial Settings ####################" << std::endl;
					std::cout << "                density: " << density << std::endl;
					std::cout << "                 jitter: " << jitter << std::endl;
					std::cout << "       lengthMultiplier: " << lengthMultiplier << std::endl;
					std::cout << "    thicknessMultiplier: " << thicknessMultiplier << std::endl;
					std::cout << "   directionalGeomScale: " << directionalGeomScale << std::endl;
					std::cout << "               haloSize: " << haloSize << std::endl;
					std::cout << "         hedgehogOffset: " << hedgehogOffset << std::endl;
					std::cout << "########################################################" << std::endl;
					std::cout << std::endl;
					std::cout << std::endl;
				}
				if (keys[GLFW_KEY_R])
					generateTrial(trial.getRenderMode());
				if (keys[GLFW_KEY_BACKSPACE])
				{
					thicknessMultiplier = lengthMultiplier = directionalGeomScale = 1.f;
					haloSize = 0.5f;
					hedgehogOffset = 0.f;
					if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
						trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
				}

				if (keys[GLFW_KEY_MINUS])
				{
					lengthMultiplier -= (lengthMultiplier > 0.1f) ? 0.1f : 0.f;
					if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
						trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
				}
				if (keys[GLFW_KEY_EQUAL])
				{
					lengthMultiplier += 0.1f;
					if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
						trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
				}
				if (keys[GLFW_KEY_LEFT_BRACKET])
					thicknessMultiplier -= (thicknessMultiplier > 0.01f) ? 0.01f : 0.f;
				if (keys[GLFW_KEY_RIGHT_BRACKET])
				{
					thicknessMultiplier += 0.01f;
					if(directionalGeomScale < thicknessMultiplier / 2.f)
						directionalGeomScale = thicknessMultiplier / 2.f;
				}
				if (keys[GLFW_KEY_SEMICOLON])
					directionalGeomScale -= (directionalGeomScale > 0.01f && directionalGeomScale > thicknessMultiplier / 2) ? 0.01f : 0.f;
				if (keys[GLFW_KEY_APOSTROPHE])
					directionalGeomScale += 0.01f;
				if (keys[GLFW_KEY_COMMA])
					haloSize -= (haloSize > 0.01f) ? 0.01f : 0.f;
				if (keys[GLFW_KEY_PERIOD])
					haloSize += 0.01f;
				if (keys[GLFW_KEY_SLASH])
					old_hog = abs(old_hog - 1);
				if (keys[GLFW_KEY_NUM_LOCK])
					target_on_top = abs(target_on_top - 1);
				if (keys[GLFW_KEY_KP_SUBTRACT])
				{
					if (density > 0.1f + 0.0001f)
					{
						density -= 0.1f;
						trial.setDensity(density);
						trial.sampleBiMap();
						if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
							trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
							trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
							trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
					}
				}
				if (keys[GLFW_KEY_KP_ADD])
				{
					density += 0.1f;
					trial.setDensity(density);
					trial.sampleBiMap();
					if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
						trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
				}

				if (keys[GLFW_KEY_INSERT])
					probe_training = abs(probe_training - 1);
				if (keys[GLFW_KEY_DELETE])
					draw_probe = abs(draw_probe - 1);
				if (keys[GLFW_KEY_HOME])
				{
					glm::vec3 eyePos(0.f, 0.f, eyeDistance);
					camera = Camera(eyePos, windowWidth, windowHeight, eyeDistance, eyeDistance + 200000.0f);
				}
				if (keys[GLFW_KEY_END])
					show_probe_hints = abs(show_probe_hints - 1);
				if (keys[GLFW_KEY_PAGE_DOWN])
					hedgehogOffset -= (hedgehogOffset > 0.1f) ? 0.1f : 0.f;
				if (keys[GLFW_KEY_PAGE_UP])
					hedgehogOffset += 0.1f;

				if (keys[GLFW_KEY_KP_1])
					glLineWidth(1.f);
				if (keys[GLFW_KEY_KP_2])
					glLineWidth(2.f);
				if (keys[GLFW_KEY_KP_3])
					glLineWidth(3.f);
				if (keys[GLFW_KEY_KP_4])
					polhemus->calibratePitchUp();
				if (keys[GLFW_KEY_KP_5])
					polhemus->calibrateRollUp();
				if (keys[GLFW_KEY_KP_6])
					polhemus->calibratePitchDown();
				if (keys[GLFW_KEY_KP_7])
					polhemus->calibrateYawUp();
				if (keys[GLFW_KEY_KP_8])
					polhemus->calibrateRollDown();
				if (keys[GLFW_KEY_KP_9])
					polhemus->calibrateYawDown();
				if (keys[GLFW_KEY_KP_DECIMAL])
					polhemus->calibrateReset();
				if (keys[GLFW_KEY_KP_ENTER])
				{
					glm::vec3 errorAxis = glm::rotate( polhemus->getQuaternion(), glm::vec3( 1.f, 0.f, 0.f ) );
					std::cout << "Uncalibrated x-axis: ( " << errorAxis.x << ", " << errorAxis.y << ", " << errorAxis.z << " )" << std::endl;
					std::cout << "Calibrating probe to correct " << glm::degrees( getAngleError( glm::quat(), polhemus->getQuaternion() ) ) << " degree error... ";
					polhemus->calibrate();
					polhemus->calibrateRollUp( 180.f );
					std::cout << "done" << std::endl;		

					glm::quat cal = polhemus->getCalibration();
					std::cout << "Calibration quaternion ( w, x, y, z ): ( " << cal.w << ", " << cal.x << ", " << cal.y << ", " << cal.z << " )" << std::endl;
				}
				
				if (keys[GLFW_KEY_SPACE] && probe_training)
				{
					recordTrial( true );
					updateTrainingTarget();
				}				
				if ( ( keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT] ) && probe_training )
				{					
					training_target_random = abs(training_target_random - 1);
					updateTrainingTarget();
				}

				if (keys[GLFW_KEY_PRINT_SCREEN])
					snapshotRequested = true;
				
				if (keys[GLFW_KEY_ENTER])
					begin();

				break;
			case PAUSED:
				if (keys[GLFW_KEY_ENTER])
					mode = STUDY;
				break;
			case NONE:
				if (keys[GLFW_KEY_ENTER])
					begin();
				//if (keys[GLFW_KEY_D])
				//{
				//	mode = Mode::DEMO;
				//	generateTrial(Trial::RenderMode::LINES_PLAIN);
				//}
				if (keys[GLFW_KEY_T])
				{
					mode = Mode::TRAINING;
					generateTrial(Trial::RenderMode::LINES_PLAIN);
				}
				break;
			case STUDY:
				if (keys[GLFW_KEY_SPACE] && stopwatch.read() > 2.0)
				{
					recordTrial();
					next();
				}
				break;
			case TRAINING:		
				if (keys[GLFW_KEY_1])
				{
					std::cout << "Rendering Mode: Plain Lines" << std::endl;
					trial.setRenderMode(Trial::RenderMode::LINES_PLAIN);
				}
				if (keys[GLFW_KEY_2])
				{
					std::cout << "Rendering Mode: Illuminated Lines" << std::endl;
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN, lengthMultiplier);		
				}
				if (keys[GLFW_KEY_3])
				{
					std::cout << "Rendering Mode: Plain Tubes" << std::endl;
					trial.setRenderMode(Trial::RenderMode::TUBES_PLAIN);
				}
				if (keys[GLFW_KEY_4])
				{
					std::cout << "Rendering Mode: Ringed Tubes" << std::endl;
					trial.setRenderMode(Trial::RenderMode::TUBES_RINGED);
				}
				if (keys[GLFW_KEY_5])
				{
					std::cout << "Rendering Mode: Shadowed Hedgehogs" << std::endl;
					trial.setRenderMode(Trial::RenderMode::SHADOWED_HEDGEHOGS);
				}
				if (keys[GLFW_KEY_R])
				{
					if( probe_training )	
					{
						std::cout << "Refreshing Training Probe... ";
						updateTrainingTarget();
						std::cout << "done" << std::endl;
					}
					else
					{
						std::cout << "Refreshing Flow Field... ";
						generateTrial(trial.getRenderMode());
						std::cout << "done" << std::endl;
					}
				}
				if (keys[GLFW_KEY_T])
				{
					probe_training = abs(probe_training - 1);					
					std::cout << "Probe Training Mode: " << probe_training << std::endl;
				}
				if (keys[GLFW_KEY_P])
				{
					draw_probe = abs(draw_probe - 1);
					std::cout << "Draw Polhemus Probe: " << draw_probe << std::endl;
				}
				if (keys[GLFW_KEY_H])
				{
					show_probe_hints = abs(show_probe_hints - 1);
					std::cout << "Show Accuracy Hints: " << show_probe_hints << std::endl;
				}
				if (keys[GLFW_KEY_A])
				{					
					training_target_random = abs(training_target_random - 1);
					updateTrainingTarget();
					std::cout << "Random Training Probe Orientation: " << training_target_random << std::endl;
				}
				if (keys[GLFW_KEY_DOWN])
				{
					if (density > 0.1f + 0.0001f)
					{
						density -= 0.1f;
						trial.setDensity(density);
						trial.sampleBiMap();
						if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
							trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
							trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
							trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
						std::cout << "Seeding Density: " << density << std::endl;
					}
					else						
						std::cout << "Minimum Seeding Density of " << density << " reached" << std::endl;
				}
				if (keys[GLFW_KEY_UP])
				{
					density += 0.1f;
					trial.setDensity(density);
					trial.sampleBiMap();
					if (trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG ||
						trial.getRenderMode() == Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG)
						trial.setRenderMode(trial.getRenderMode(), lengthMultiplier);
					std::cout << "Seeding Density: " << density << std::endl;
				}				
				if (keys[GLFW_KEY_LEFT])
				{
					switch( trial.getRenderMode() )
					{
					case Trial::RenderMode::LINES_PLAIN:
						trial.setRenderMode(Trial::RenderMode::SHADOWED_HEDGEHOGS);
						std::cout << "Rendering Mode: Shadowed Hedgehogs" << std::endl;
						break;
					case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN:
						trial.setRenderMode(Trial::RenderMode::LINES_PLAIN);
						std::cout << "Rendering Mode: Plain Lines" << std::endl;
						break;
					case Trial::RenderMode::TUBES_PLAIN:
						trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN);
						std::cout << "Rendering Mode: Illuminated Lines" << std::endl;
						break;
					case Trial::RenderMode::TUBES_RINGED:
						trial.setRenderMode(Trial::RenderMode::TUBES_PLAIN);
						std::cout << "Rendering Mode: Plain Tubes" << std::endl;
						break;
					case Trial::RenderMode::SHADOWED_HEDGEHOGS:
						trial.setRenderMode(Trial::RenderMode::TUBES_RINGED);
						std::cout << "Rendering Mode: Ringed Tubes" << std::endl;
						break;
					}
				}				
				if (keys[GLFW_KEY_RIGHT])
				{
					switch( trial.getRenderMode() )
					{
					case Trial::RenderMode::LINES_PLAIN:
						trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN);
						std::cout << "Rendering Mode: Illuminated Lines" << std::endl;
						break;
					case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN:
						trial.setRenderMode(Trial::RenderMode::TUBES_PLAIN);
						std::cout << "Rendering Mode: Plain Tubes" << std::endl;
						break;
					case Trial::RenderMode::TUBES_PLAIN:
						trial.setRenderMode(Trial::RenderMode::TUBES_RINGED);
						std::cout << "Rendering Mode: Ringed Tubes" << std::endl;
						break;
					case Trial::RenderMode::TUBES_RINGED:
						trial.setRenderMode(Trial::RenderMode::SHADOWED_HEDGEHOGS);
						std::cout << "Rendering Mode: Shadowed Hedgehogs" << std::endl;
						break;
					case Trial::RenderMode::SHADOWED_HEDGEHOGS:
						trial.setRenderMode(Trial::RenderMode::LINES_PLAIN);
						std::cout << "Rendering Mode: Plain Lines" << std::endl;
						break;
					}
				}
				if (keys[GLFW_KEY_SPACE])
				{
					if( probe_training && training_target_random )
					{
						std::cout << "Angular Error to Training Probe: " << glm::degrees( getAngleError( trainingTarget.getOrientation(), polhemus->getQuaternion() ) ) << std::endl;
						recordTrial( true );
						updateTrainingTarget();
					}
					else
						std::cout << "Angular Error to Flow at Target Cursor: " << glm::degrees( getAngleError( getAdjustedTargetCursorOrientation(), polhemus->getQuaternion() ) ) << std::endl;
				}
				if (keys[GLFW_KEY_ENTER])
					begin();
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

    if(mode == DEMO) camera.processMouseMovement(xoffset, yoffset);
}

void Study::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	getInstance(window)->scroll_process(window, xoffset, yoffset);
}

void Study::scroll_process(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll((GLfloat) yoffset);
}

void Study::generateTrial( Trial::RenderMode renderMode )
{
	//std::cout << "Generating trial for " << windowWidth << " x " << windowHeight << "mm screen..." << std::endl;
	trial = Trial(windowWidth, windowHeight, density, jitter);
	trial.init();
	trial.setRenderMode(renderMode, lengthMultiplier);

	Trial::Seed targSeed = trial.getRandomSeed( 0.5f );

	targetCursor.setPosition(targSeed.x - windowWidth / 2.f, targSeed.y - windowHeight / 2.f, 0.f);
	targetCursor.setSeed(targSeed);
	
	updateTrainingTarget();
}

glm::quat Study::getAdjustedTargetCursorOrientation()
{
	glm::vec3 flowVec = targetCursor.getFlowVector();

	glm::vec3 v1 = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 v2 = glm::normalize( -flowVec );

	return glm::normalize(vecsToQuat(v1, v2));
}

float Study::getAngleError(glm::quat p, glm::quat q)
{
	p = glm::normalize( p ); 
	q = glm::normalize( q );
	glm::vec3 vecXp = glm::rotate( p, glm::vec3( 1.f, 0.f, 0.f ) );
	glm::vec3 vecXq = glm::rotate( q, glm::vec3( 1.f, 0.f, 0.f ) );
	float cosTheta = dot( vecXp, vecXq );
	return ( cosTheta > 0.999999f ) ? 0.f : acos( cosTheta );
}

void Study::updateTrainingTarget()
{
	if( training_target_random )
		trainingTarget.setOrientation( getRandomOrientation() );
	else
		trainingTarget.setOrientation( this->getAdjustedTargetCursorOrientation() );
}
glm::quat Study::getRandomOrientation()
{
	std::random_device seed;  // random seed
	std::mt19937 gen(seed()); // Mersenne Twister RNG
	std::uniform_real_distribution<float> dist(-1, 1);

	float w, x, y, z;

	w = dist( gen );
	x = dist( gen );
	y = dist( gen );
	z = dist( gen );
	
	return glm::normalize( glm::quat( w, x, y, z ) );
}

glm::quat Study::vecsToQuat(glm::vec3 u, glm::vec3 v)
{
	//glm::vec3 w = glm::cross(u, v);
	//glm::quat q = glm::quat(1.f + glm::dot(u, v), w.x, w.y, w.z);

	glm::quat q;
	glm::vec3 a = glm::cross(u, v);
	q.x = a.x;
	q.y = a.y;
	q.z = a.z;
	q.w = sqrt((u.length() ^ 2) * (v.length() ^ 2)) + glm::dot(u, v);

	return glm::normalize(q);
}

bool Study::fileExists( const std::string &fname )
{
  struct stat buffer;   
  return (stat (fname.c_str(), &buffer) == 0);
}

void Study::prepareOutput( std::string name )
{
	// construct filename
	outFileName = std::string( name + "_data" + ".csv" );

	// if file exists, keep trying until we find a filename that doesn't already exist
	for (int i = 0; fileExists( outFileName ); ++i)
		outFileName = std::string(name + "_data_" + std::to_string(i) + ".csv");
	
	outFile.open( outFileName );

	if( outFile.is_open() )
	{
		std::cout << "Opened file " << outFileName << " for writing output" << std::endl;		
		outFile << "participant,block,trial,probe_training,draw_probe,show_probe_hints,render,density,lengthMulti,thicknessMulti,directGeomMulti,probe.x,probe.y,probe.z,target.x,target.y,target.z,target.length,error_degrees,time" << std::endl;
	}
	else
		std::cout << "Error opening file " << outFileName << " for writing output" << std::endl;
}

// takes in a target orientation quaternion referenced to the +x axis vector
// (1,0,0) and compares it to the Polhemus probe to get angular difference
// (error) between the two and records it to the output file
void Study::recordTrial( bool trainingTrial )
{
	// Construct string for rendering mode enum
	std::string rm;

	if( trainingTrial )
		rm = std::string( "training" );
	else
		switch( trial.getRenderMode() )
		{
			case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN:
				rm = std::string("il_cylblinn");
				break;		
			case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG:
				rm = std::string("il_cylphong");
				break;
			case Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG:
				rm = std::string("il_maxphong");
				break;
			case Trial::RenderMode::LINES_PLAIN:
				rm = std::string("plain_lines");
				break;
			case Trial::RenderMode::SHADOWED_HEDGEHOGS:
				rm = std::string("shadowed_hedgehogs");
				break;
			case Trial::RenderMode::TUBES_PLAIN:
				rm = std::string("tubes_plain");
				break;
			case Trial::RenderMode::TUBES_RINGED:
				rm = std::string("tubes_ringed");
				break;
		}

	// Get unit vectors of probe and target directions
	glm::quat probeQuat = polhemus->getQuaternion();
	glm::quat targetQuat = trainingTrial ? trainingTarget.getOrientation() : getAdjustedTargetCursorOrientation();
	glm::vec3 pVec = glm::normalize(glm::rotate(probeQuat, glm::vec3(1.f, 0.f, 0.f)));
	glm::vec3 tVec = glm::normalize(glm::rotate(targetQuat, glm::vec3(1.f, 0.f, 0.f)));
	
	// Begin outputting trial into file
	outFile << participant << ",";
	outFile << ( trainingTrial ? 0 : ( NBLOCKS - blocks.size() ) ) << ",";
	outFile << ( trainingTrial ? 0 : ( NTRIALSPERBLOCK - ( ( blocks.back().size() - 1 ) * NREPLICATESPERBLOCK + blocks.back().back().size() ) ) ) << ",";
	outFile << probe_training << ",";
	outFile << draw_probe << ",";
	outFile << show_probe_hints << ",";
	outFile << rm << ",";
	outFile << ( trainingTrial ? 0 : density ) << ",";
	outFile << ( trainingTrial ? 0 : lengthMultiplier ) << ",";
	outFile << ( trainingTrial ? 0 : thicknessMultiplier ) << ",";
	outFile << ( trainingTrial ? 0 : directionalGeomScale ) << ",";
	outFile << pVec.x << ",";
	outFile << pVec.y << ",";
	outFile << pVec.z << ",";
	outFile << tVec.x << ",";
	outFile << tVec.y << ",";
	outFile << tVec.z << ",";
	outFile << glm::length( targetCursor.getFlowVector() ) << ",";
	outFile << glm::degrees( getAngleError( targetQuat, probeQuat ) ) << ",";
	outFile << ( trainingTrial ? 0 : stopwatch.read() ) << std::endl;
}

bool Study::snapshotTGA( std::string filename, bool append_timestamp )
{
	// get frame buffer size
	int w, h;	
	glfwGetFramebufferSize(window, &w, &h);

	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w*h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize*sizeof(char));

	if (!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint)0, (GLint)0,
		(GLint)w, (GLint)h,
		GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	if (append_timestamp)
	{
		time_t t = time(0);   // get time now
		struct tm *now = localtime(&t);

		/*** DATE ***/
		// year
		filename += "_" + intToString(now->tm_year + 1900, 3) + "-";

		// month
		filename += intToString(now->tm_mon + 1, 1) + "-";
		
		// day
		filename += intToString(now->tm_mday, 1);

		/*** TIME ***/
		// hour
		filename += "_" + intToString(now->tm_hour, 1);
		
		// minute
		filename += "-" + intToString(now->tm_min, 1);
		
		// second
		filename += "-" + intToString(now->tm_sec, 1);
	}

	filename = "snapshots\\" + filename + ".tga";

	//Now the file creation
	FILE *filePtr = fopen(std::string( filename ).c_str(), "wb");
	if (!filePtr)
	{
		std::cerr << "ERROR: Could not save snapshot to " << filename << std::endl;
		std::cerr << "Make sure the path is valid and that the 'snapshots' directory exists and try again." << std::endl;
		return false;
	}


	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char header[6] = { w % 256, w / 256,
		h % 256, h / 256,
		24, 0 };
	// We write the headers
	fwrite(TGAheader, sizeof(unsigned char), 12, filePtr);
	fwrite(header, sizeof(unsigned char), 6, filePtr);
	// And finally our image data
	fwrite(dataBuffer, sizeof(GLubyte), nSize, filePtr);
	fclose(filePtr);

	std::cout << "Snapshot saved to " << filename << std::endl;

	return true;
}

std::string Study::intToString( int i, unsigned int pad_to_magnitude )
{
	if( pad_to_magnitude < 1 )
		return std::to_string( i );

	std::string ret;

	int mag = i == 0 ? 0 : (int) log10( i );

	for( int j = pad_to_magnitude - mag; j > 0; --j )
		ret += std::to_string( 0 );
	
	ret += std::to_string( i );

	return ret;
}