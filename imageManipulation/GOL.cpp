#include "global.h"

// our minimal latency between frames or responses
const float GOL::tic = 1.0f / 60.0f;

// a lot of this is similar to DragonGL
mutex GOL::keyQueueLock;
list<keyAction> GOL::keyQueue;

// so expect a lot of cut-and-paste
bool GOL::terminate(string m) {

	cerr << "ERROR: " << m << endl;
	glfwTerminate();
	return false;
}

// set up the RNG-related functions here
GOL::GOL() {

	RNG = mt19937(duration_cast<nanoseconds>(
		high_resolution_clock::now().time_since_epoch()).count());

	dist = uniform_int_distribution<short>(0, 1);

}

// this one has a bit more variety, though
int GOL::run(int argc, const char ** argv) {

	// do the initialization loop
//	if (!initGLFW() || !initShaders() || !initGeometry() ||
//		!initTextures())
//		return -1;

	// sanity check on these
//	if (!vertexArray || !bufferDst || !bufferSrc)
//		return -2;

//	last = steady_clock::now();

	while (live) {

		render();
		iterate();
		keyInput();
	}

	cleanup();
	return 0;
}


bool GOL::initGLFW() {

	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cerr << "ERROR: GLFW did not initialize!" << endl;
		return false;
	}

	// Use OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "CPSC 453 Game of Life Example", 0, 0);

	// was the context created?
	if (!window)
		return terminate("Could not create a render context!");

	glfwMakeContextCurrent(window);


	// initialize glew to handle access to GL functions, must be done after window is set to context
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}



	// set up an error callback
	glfwSetErrorCallback([](int error, const char* description) {

		cerr << "ERROR: GLFW threw error " << error << ":" << endl;
		cerr << description << endl;
	});

	// ditto keyboard callback
	glfwSetKeyCallback(window, GOL::keyCallback);

	return !OpenGL::error("GOL::initGLFW() assert");

}

bool GOL::initShaders() {

	// build up the GLSL programs, iterator first
	if (!CurveDrawingProgram.attachShader(vertexShader, GL_VERTEX_SHADER))
		return terminate("No luck with the iterative vertex shader, quitting.");

	if (!CurveDrawingProgram.attachShader(CurveDrawingShader, GL_FRAGMENT_SHADER))
		return terminate("No luck with the iterative fragment shader, quitting.");

	if (!CurveDrawingProgram.link())
		return terminate("Couldn't link the iterative shader program, quitting.");


	// then the displayer
	if (!TextureProgram.attachShader(vertexShader, GL_VERTEX_SHADER))
		return terminate("No luck with the drawing vertex shader, quitting.");

	if (!TextureProgram.attachShader(TextureShader, GL_FRAGMENT_SHADER))
		return terminate("No luck with the drawing fragment shader, quitting.");

	if (!TextureProgram.link())
		return terminate("Couldn't link the drawing shader program, quitting.");


	// The image display program
	if (!ImageDisplayProgram.attachShader(vertexShader, GL_VERTEX_SHADER))
		return terminate("No luck with the drawing vertex shader, quitting.");

	if (!ImageDisplayProgram.attachShader(FragmentShader, GL_FRAGMENT_SHADER))
		return terminate("No luck with the drawing fragment shader, quitting.");

	if (!ImageDisplayProgram.link())
		return terminate("Couldn't link the drawing shader program, quitting.");



	return !OpenGL::error("GOL::initShaders() assert");
}


// create the geometry we need
bool GOL::initGeometry() {

	// quite simple, as we only need a full-screen pair of tris
	vector<GLfloat> data = 
	{	
		-1., -1., 1., -1., -1., 1.,
		-1.,  1., 1., -1.,  1., 1.
	};
	
	vertexArray = make_shared<VertexArray>(2, data);

	return !OpenGL::error("GOL::initGeometry() assert");
}


// create or re-create the textures that contain the board
bool GOL::initTextures() {

	// hurrah for smart pointers!
	bufferDst = genBoard(0);	// destination = blank
	bufferSrc = genBoard(1);	// source = random

								// double-check the textures were made
	return (!bufferDst || !bufferSrc || !OpenGL::error("GOL::initGeometry() assert"));

}


void GOL::cleanup() {

	// skip freeing up buffers, it doesn't seem to effect anything
	glfwDestroyWindow(window);
	glfwTerminate();

}

// the handler is pretty simple, just tossing the input into a queue
void GOL::keyCallback(GLFWwindow* w, int a, int b, int c, int d) {

	keyQueueLock.lock();
	keyQueue.push_back({ a, b, c, d });
	keyQueueLock.unlock();

}

// this is where the real action is for keyboard input
void GOL::keyInput() {

	// grab key events
	glfwPollEvents();

	// grab exclusive control of the keyboard queue
	keyQueueLock.lock();
	while (keyQueue.size() > 0) {

		// read off the keypress and branch
		keyAction ka = keyQueue.front();

		// was this a release? ignore
		if (ka.action == GLFW_RELEASE) {

			keyQueue.pop_front();
			continue;
		}

		uint initGame = 1;	// this helps select the proper board

							// otherwise, branch based on the key
		switch (ka.key) {

			// numeric keys: control sim/draw speed
		case GLFW_KEY_1:
			delayTics = 1.0;
			break;

		case GLFW_KEY_2:
			delayTics = 2.0;
			break;

		case GLFW_KEY_3:
			delayTics = 4.0;
			break;

		case GLFW_KEY_4:
			delayTics = 8.0;
			break;

		case GLFW_KEY_5:
			delayTics = 16.0;
			break;

		case GLFW_KEY_6:
			delayTics = 32.0;
			break;

		case GLFW_KEY_7:
			delayTics = 64.0;
			break;

			// keypad keys: change starting point
		case GLFW_KEY_KP_6:		// fall through!
			initGame++;

		case GLFW_KEY_KP_5:
			initGame++;

		case GLFW_KEY_KP_4:
			initGame++;

		case GLFW_KEY_KP_3:
			initGame++;

		case GLFW_KEY_KP_2:
			initGame++;

		case GLFW_KEY_KP_1:
			initGame++;

		case GLFW_KEY_KP_0:
			bufferSrc = genBoard(initGame);
			bufferDst = genBoard(0);	// ensure the sizes match
			break;

			// up/down: change zoom
		case GLFW_KEY_UP:

			zoom *= 0.70710678118654752440f;
			if (zoom < maxZoom)
				zoom = maxZoom;
			break;

		case GLFW_KEY_DOWN:

			// slightly more efficient than multiply-then-check
			if (zoom < 0.70710678118654752440f)
				zoom *= 1.41421356237309504880f;
			else if (zoom != 1.0)
				zoom = 1.0;
			break;

		case GLFW_KEY_HOME:
		case GLFW_KEY_BACKSPACE:
		case GLFW_KEY_DELETE:

			// reset with the default layout
			initTextures();
			break;

		case GLFW_KEY_SPACE:
		case GLFW_KEY_ENTER:

			// toggle the simulation
			active = !active;
			break;

		case GLFW_KEY_TAB:

			// turbo mode!
			turbo = !turbo;
			break;

		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
		case GLFW_KEY_X:

			// quit!
			live = false;
			break;

		}

		// discard the processed key
		keyQueue.pop_front();
	}

	keyQueueLock.unlock();

	// catch mouse clicks, too
	if (glfwWindowShouldClose(window))
		live = false;

}

// render the board
void GOL::render() {

	// sanity check on these
	if (!vertexArray || !bufferDst || !bufferSrc) {

		live = false;

		cout << "GOL::render - error here:" << endl;
//		return;
	}

	// read off the window size and update OpenGL, if necessary
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	if ((w != width) || (h != height)) {

		width = w;
		height = h;
		glViewport(10, 10, width-20, height-20);
		cout << "Window size: " << width << "x" << height << endl;
	}

	// calculate the scaling and offset values for the fragment shader
	GLfloat scalar[2];
	GLfloat offset[2];	// these are used by the shader to properly display the board

	float texWidth = texture->getWidth();
	float texHeight = texture->getHeight();

	float widthScale = (float)texture->getWidth() / (float)width;
	float heightScale = (float)texture->getHeight() / (float)height;

	// Need to work with aspect ratio.
	// Get ratio of image width/height to screen width/height
	// Whichever is less than or equal to 1 (default if both == 1), use as limiting factor
	//

	scalar[0] = widthScale / (float)texture->getWidth();
	scalar[1] = heightScale / (float)texture->getHeight();

	offset[0] = 0.5 * (1.0 - zoom * (float)width / (float)texture->getWidth());
	offset[1] = 0.5 * (1.0 - zoom * (float)height / (float)texture->getHeight());




	mat4 translateToBottomLeft = translate(mat4(1.0f), vec3(-1.0f, -1.0f, 0.0f));
	mat4 scaleToFit = scale(mat4(1.0f), vec3(widthScale, heightScale, 1.0f));

	mat4 viewMatrix = scaleToFit;



	// ensure the texture is linearly interpolated
	texture->setDownsampler(GL_LINEAR);
	texture->setUpsampler(GL_LINEAR_MIPMAP_LINEAR);

	// display the game board
	Framebuffer::unbind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	// do all the drawing
	bool cond = TextureProgram.bind();
	
	if (cond)
		cond = TextureProgram.setVec2("scalar", scalar[0], scalar[1]);

//	if (cond)
//		cond = TextureProgram.setVec2("offset", offset[0], offset[1]);

	if (cond)
		cond = TextureProgram.setTexture("source", texture);

//	if (cond)
//		cond = TextureProgram.setMat4("viewMatrix", viewMatrix);

	if (cond)
		cond = vertexArray->draw();

	live = cond;

	if (!live)				// might as well quit now
		return;

	// then transfer the buffer over
	glfwSwapBuffers(window);
}





















// go through one iteration of the board
void GOL::iterateOne() {

	// sanity check on these
	if (!vertexArray || !texture)// || !bufferDst || !bufferSrc)
	{
		live = false;
		return;
	}

	// set up the framebuffer
	frameBuffer.setTexture(texture);
	if (!frameBuffer.bind()) {

		live = false;
		return;
	}

	// figure out the proper scalars
	GLfloat scalar[2];
	scalar[0] = 1.0 / (float)texture->getWidth();
	scalar[1] = 1.0 / (float)texture->getHeight();

	// ensure the texture is nearest-neighbour
	texture->setDownsampler(GL_NEAREST);
	texture->setUpsampler(GL_NEAREST);

	// do all the binding and drawing
	if (!ImageDisplayProgram.bind() ||
		!ImageDisplayProgram.setVec2("scalar", scalar[0], scalar[1]) ||
		!ImageDisplayProgram.setTexture("source", texture) ||
		!vertexArray->draw()) {

		live = false;
		return;
	}

	// unbind the framebuffer
	Framebuffer::unbind();

	// swap texture buffers
	shared_ptr<SimpleTexture> temp = bufferDst;
	bufferDst = bufferSrc;
	bufferSrc = temp;

}

// maybe iterate over the board?
void GOL::iterate() {

	// are we active?
	if (active) {

		// if turbo mode is activated, loop until we've run out of time
		if (turbo) {

			float goal = delayTics*tic;	// pre-calculate these
			uint limit = (uint)delayTics;

			auto end = steady_clock::now();
//			while (seconds(end - last).count() < goal) {

				// clear any existing commands and draw
				for (uint it = 0; it < limit; it++) {

					iterateOne();		// reduce overhead by unrolling
					iterateOne();
					iterateOne();
					iterateOne();
				}

				// update the time and force a finish
				end = steady_clock::now();
				OpenGL::finish();
			}
//		}

		// no turbo? then has enough time elapsed?
 		else if (elapsed > delayTics*tic) {

			// draw one iteration, then reset the elapsed time
			iterateOne();
			elapsed = 0.0;

		}
	}

	// increment the elapsed time and reset the start of the interval
	auto end = steady_clock::now();
	elapsed += seconds(end - last).count();
	last = end;

}


// generate a pre-defined board
shared_ptr<SimpleTexture> GOL::genBoard(uint type) {

	// first, generate a SimpleTexture to contain it
	shared_ptr<SimpleTexture> retVal = make_shared<SimpleTexture>(width, height, GL_R8);

	// set up sampling
	retVal->setDownsampler(GL_NEAREST);
	retVal->setUpsampler(GL_NEAREST);
	retVal->setWrapping(GL_MIRRORED_REPEAT);

	// pre-calculate this
	uint actual = type - 2;		// may overflow!

								// branch, depending on the requested board
	if (type == 0)

		retVal->load();			// create a blank board

	else if ((type > 1) && (actual < presets.size()) &&
		((uint)width >= presets[actual][0].size()) &&		// ensure the board doesn't overflow
		((uint)height >= presets[actual].size())) {

		vector<float> data;		// create a specific game board
		data.reserve(width * height);
		// pad out the top
		uint padTop = (height - presets[actual].size()) >> 1;
		for (uint y = 0; y < padTop; y++)
			for (uint x = 0; x < (uint)width; x++)
				data.push_back(0.0);

		// draw the board, with padding
		uint padLeft = (width - presets[actual][0].size()) >> 1;
		for (uint y = 0; y < presets[actual].size(); y++) {

			for (uint x = 0; x < padLeft; x++)
				data.push_back(0.0);

			for (uint x = 0; x < presets[actual][y].size(); x++)
				data.push_back(presets[actual][y][x]);

			uint remainder = width - padLeft - presets[actual][y].size();
			for (uint x = 0; x < remainder; x++)
				data.push_back(0.0);

		}

		// pad the bottom
		uint remainder = height - padTop - presets[actual].size();
		for (uint y = 0; y < remainder; y++)
			for (uint x = 0; x < (uint)width; x++)
				data.push_back(0.0);

		retVal->load(data);		// load it up
	}

	else {			// all else fails, do a random board

		vector<float> data;		// create a random field of data
		data.reserve(width * height);
		for (uint it = 0; it < (uint)(width * height); it++)
			data.push_back((float)dist(RNG));

		retVal->load(data);		// load it up
	}


	cout << "Texture size (" << retVal << "): " << width << "x" << height << endl;
	return retVal;

}
