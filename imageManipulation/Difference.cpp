#include "global.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// define these static entities here
//vector<shared_ptr<Image>> Difference::imageVector;
//vector<DiffResult> Difference::state;
//mutex Difference::dataLock;

//atomic<int> Difference::activeThreads;
//const double Difference::maxPrecisLoss = 16.0;




// the main routine
int Difference::run(const int argc, const char** argv)
{
	GOL gol;

	// do the initialization loop
	if (!gol.initGLFW() || !gol.initShaders() ||  // !gol.initGeometry() ||
		!gol.initTextures())
		return -1;


	if (argc != 2)
	{
		cout << "Usage: [image] ...." << endl;
		cout << endl;
		cout << "* ERROR: you must supply one image to load." << endl;
		return -1;
	}

	return 0;
}


shared_ptr<SimpleTexture> Difference::loadImageDataIntoTexture(const char* file, uint index)
{
	cout << "* Attempting to load image \"" << file << "\"." << endl;

	// call STB
	int width, height, channels;
	float* pixels = stbi_loadf(file, &width, &height, &channels, 0);

	// null return? ERROR
	if (pixels == nullptr)
	{

		cerr << endl << "* ERROR: Could not load \"" << file << "\"." << endl;
		return NULL;
	}

	shared_ptr<SimpleTexture> texture = make_shared<SimpleTexture>(width, height, GL_RGB8);

	// set up sampling
	texture->setDownsampler(GL_NEAREST);
	texture->setUpsampler(GL_NEAREST);
	texture->setWrapping(GL_MIRRORED_REPEAT);


	int numPixels = width*height*channels;

	vector<GLfloat> imageData(numPixels);

	cout << "\nI store the image data in reverse order.\n";
	cout << "stbi_loadf() starts in the top left, textures sample from bottom left.\n";

	
	for (int i = 0; i < numPixels; i++)
	{
		imageData[numPixels - i - 1] = (pixels[i]);
	}


	texture->load(imageData);

	return texture;
}

vector<GLfloat> Difference::getVectorOfImageData(const char* file)
{
	cout << "* Attempting to load image \"" << file << "\"." << endl;

	// call STB
	int width, height, channels;
	GLfloat* pixels = stbi_loadf(file, &width, &height, &channels, 0);

	vector<GLfloat> imageData(width*height*channels);

	// null return? ERROR
	if (pixels == nullptr) {

		cerr << endl << "* ERROR: Could not load \"" << file << "\"." << endl;
		return vector<GLfloat>(1,-1);
	}
	else
		for (int i = 0; i < width*height*channels; i++)
			imageData.push_back(pixels[i]);


	return imageData;
}



/*
// convert an image into an Image
void Difference::loadImage(const char* file, uint index) {

	// increment our atomics
	activeThreads++;

	cout << "* Attempting to load image \"" << file << "\"." << endl;

	// call STB
	int width, height, channels;
	float* pixels = stbi_loadf(file, &width, &height, &channels, 0);

	// null return? ERROR
	if (pixels == nullptr) {

		cerr << endl << "* ERROR: Could not load \"" << file << "\"." << endl;
		activeThreads--;
		return;
	}

	// create an Image pointer to receive this data
	shared_ptr<Image> target = make_shared<Image>(width, height, channels);

	dataLock.lock();

	// if we're the first, life is easy
	if (index == 0)
		imageVector[0] = target;

	else {

		// no first image yet? defer until there is
		while (imageVector[0] == nullptr) {

			dataLock.unlock();
			Sleep(1000);		// make it quick
			dataLock.lock();
		}

		// now that there is, do we match the first image?
		if ((imageVector[0]->width() != (uint)width) ||
			(imageVector[0]->height() != (uint)height) ||
			(imageVector[0]->channels() != (uchar)channels)) {

			cerr << endl << "* ERROR: Image \"" << file <<
				"\" doesn't have the expected size." << endl;
			activeThreads--;
			dataLock.unlock();
			return;
		}

		imageVector[index] = target;
	}

	dataLock.unlock();

	// now start adding the raw data into the image
	ulong pixel = 0;
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			for (char c = 0; c < channels; c++)
				(*target)[y][x].set(c, pixels[pixel++]);

	// free up the buffer
	stbi_image_free(pixels);

	// flag we're finished and exit
	activeThreads--;

}
*/

/*
// calculate the metric we're interested in
void Difference::calcMetrics(uint a, uint b) {

	// a = x, b = y, y > x in all cases

	// increment our atomics
	activeThreads++;

	if (a > b) {		// shoot, gotta fix this

		uint temp = a;
		a = b;
		b = temp;
	}

	// create these before locking
	shared_ptr<Image> first = nullptr;
	shared_ptr<Image> second = nullptr;

	DiffResult results;
	results.x = a;
	results.y = b;

	uint resultsIndex;

	dataLock.lock();

	// store our results
	resultsIndex = state.size();
	state.push_back(results);

	// are we out of bounds? nullptrs?
	if ((a >= imageVector.size()) || (b >= imageVector.size()) ||
		(imageVector[a] == nullptr) || (imageVector[b] == nullptr)) {

		// nevermind
		dataLock.unlock();
		activeThreads--;
		return;
	}

	// otherwise, retrieve the appropriate images
	first = imageVector[a];
	second = imageVector[b];

	dataLock.unlock();

	// now, build up the stats
	vector<double> dsquared; dsquared.push_back(0.0);		// use these to preserve precision
	uint dsquaredIndex = 0;
	vector<double> dabs; dabs.push_back(0.0);
	uint dabsIndex = 0;

	for (uint y = 0; y < first->height(); y++) {

		for (uint x = 0; x < first->width(); x++)
			for (uchar c = 0; c < first->channels(); c++) {

				// read-only, so this is fine
				double temp = (double)((*first)[y][x].get(c) - (*second)[y][x].get(c));

				dsquared[dsquaredIndex] += temp * temp;		// careful to preserve precision
				if (dsquared[dsquaredIndex] > maxPrecisLoss) {
					dsquared.push_back(0.0);
					dsquaredIndex++;
				}

				dabs[dabsIndex] += fabs(temp);
				if (dabs[dabsIndex] > maxPrecisLoss) {
					dabs.push_back(0.0);
					dabsIndex++;
				}
			}

		// update our progress
		dataLock.lock();
		state[resultsIndex].progress = (float)(y + 1) / (float)first->height();
		dataLock.unlock();

	}

	// do our final gathering
	while (dsquared.size() > 1) {

		uint index = 0;
		while (index < (dsquared.size() - 1)) {

			dsquared[index] += dsquared[dsquared.size() - 1];
			dsquared.pop_back();		// more efficient than deleting a middle element
			index++;
		}


	}

	while (dabs.size() > 1) {

		uint index = 0;
		while (index < (dabs.size() - 1)) {

			dabs[index] += dabs[dabs.size() - 1];
			dabs.pop_back();
			index++;
		}
	}

	double total = 1.0 / (double)(first->pixels() * first->channels());
	results.mae = dabs[0] * total;

	double max = first->max();		// need the maximum value for PSNR
	double temp = second->max();
	if (temp > max)
		max = temp;

	temp = dsquared[0] * total;
	results.psnr = 20.0 * log10(max) - 10.0 * log10(temp);

	results.rmse = sqrt(temp);

	results.progress = 1.0;			// we are done, after all

									// update the stats and exit
	dataLock.lock();
	state[resultsIndex] = results;
	dataLock.unlock();

	activeThreads--;

}
*/


/*
// convert a pair of values into a linear index
int Difference::linearize(uint x, uint y) {

// the algorithm assumes y > x
if (x > y)
return linearize(y, x);

// we don't compare an image to itself
else if (x == y)
return -1;

else
return ((y*(y - 1)) >> 1) + x;

}
*/
