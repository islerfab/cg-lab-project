#include "RenderProject.h"


#include <stdio.h>
#include <time.h>

/* Initialize the Project */
void RenderProject::init()
{
    bRenderer::loadConfigFile("config.json");	// load custom configurations replacing the default values in Configuration.cpp
    
    // let the renderer create an OpenGL context and the main window
    if(Input::isTouchDevice())
        bRenderer().initRenderer(true);										// full screen on iOS
    else
        bRenderer().initRenderer(1920, 1080, false, "The Dive - Demo");		// windowed mode on desktop
    
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
	bRenderer().getObjects()->setShaderVersionES("#version 120");

	// load materials and shaders before loading the model
	ShaderPtr customShader = bRenderer().getObjects()->generateShader("customShader", { 2, true, true, true, true, true, true, true, true, true, false, false, false });

	// create additional properties for a model
	PropertiesPtr causticProperties = bRenderer().getObjects()->createProperties("causticProperties");


	// create text sprites
	FontPtr font = bRenderer().getObjects()->loadFont("KozGoPro-ExtraLight.otf", 50);
    
    // set shader versions (optional)
    bRenderer().getObjects()->setShaderVersionDesktop("#version 120");
    bRenderer().getObjects()->setShaderVersionES("#version 100");
    bRenderer().getObjects()->createTextSprite("gamename", vmml::Vector3f(1,1,1), "TREASURE SEEKER", font);
    

    bRenderer().getObjects()->createTextSprite("gameOver", vmml::Vector3f(1,1,1), "GAME OVER", font);
    
 
    bRenderer().getObjects()->createTextSprite("tryAgain", vmml::Vector3f(1,1,1), "Double tap to try again", font);
    
	srand((float)time(NULL));
    // fill arrays for plants
    for (int i = 0; i < NO_PLANTS; i++) {
        plantSizes[i] = rand() % 71 + 30;
        plantPos[i] = vmml::Vector3f(float(rand() % 800 - 400), -198.0f, float(rand() % 800 - 400));
    }
    
    // fill arrays for bottles
    for (int i = 0; i < NO_BOTTLES; i++) {
        bottleSize[i] = 1.0f;
        bottleDraw[i] = true;
        bottlePos[i] = vmml::Vector3f(float(rand() % 800 - 400), -198.0f, float(rand() % 800 - 400));
    }
    
    
    // set position of shark
	sharkHeight = -160.0f;
	//sharkPos = vmml::Vector3f(10.0f, sharkHeight, 10.0f);
    sharkPos = vmml::Vector3f(float(rand() % 600 - 300), sharkHeight, float(rand() % 600 - 300));
	//std::cout << "sharkpos: " << sharkPos << std::endl;

	// set position of chest
	//chestPos = vmml::Vector3f(float(rand() % 1000 - 150), -193.0f, float(rand() % 1000 - 150));
	chestPos = vmml::Vector3f(180.0f, -198.0f, -120.0f);
    
    if (Input::isTouchDevice()){
        bRenderer().getObjects()->createTextSprite("instructions", vmml::Vector3f(1,1,1), "Try to find the treasure before your air runs out \n \nFind some extra air on your way - but be aware of the shark! \n \nDouble tap to start", font);}
    else{
        bRenderer().getObjects()->createTextSprite("instructions", vmml::Vector3f(1.f, 1.f, 1.f), "Try to find the treasure before your air runs out \n \nFind some extra air on your way - but be aware of the shark! \n \nPress space to start", font);}

	// create camera
	bRenderer().getObjects()->createCamera("camera", vmml::Vector3f(0.0f, 0.0f, 0.0f), vmml::Vector3f(0.f, -M_PI_F / 2, 0.f));
    
    // load models
    bRenderer().getObjects()->loadObjModel_o("dune.obj", 4, FLIP_Z | SHADER_FROM_FILE, causticProperties);								// automatically generates a shader with a maximum of 4 lights (number of lights may vary between 0 and 4 during rendering without performance loss)
    bRenderer().getObjects()->loadObjModel_o("AG01_1.obj", customShader, FLIP_Z);
    bRenderer().getObjects()->loadObjModel_o("Chest.obj", 4, SHADER_FROM_FILE, causticProperties);									// the custom shader created above is used
    bRenderer().getObjects()->loadObjModel_o("plane.obj", 4, SHADER_FROM_FILE, causticProperties);
    bRenderer().getObjects()->loadObjModel_o("shark.obj", 4, FLIP_Z | SHADER_FROM_FILE, causticProperties);
    bRenderer().getObjects()->loadObjModel_o("submarine.obj", 4, SHADER_FROM_FILE, causticProperties);
	bRenderer().getObjects()->loadObjModel_o("temple.obj", 4, SHADER_FROM_FILE, causticProperties);
    bRenderer().getObjects()->loadObjModel_o("bottle.obj", 4, SHADER_FROM_FILE, causticProperties);
	bRenderer().getObjects()->loadObjModel_o("ray.obj", 0, FLIP_Z | SHADER_FROM_FILE, causticProperties);

    //game state
    for(int i = 0; i <= 100; i += 5){
        bRenderer().getObjects()->createTextSprite("gameState "+std::to_string(i), vmml::Vector3f(1,1,1), "air:"+std::to_string(100-i)+"%", font);
    }
    

    bRenderer().getObjects()->createTextSprite("dead", vmml::Vector3f(1, 1, 1), "SHARK FOOD OMNOMNOM", font);
    
    bRenderer().getObjects()->createTextSprite("win", vmml::Vector3f(1, 1, 1), "YOU FOUND THE TREASURE GREAT JOB", font);
    
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
        
        else if(!_gameOver){
            
            /*** Instructions ***/
            titleScale = 0.08f;
            scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
            modelMatrix = vmml::create_translation(vmml::Vector3f(-0.9f / bRenderer().getView()->getAspectRatio(), -0.6f, -0.65f)) * scaling;
            // draw
            bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("instructions"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
            
            titleScale = 0.18f;
            scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
            modelMatrix = vmml::create_translation(vmml::Vector3f(-0.9f / bRenderer().getView()->getAspectRatio(), 0.6f, -0.65f)) * scaling;
            bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("gamename"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
            
        }else if(_gameOver){
            //Game Over screen
            titleScale = 0.18f;
            scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
            modelMatrix = vmml::create_translation(vmml::Vector3f(-0.9f / bRenderer().getView()->getAspectRatio(), 0.6f, -0.65f)) * scaling;
            bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("gameOver"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
            
            /*** Instructions to Try again ***/
            titleScale = 0.08f;
            scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
            modelMatrix = vmml::create_translation(vmml::Vector3f(-0.9f / bRenderer().getView()->getAspectRatio(), -0.6f, -0.65f)) * scaling;
            // draw
            bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("tryAgain"), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
            
        }
        
        
    }
    

	if (_running) {
        
        if (deltaTime > 0.0f) {
            _offset += 5 * deltaTime;
            _randomOffset += (randomNumber(10.0f, 20.0f)) * deltaTime;
            
        }
        
        // update shark position
        vmml::Vector3f sharkTemp = vmml::normalize(vmml::Vector3f(sharkPos.x(), 0.0f, sharkPos.z()));
        sharkUp = sharkTemp.cross(vmml::Vector3f(0.0f, 1.0f, 0.0f));
        sharkPos -= sharkUp;
        //std::cout << "update: " << sharkPos << std::endl;
        
        vmml::Matrix4f modelMatrix = vmml::create_translation(vmml::Vector3f(0.0f, 0.0f, -0.5));
        
        GLfloat titleScale = 0.1f;
        vmml::Matrix4f scaling = vmml::create_scaling(vmml::Vector3f(titleScale / bRenderer().getView()->getAspectRatio(), titleScale, titleScale));
        modelMatrix = vmml::create_translation(vmml::Vector3f(-0.4f, 0.0f, -0.65f)) * scaling;
        
        modelMatrix = vmml::create_translation(vmml::Vector3f(-0.9f / bRenderer().getView()->getAspectRatio(), 0.7f, -0.65f)) * scaling;
        bRenderer().getModelRenderer()->drawModel(bRenderer().getObjects()->getTextSprite("gameState "+std::to_string(_airCounter)), modelMatrix, _viewMatrixHUD, vmml::Matrix4f::IDENTITY, std::vector<std::string>({}), false);
    
	
        //the higher the more time you have to find the treasure
        if((int)_offset % 20 < 10 && !_lostAir)
        {
			_lostAir = true;
            _airCounter = _airCounter+5;
        } else if ((int)_offset % 20 >= 10)
        {
			_lostAir = false;
        }
        
        if(_airCounter >= 100){
            
            /// GAME OVER ///
            
            //reset counter
            _gameOver = true;
            _running = !_running;
        }
        
        
     

	}
    
    //// Camera Movement ////
    updateCamera("camera", deltaTime);
    // Change the deltas for waves
    
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

    // Floor
    vmml::Matrix4f modelMatrix = vmml::create_translation(vmml::Vector3f(50.0f, -199.0f, 50.0f)) * vmml::create_scaling(vmml::Vector3f(2.20f)) * vmml::create_translation(vmml::Vector3f(-500.0f, 0.0f, 0.0f));
    // First quarter
    bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    // Second quarter
    modelMatrix = vmml::create_translation(vmml::Vector3f(-50.0f, -199.0f, 50.0f)) * vmml::create_scaling(vmml::Vector3f(2.20f)) * vmml::create_translation(vmml::Vector3f(500.0f, 0.0f, 0.0f));
    bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance2", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    // Third quarter
    modelMatrix = vmml::create_translation(vmml::Vector3f(50.0f, -199.0f, -50.0f)) * vmml::create_scaling(vmml::Vector3f(2.20f)) * vmml::create_translation(vmml::Vector3f(-500.0f, 0.0f, 1000.0f));
    bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance3", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    // Fourth quarter
    modelMatrix = vmml::create_translation(vmml::Vector3f(-50.0f, -199.0f, -50.0f)) * vmml::create_scaling(vmml::Vector3f(2.20f)) * vmml::create_translation(vmml::Vector3f(500.0f, 0.0f, 1000.0f));
    bRenderer().getModelRenderer()->queueModelInstance("dune", "dune_instance4", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    
    // plane
    modelMatrix = vmml::create_translation(vmml::Vector3f(70.0f, -180.0f, 10.0f)) * vmml::create_rotation(float(45 * M_PI / 180), vmml::Vector3f::UNIT_X) * vmml::create_rotation(float (45 * M_PI / 180), vmml::Vector3f::UNIT_Z) * vmml::create_scaling(vmml::Vector3f(0.22f));
    bRenderer().getModelRenderer()->queueModelInstance("plane", "plane_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);
    
    // shark
	modelMatrix = vmml::create_translation(sharkPos) * vmml::create_scaling(vmml::Vector3f(0.4f)) * vmml::create_rotation(float(1 * acos(sharkUp.dot(vmml::Vector3f::UNIT_Z) / sharkUp.length())), vmml::Vector3f::UNIT_Y);// * vmml::create_rotation(float(M_PI), vmml::Vector3f::UNIT_Y);
    bRenderer().getModelRenderer()->queueModelInstance("shark", "shark_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);

    // submarine
    modelMatrix = vmml::create_translation(vmml::Vector3f(10.0f, -140.0f, 50.0f)) * vmml::create_scaling(vmml::Vector3f(0.4f));
    bRenderer().getModelRenderer()->queueModelInstance("submarine", "submarine_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);


    /*** Plants ***/
    for (int i = 0; i < NO_PLANTS; i++) {
        modelMatrix = vmml::create_translation(plantPos[i]) * vmml::create_scaling(plantSizes[i]);
        // submit to render queue
        // bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
        bRenderer().getModelRenderer()->queueModelInstance("AG01_1", &"plant_" [i], camera, modelMatrix, std::vector<std::string>({ "headLamp" }));
    }
    
    /*** AIR BOTTLES ***/
    for (int i = 0; i < NO_BOTTLES; i++) {
        modelMatrix = vmml::create_translation(bottlePos[i]) * vmml::create_scaling(bottleSize[i]);
        // submit to render queue
        if(bottleDraw[i] == true){
             bRenderer().getModelRenderer()->queueModelInstance("bottle", &"bottle_instance" [i], camera, modelMatrix, std::vector<std::string>({ "headLamp" }));
        }
       
    }
    
    
    /*** Treasure ***/
    // translate and scale
    modelMatrix = vmml::create_translation(chestPos) * vmml::create_scaling(vmml::Vector3f(0.24f));
    // submit to render queue
    // bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
    bRenderer().getModelRenderer()->queueModelInstance("Chest", "treasure", camera, modelMatrix, std::vector<std::string>({ "headLamp" }));

	// temple
	modelMatrix = vmml::create_translation(vmml::Vector3f(-160.0f, -200.0f, 80.0f)) * vmml::create_scaling(vmml::Vector3f(1.0f));
	bRenderer().getModelRenderer()->queueModelInstance("temple", "temple_instance1", camera, modelMatrix, std::vector<std::string>({ "headLamp" }), false);

	// God Rays
	int count = 0;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			// translate and scale
			modelMatrix = vmml::create_translation(vmml::Vector3f(i*150.0f, -180.0f, j*150.0f)) * vmml::create_rotation(float(M_PI / 4), vmml::Vector3f::UNIT_X) * vmml::create_scaling(vmml::Vector3f(1.0f, 1.0f, 15.0f));
			// submit to render queue
			// bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
			bRenderer().getModelRenderer()->queueModelInstance("ray", &"ray_instance_"[count++], camera, modelMatrix, std::vector<std::string>());

			modelMatrix = vmml::create_translation(vmml::Vector3f(-i*150.0f, -180.0f, j*150.0f)) * vmml::create_rotation(float(M_PI / 4), vmml::Vector3f::UNIT_X) * vmml::create_scaling(vmml::Vector3f(1.0f, 1.0f, 15.0f));
			// submit to render queue
			// bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
			bRenderer().getModelRenderer()->queueModelInstance("ray", &"ray_instance_"[count++], camera, modelMatrix, std::vector<std::string>());

			modelMatrix = vmml::create_translation(vmml::Vector3f(i*150.0f, -180.0f, -j*150.0f)) * vmml::create_rotation(float(M_PI / 4), vmml::Vector3f::UNIT_X) * vmml::create_scaling(vmml::Vector3f(1.0f, 1.0f, 15.0f));
			// submit to render queue
			// bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
			bRenderer().getModelRenderer()->queueModelInstance("ray", &"ray_instance_"[count++], camera, modelMatrix, std::vector<std::string>());

			modelMatrix = vmml::create_translation(vmml::Vector3f(-i*150.0f, -180.0f, -j*150.0f)) * vmml::create_rotation(float(M_PI / 4), vmml::Vector3f::UNIT_X) * vmml::create_scaling(vmml::Vector3f(1.0f, 1.0f, 15.0f));
			// submit to render queue
			// bRenderer().getObjects()->setAmbientColor(vmml::Vector3f(0.2f, 0.2f, 1.0f));
			bRenderer().getModelRenderer()->queueModelInstance("ray", &"ray_instance_"[count++], camera, modelMatrix, std::vector<std::string>());
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
    
    
    /* iOS: control movement using touch screen */
    if (Input::isTouchDevice()){
        
        // pause using double tap
        if (bRenderer().getInput()->doubleTapRecognized()){
            _running = !_running;
            
            if(_dead || _win || _gameOver){
                resetGame();
            
            }
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
                
                if(_dead || _win || _gameOver){
                    resetGame();
                    
                }
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
		//std::cout << "pos: " << campos << std::endl;
        if (campos.distance(-sharkPos) < 40) {
            _running = false;
            _dead = true;
        }
        else if (campos.distance(-chestPos) < 30) {
            _running = false;
            _win = true;
        }
        else{
            for(int i = 0; i< NO_BOTTLES; i++){
                if (campos.distance(-(bottlePos[i])) < 30 && bottleDraw[i] == true){
                    if(_airCounter > 0){
                        _airCounter = _airCounter-20;
                        if(_airCounter < 0){
                            _airCounter = 0;
                        }
                    }
                    bottleDraw[i] = false;
                }
                
            }

        }
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

void RenderProject::resetGame(){
    bRenderer().getObjects()->getCamera("camera")->resetCamera();
    _dead = false;
    _win = false;
    _gameOver = false;
    _airCounter = 0;
    init();
    initFunction();
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
