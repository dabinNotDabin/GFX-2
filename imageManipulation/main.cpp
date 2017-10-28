

#include "global.h"

using namespace glm;
using std::cout;
using std::endl;
using std::cerr;
using std::string;


// just pass through
int main(int argc, const char** argv) {

	Difference diff;
	GOL gol;
	

	if (!gol.initGLFW() || !gol.initShaders() || !gol.initGeometry()) // || !gol.initTextures())  
		return -1;


	//vector<GLfloat> pixels = diff.getVectorOfImageData(argv[1]);

	//diff.vertexArray = make_shared<VertexArray>(2, pixels);
	gol.texture = diff.loadImageDataIntoTexture(argv[1], 0);



//	gol.frameBuffer.setTexture(gol.texture);
//	if (!gol.frameBuffer.bind())
//	{
//		return 0;
//	}


	while (1)
		gol.run(argc, argv);
	

	// uniforms for scale and translate
	// when the user zooms, modify the scale uniform matrix.
	// multiply by the uniforms in the vertex shader

	return diff.run(argc, argv);
}
