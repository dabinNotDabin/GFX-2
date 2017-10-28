#include "global.h"

/**************************************************************
* Yes, this approach means that you have to recompile the code
*  if you edit a shader. This should be ridiculously quick,
*  though, as you've probably cached your object files so
*  we're just appending strings and re-linking. In addition,
*  this technique also saves you from having to worry about
*  including the "data" directory in the right place.
*/

const string GOL::TextureShader = R"(
#version 430

// TextureShader
// incoming texture and scalar values
layout(binding=0) uniform sampler2D source;
uniform vec2 scalar;
uniform vec2 offset;

// set a colour scale
uniform vec4 dead = vec4( 0.0, 0.0, 0.1, 1.0 );
uniform vec4 alive = vec4( 1.0, 1.0, 0.9, 1.0 );

// write out the colour
out vec4 colour;

void main() {

	vec2 address = (gl_FragCoord.xy * scalar);// + offset;

	colour = texture2D( source, address );

	//float scalar = 1.0;		// tweak if out of bounds
	//if ( (address.x < 0.) || (address.x > 1.) ||
	//		(address.y < 0.) || (address.y > 1.) )
	//	scalar = 0.5;

	// just do a simple linear interpolation
	//colour = scalar * mix(dead, alive, value);

	//colour = vec4 (offset, value, 1.0);

	}
)";

const string GOL::CurveDrawingShader = R"(
#version 430

// CurveDrawingShader
// incoming texture and scalar values
layout(binding=0) uniform sampler2D source;
uniform vec2 scalar;
uniform vec2 offset;

// tweak these to change the representation
uniform float dead = 0.0;
uniform float reborn = 1.0;
uniform float old = 0.51;

// NOTE: switch to a jump table would reduce divergence, but won't speed
//  up this algorithm as it is limited by bandwidth and not calculation.

// write out the new value
out float colour;

// grab a value at a specific offset from the current pixel
int grab(vec2 off) {

	vec2 address = (gl_FragCoord.xy + off) * scalar;

	// added for testing!
	address = address + offset;

	// out of bounds? treat it as dead, despite the wrapping value
	if ( (address.x < 0.) || (address.y < 0.) || 
			(address.x > 1.) || (address.y > 1.) )
		return 0;

	float value = texture2D( source, address ).r;
	if (value > 0.5)
		return 1;
	else
		return 0;
	}

void main() {

	int sum = 0;

	// grab an X row
	sum += grab( vec2(-1.0, 1.0) );
	sum += grab( vec2( 0.0, 1.0) );
	sum += grab( vec2( 1.0, 1.0) );

	// then a second
	sum += grab( vec2( 1.0, 0.0) );
	int centre = grab( vec2( 0.0, 0.0 ) );
	sum += grab( vec2(-1.0, 0.0) );

	// check for an early exit
	if ( sum > 3 ) {
		colour = dead;
		return;
		}

	// otherwise, grab the last X row
	sum += grab( vec2(-1.0,-1.0) );
	sum += grab( vec2( 0.0,-1.0) );
	sum += grab( vec2( 1.0,-1.0) );

	// begin the final check
	if ( (sum > 3) || (sum < 2) )

		colour = dead;

	// if the current cell is alive, it survives on
	else if ( centre == 1 )

		colour = old;

	// a dead cell can come alive if it has three friends
	else if ( sum == 3 )

		colour = reborn;

	// otherwise, it's dead
	else
		colour = dead;

	}
)";

const string GOL::vertexShader = R"(
#version 430

layout(location = 0) in vec2 position;

uniform mat4 viewMatrix;

void main() {

	// boring pass-through
	gl_Position = vec4(position, 0.0, 1.0);
}
)";



const string GOL::FragmentShader = R"(
#version 430

layout(binding=0) uniform sampler2D source;
uniform vec2 scalar;
uniform vec2 offset;

// write out the colour
out vec4 colour;


void main() {

	vec2 address = (gl_FragCoord.xy * scalar); + offset;

	colour = texture2D( source, address );

	}
)";




