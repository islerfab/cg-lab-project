#include "RenderProject.h"

/* Initialize the Project */
void RenderProject::init()
{
	bRenderer::loadConfigFile("config.json");	// load custom configurations replacing the default values in Configuration.cpp

	// let the renderer create an OpenGL context and the main window
	if(Input::isTouchDevice())
		bRenderer().initRenderer(true);										// full screen on iOS
	else
		bRenderer().initRenderer(1920, 1080, false, "The Dive - Demo");		// windowed mode on desktop
		//bRenderer().initRenderer(View::getScreenWidth(), View::getScreenHeight(), true);		// full screen using full width and height of the screen

	// start main loop 
	bRenderer().runRenderer();
}

/* This function is executed when initializing the renderer */
void RenderProject::initFunction()
{
	// get OpenGL and shading language version
	bRenderer::log("OpenGL Version: ", glGetString(GL_VERSION));
	bRenderer::log("Shading Language Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// initialize variables
	_offset = 0.0f;
	_randomOffset = 0.0f;
	_cameraSpeed = 40.0f;
	_running = false; _lastStateSpaceKey = bRenderer::INPUT_UNDEFINED;
	_viewMatrixHUD = Camera::lookAt(vmml::Vector3f(0.0f, 0.0f, 0.25f), vmml::Vector3f::ZERO, vmml::Vector3f::UP);

	// set shader versions (optional)
	bRenderer().getObjects()->setShaderVersionDesktop("#version 120");
	bRenderer().getObjects()->setShaderVersionES("#version 100");

	// load materials and shaders before loading the model
	ShaderPtr customShader = bRenderer().getObjects()->generateShader("customShader", { 2, true, true, true, true, true, true, true, true, true, false, false, false });	// automatically generates a shader with a maximum of 2 lights
	// ShaderPtr flameShader = bRenderer().getObjects()->loadShaderFile_o("flame", 0, AMBIENT_LIGHTING);				// load shader from file without lighting, the number of lights won't ever change during rendering (no variable number of lights)
	// MaterialPtr flameMaterial = bRenderer().getObjects()->loadObjMaterial("flame.mtl", "flame", flameShader);				// load material from file using the shader created above

	// create additional properties for a model
	PropertiesPtr causticProperties = bRenderer().getObjects()->createProperties("causticProperties");

	// fill arrays for plants
	for (int i = 0; i < NO_PLANTS; i++) {
		plantSizes[i] = rand() % 71 + 30;
		plantLocs[i] = vmml::Vector3f(float(rand() % 400 - 200), -198.0f, float(rand() % 400 - 200));
	}

	// set position of shark
	sharkPos = vmml::Vector3f(float(rand() % 200 - 100), -160.0f, float(rand() % 200 - 100));

	// set position of chest
	chestPos = vmml::Vector3f(float(rand() % 300 - 150), -198.0f, float(rand() % 300 - 150));

	// load models
	bRenderer().getObjects()->loadObjModel_o("dune.obj", 4, FLIP_Z | SHADER_FROM_FILE, causticProperties);								// automatically generates a shader with a maximum of 4 lights (number of lights may vary between 0 and 4 during rendering without performance loss)
	bRenderer().getObjects()->loadObjModel_o("cube.obj", 4, SHADER_FROM_FILE);
	bRenderer().getObjects()->loadObjModel_o("AG01_1.obj", customShader, FLIP_Z);
	bRenderer().getObjects()->loadObjModel_o("lambis_truncata_shell.obj", 4, FLIP_Z | SHADER_FROM_FILE);
    bRenderer().getObjects()->loadObjModel_o("Chest.obj", customShader, FLIP_Z);									// the custom shader created above is used
	bRenderer().getObjects()->loadObjModel_o("plane.obj", 4, SHADER_FROM_FILE);
	//bRenderer().getObjects()->loadObjModel_o("shark.obj", 4, SHADER_FROM_FILE);
	bRenderer().getObjects()->loadObjModel_o("shark.obj", customShader, FLIP_Z);
	bRenderer().getObjects()->loadObjModel_o("submarine.obj", 4, SHADER_FROM_FILE);
	bRenderer().getObjects()->loadObjModel_o("rock.obj", 4, SHADER_FROM_FILE);
	bRenderer().getObjects()->loadObjModel_o("debris.obj", 4, SHADER_FROM_FILE);

	// create sprites
	bRenderer().getObjects()->createSprite("bTitle", "basicTitle_light.png");							// create a sprite displaying the title as a texture

	// create text sprites
	FontPtr font = bRenderer().getObjects()->loadFont("KozGoPro-ExtraLight.otf", 50);
	if (Input::isTouchDevice())
		bRenderer().getObjects()->createTextSprite("instructions", vmml::Vector3f(0,0,255), "Try to find the treasure before your air runs out \n \nDouble tap to start", font);
	else
		bRenderer().getObjects()->createTextSprite("instructions", vmml::Vector3f(1.f, 1.f, 1.f), "Press space to start", font);

	bRenderer().getObjects()->createTextSprite("dead", vmml::Vector3f(0, 0, 255), "SHARK FOOD OMNOMNOM", font);

	bRenderer().getObjects()->createTextSprite("win", vmml::Vector3f(0, 0, 255), "YOU FOUND THE TREASURE GREAT JOB", font);

	// create camera
	bRenderer().getObjects()->createCamera("camera", vmml::Vector3f(0.0f, 0.0f, 0.0f), vmml::Vector3f(0.f, -M_PI_F / 2, 0.f));

	// create lights
	// bRenderer().getObjects()->createLight("dayLight", vmml::Vector3f(0.0f, 0.0f, 0.0f), vmml::Vector3f(1.0f, 0.85f, 0.7f), 10.0f, 0.01f, 300.0);
	bRenderer().getObjects()->createLight("headLamp", -bRenderer().getObjects()->getCamera("camera")->getPosition(), vmml::Vector3f(1.0f, 1.0f, 1.0f), 1000.0f, 0.5f, 300.0f);

	// postprocessing
	bRenderer().getObjects()->createFramebuffer("fbo");					// create framebuffer object
	bRenderer().getObjects()->createTexture("fbo_texture1", 0.f, 0.f);	// create texture to bind to the fbo
	bRenderer().getObjects()->createTexture("fbo_texture2", 0.f, 0.f);	// create texture to bind to the fbo
	ShaderPtr blurShader = bRenderer().getObjects()->loadShaderFile_o("blurShader", 0);			// load shader that blurs the texture
	MaterialPtr blurMaterial = bRenderer().getObjects()->createMaterial("blurMaterial", blurShader);								// create an empty material to assign either texture1 or texture2 to
	bRenderer().getObjects()->createSprite("blurSprite", blurMaterial);		// create a sprite using the material created above

	// Update render queue
	updateRenderQueue("camera", 0.0f);
}

/* Draw your scene here */
void RenderProject::loopFunction(const double &deltaTime, const double &elapsedTime)
{
//	bRenderer::log("FPS: " + std::to_string(1 / deltaTime));	// write number of frames per second to the console every frame

	//// Draw Scene and do post processing ////

	/// Begin post processing ///
	GLint defaultFBO;
	if (!_running){
		bRenderer().getView()->setViewportSize(bRenderer().getView()->getWidth() / 5, bRenderer().getView()->getHeight() / 5);		// reduce viewport size
		defaultFBO = Framebuffer::getCurrentFramebuffer();	// get current fbo to bind it again after drawing the scene
		bRenderer().getObjects()->getFramebuffer("fbo")->bindTexture(bRenderer().getObjects()->getTexture("fbo_texture1"), false);	// bind the fbo
	}

	/// Draw scene ///	
	
	bRenderer().getModelRenderer()->drawQueue(/*GL_LINES*/);
	bRenderer().getModelRenderer()->clearQueue();
	
	if (!_running){
		/// End post processing ///		
        /*** Blur ***/
		// translate
		vmml::Matrix4f modelMatrix = vmml::create_translation(vmml::Vector3f(0.0f, 0.0f, -0.5));
		// blur vertically and horizontally
		bool b = true;		int numberOfBlurSteps = 2;
		for (int i = 0; i < numberOfBlurSteps; i++) {
			if (i == numberOfBlurSteps - 1){
				bRenderer().getObjects()->getFramebuffer("fbo")->unbind(defaultFBO); //unbind (original fbo will be bound)
				bRenderer().getView()->setViewportSize(bRenderer().getView()->getWidth(), bRenderer().getView()->getHeight());								// reset vieport size
			}
			else
				bRenderer().getObjects()->getFramebuffer("fbo")->bindTexture(bRenderer().getObjects()->getTexture(b ? "fbo_texture2" : "fbo_texture1"), false);
			bRenderer().getObjects()->getMaterial("blurMaterial")->setTexture("fbo_texture", bRenderer().getObjects()->getTexture(b ? "fbo_texture1" : "fbo_texture2"));
			bRenderer().getObjects()->getMaterial("blurMaterial")->setScalar("isVertical", static_cast<GLfloat>(b));
			// draw
			bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getModel("blurSprite"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
			b = !b;
		}
	
        /*** Title ***/
        // translate and scale 
        GLfloat titleScale = 0.5f;
        vmml::Matrix4f scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
		modelMatrix = vmml::create_translation(vmml::Vector3f(-0.4f, 0.0f, -0.65f)) * scaling;
        // draw
		bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getModel("bTitle"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false, false);

		/*** Instructions ***/
        titleScale = 0.08f;
        scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
        modelMatrix = vmml::create_translation(vmml::Vector3f(-0.9f / bRenderer().getView()->getAspectRatio(), -0.6f, -0.65f)) * scaling;
        // draw
		if (_dead) {
			bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("dead"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
		}
		else if (_win) {
			bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("win"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
		}
		else {
			bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("instructions"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
		}
    }

	//// Camera Movement ////
	updateCamera("camera", deltaTime);
	
	// Change the deltas for waves
	if (_running) {
		if (deltaTime > 0.0f) {
			_offset += 5 * deltaTime;
			_randomOffset += (randomNumber(10.0f, 20.0f)) * deltaTime;
		}
	}

	//// Head Lamp ////
	// set the light to be at the camera position
	bRenderer().getObjects()->getLight("headLamp")->setPosition(-bRenderer().getObjects()->getCamera("camera")->getPosition());

	/// Update render queue ///
	updateRenderQueue("camera", deltaTime);

	// Quit renderer when escape is pressed
	if (bRenderer().getInput()->getKeyState(bRenderer::KEY_ESCAPE) == bRenderer::INPUT_PRESS)
		bRenderer().terminateRenderer();
}

/* This function is executed when terminating the renderer */
void RenderProject::terminateFunction()
{
	bRenderer::log("I totally terminated this Renderer :-)");
}

/* Update render queue */
void RenderProject::updateRenderQueue(const std::string &camera, const double &deltaTime)
{
	// Set offset
	bRenderer().getObjects()->getProperties("causticProperties")->setScalar("offset", _offset);		// pass offset for wave effect
	bRenderer().getObjects()->getProperties("causticProperties")->setVector("waterAmbient", vmml::Vector3f(0.1, 0.1f, 0.15f));		// pass ambient color (could be changing dynamically)

	// Cube
	vmml::Matrix4f modelMatrix = vmml::create_translation(vmml::Vector3f(0.0f, 0.0f, 0.0f)) * vmml::create_scaling(vmml::Vector3f(2.0f));
	// submit to render queue
	bRenderer().getModelRenderer()->queueModelInstance("cube", "cube_instance", camera, modelMatrix, std::vector<std::string>({ "headLamp" }));

	// Floor
	modelMatrix = vmml::create_translation(vmml::Vector3f(5.0f, -199.0f, 5.0f)) * vmml::create_scaling(vmml::Vector3f(0.22f)) * vmml::create_translation(vmml::Vector3f(-500.0f, 0.0f, 0.0f));
	// First quarter
	bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
	// Second quarter
	modelMatrix = vmml::create_translation(vmml::Vector3f(-5.0f, -199.0f, 5.0f)) * vmml::create_scaling(vmml::Vector3f(0.22f)) * vmml::create_translation(vmml::Vector3f(500.0f, 0.0f, 0.0f));
	bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance2", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
	// Third quarter
	modelMatrix = vmml::create_translation(vmml::Vector3f(5.0f, -199.0f, -5.0f)) * vmml::create_scaling(vmml::Vector3f(0.22f)) * vmml::create_translation(vmml::Vector3f(-500.0f, 0.0f, 1000.0f));
	bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance3", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
	// Fourth quarter
	modelMatrix = vmml::create_translation(vmml::Vector3f(-5.0f, -199.0f, -5.0f)) * vmml::create_scaling(vmml::Vector3f(0.22f)) * vmml::create_translation(vmml::Vector3f(500.0f, 0.0f, 1000.0f));
	bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance4", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);

	// plane
	modelMatrix = vmml::create_translation(vmml::Vector3f(10.0f, -180.0f, 10.0f)) * vmml::create_rotation(float(45 * M_PI / 180), vmml::Vector3f::UNIT_X) * vmml::create_rotation(float (45 * M_PI / 180), vmml::Vector3f::UNIT_Z) * vmml::create_scaling(vmml::Vector3f(0.22f));
	bRenderer().getModelRenderer()->queueModelInstance("plane", "plane_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    
	// shark
	modelMatrix = vmml::create_translation(sharkPos) * vmml::create_scaling(vmml::Vector3f(0.4f)) * vmml::create_rotation(float(M_PI), vmml::Vector3f::UNIT_Y);
	bRenderer().getModelRenderer()->queueModelInstance("shark", "shark_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);

	// submarine
	modelMatrix = vmml::create_translation(vmml::Vector3f(10.0f, -20.0f, 50.0f)) * vmml::create_scaling(vmml::Vector3f(0.4f));
	bRenderer().getModelRenderer()->queueModelInstance("submarine", "submarine_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);

	// rock
	modelMatrix = vmml::create_translation(vmml::Vector3f(30.0f, -190.0f, 30.0f)) * vmml::create_scaling(vmml::Vector3f(0.2f));
	bRenderer().getModelRenderer()->queueModelInstance("rock", "rock_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);

	// debris
	modelMatrix = vmml::create_translation(vmml::Vector3f(30.0f, -190.0f, -70.0f)) * vmml::create_scaling(vmml::Vector3f(2.2f));
	bRenderer().getModelRenderer()->queueModelInstance("debris", "debris_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    
    
	/*** Plants ***/
	for (int i = 0; i < NO_PLANTS; i++) {
		modelMatrix = vmml::create_translation(plantLocs[i]) * vmml::create_scaling(plantSizes[i]);
		// submit to render queue
		// bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
		bRenderer().getModelRenderer()->queueModelInstance("AG01_1", &"plant_" [i], camera, modelMatrix, std::vector<std::string>({ "headLamp" }));
	}
    
    /*** Treasure ***/
    // translate and scale
    modelMatrix = vmml::create_translation(chestPos) * vmml::create_scaling(vmml::Vector3f(0.24f));
    // submit to render queue
    // bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
    bRenderer().getModelRenderer()->queueModelInstance("Chest", "treasure", camera, modelMatrix, std::vector<std::string>({ "headLamp" }));    
    
	/*** Shells ***/
	int count = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// translate and scale
			modelMatrix = vmml::create_translation(vmml::Vector3f(i*5.0f-1.0, -198.0f, j*5.0-1.0f)) * vmml::create_scaling(vmml::Vector3f(10.0f));
			// submit to render queue
			// bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
			bRenderer().getModelRenderer()->queueModelInstance("lambis_truncata_shell", &"shell_instance_" [ count++], camera, modelMatrix, std::vector<std::string>({ "headLamp" }));
		}
	}
	
	// reset ambient color
	bRenderer().getObjects()->setAmbientColor(bRenderer::DEFAULT_AMBIENT_COLOR());

}

/* Camera movement */
void RenderProject::updateCamera(const std::string &camera, const double &deltaTime)
{
	//// Adjust aspect ratio ////
	bRenderer().getObjects()->getCamera(camera)->setAspectRatio(bRenderer().getView()->getAspectRatio());

	double deltaCameraY = 0.0;
	double deltaCameraX = 0.0;
	double cameraForward = 0.0;
	double cameraSideward = 0.0;

	//vmml::Vector3f oldcampos = bRenderer().getObjects()->getCamera(camera)->getPosition();
	//std::cout << oldcampos << std::endl;

	/* iOS: control movement using touch screen */
	if (Input::isTouchDevice()){

		// pause using double tap
		if (bRenderer().getInput()->doubleTapRecognized()){
			_running = !_running;
		}

		if (_running){
			// control using touch
			TouchMap touchMap = bRenderer().getInput()->getTouches();
			int i = 0;
			for (auto t = touchMap.begin(); t != touchMap.end(); ++t)
			{
				Touch touch = t->second;
				// If touch is in left half of the view: move around
				if (touch.startPositionX < bRenderer().getView()->getWidth() / 2){
					cameraForward = -(touch.currentPositionY - touch.startPositionY) / 100;
					cameraSideward = (touch.currentPositionX - touch.startPositionX) / 100;

				}
				// if touch is in right half of the view: look around
				else
				{
					deltaCameraY = (touch.currentPositionX - touch.startPositionX) / 2000;
					deltaCameraX = (touch.currentPositionY - touch.startPositionY) / 2000;
				}
				if (++i > 2)
					break;
			}
		}

	}
	/* Windows: control movement using mouse and keyboard */
	else{
		// use space to pause and unpause
		GLint currentStateSpaceKey = bRenderer().getInput()->getKeyState(bRenderer::KEY_SPACE);
		if (currentStateSpaceKey != _lastStateSpaceKey)
		{
			_lastStateSpaceKey = currentStateSpaceKey;
			if (currentStateSpaceKey == bRenderer::INPUT_PRESS){
				_running = !_running;
				bRenderer().getInput()->setCursorEnabled(!_running);
			}
		}

		// mouse look
		double xpos, ypos; bool hasCursor = false;
		bRenderer().getInput()->getCursorPosition(&xpos, &ypos, &hasCursor);

		deltaCameraY = (xpos - _mouseX) / 1000;
		_mouseX = xpos;
		deltaCameraX = (ypos - _mouseY) / 1000;
		_mouseY = ypos;

		if (_running){
			// movement using wasd keys
			if (bRenderer().getInput()->getKeyState(bRenderer::KEY_W) == bRenderer::INPUT_PRESS)
				if (bRenderer().getInput()->getKeyState(bRenderer::KEY_LEFT_SHIFT) == bRenderer::INPUT_PRESS) 			cameraForward = 2.0;
				else			cameraForward = 1.0;
			else if (bRenderer().getInput()->getKeyState(bRenderer::KEY_S) == bRenderer::INPUT_PRESS)
				if (bRenderer().getInput()->getKeyState(bRenderer::KEY_LEFT_SHIFT) == bRenderer::INPUT_PRESS) 			cameraForward = -2.0;
				else			cameraForward = -1.0;
			else
				cameraForward = 0.0;

			if (bRenderer().getInput()->getKeyState(bRenderer::KEY_A) == bRenderer::INPUT_PRESS)
				cameraSideward = -1.0;
			else if (bRenderer().getInput()->getKeyState(bRenderer::KEY_D) == bRenderer::INPUT_PRESS)
				cameraSideward = 1.0;
			if (bRenderer().getInput()->getKeyState(bRenderer::KEY_UP) == bRenderer::INPUT_PRESS)
				bRenderer().getObjects()->getCamera(camera)->moveCameraUpward(_cameraSpeed*deltaTime);
			else if (bRenderer().getInput()->getKeyState(bRenderer::KEY_DOWN) == bRenderer::INPUT_PRESS)
				bRenderer().getObjects()->getCamera(camera)->moveCameraUpward(-_cameraSpeed*deltaTime);
			if (bRenderer().getInput()->getKeyState(bRenderer::KEY_LEFT) == bRenderer::INPUT_PRESS)
				bRenderer().getObjects()->getCamera(camera)->rotateCamera(0.0f, 0.0f, 0.03f*_cameraSpeed*deltaTime);
			else if (bRenderer().getInput()->getKeyState(bRenderer::KEY_RIGHT) == bRenderer::INPUT_PRESS)
				bRenderer().getObjects()->getCamera(camera)->rotateCamera(0.0f, 0.0f, -0.03f*_cameraSpeed*deltaTime);
		}
	}

	//// Update camera ////
	if (_running){
		bRenderer().getObjects()->getCamera(camera)->moveCameraForward(cameraForward*_cameraSpeed*deltaTime);
		bRenderer().getObjects()->getCamera(camera)->rotateCamera(deltaCameraX, deltaCameraY, 0.0f);
		bRenderer().getObjects()->getCamera(camera)->moveCameraSideward(cameraSideward*_cameraSpeed*deltaTime);
		
		// check if near shark
		vmml::Vector3f campos = bRenderer().getObjects()->getCamera(camera)->getPosition();
		if (campos.distance(-sharkPos) < 40) {
			_running = false;
			_dead = true;
		}
		else if (campos.distance(-chestPos) < 20) {
			_running = false;
			_win = true;
		}
		/*vmml::Vector3f newcampos = bRenderer().getObjects()->getCamera(camera)->getPosition();
		if ((newcampos.y() < -30.0f) || (newcampos.y() > 23.0f)){
			bRenderer().getObjects()->getCamera(camera)->setPosition(oldcampos);
		}*/
		//std::cout << bRenderer().getObjects()->getCamera(camera)->getPosition() << std::endl;
	}	
}

/* For iOS only: Handle device rotation */
void RenderProject::deviceRotated()
{
	if (bRenderer().isInitialized()){
		// set view to full screen after device rotation
		bRenderer().getView()->setFullscreen(true);
		bRenderer::log("Device rotated");
	}
}

/* For iOS only: Handle app going into background */
void RenderProject::appWillResignActive()
{
	if (bRenderer().isInitialized()){
		// stop the renderer when the app isn't active
		bRenderer().stopRenderer();
	}
}

/* For iOS only: Handle app coming back from background */
void RenderProject::appDidBecomeActive()
{
	if (bRenderer().isInitialized()){
		// run the renderer as soon as the app is active
		bRenderer().runRenderer();
	}
}

/* For iOS only: Handle app being terminated */
void RenderProject::appWillTerminate()
{
	if (bRenderer().isInitialized()){
		// terminate renderer before the app is closed
		bRenderer().terminateRenderer();
	}
}

/* Helper functions */
GLfloat RenderProject::randomNumber(GLfloat min, GLfloat max){
	return min + static_cast <GLfloat> (rand()) / (static_cast <GLfloat> (RAND_MAX / (max - min)));
}
