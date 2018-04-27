// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "SDL/SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
int starTime;
float starVelocity;
float dt;
vector<vec3> stars(1000);
SDL_Surface* screen;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

//void Draw();
void DrawColors();
void DrawStars();
void Update();
void Assign(vec3, vector<vec3>&, int);
void Interpolate(float, float, vector<vec3>&);
void InterpolateVec3(vec3, vec3, vector<vec3>&);

// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main( int argc, char* argv[] )
{

	///////////////////////
	//		Numbers		///
	///////////////////////
	std::vector<vec3> result(4);
	vec3 a(1,4,9.2);
	vec3 b(4,1,9.8);
	InterpolateVec3(a,b,result);
	for (int i=0; i<result.size(); ++i) {
		cout << "( "
		<< result[i].x << ", "
		<< result[i].y << ", "
		<< result[i].z << " ) ";
	}
	cout << "" << endl;

	// Set each star's initial position to a random location
	float r = float(rand()) / float(RAND_MAX);
	for (int i = 0; i < stars.size(); ++i) {
		stars[i].x = float(rand()-rand()) / float(RAND_MAX);
		stars[i].y = float(rand()-rand()) / float(RAND_MAX);
		stars[i].z = float(rand()) / float(RAND_MAX);
	}

	starVelocity = 0;
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	int blur = 0;
	while( NoQuitMessageSDL() )
	{
		if (blur == 30) {
			SDL_FillRect( screen, 0, 0 );
			blur = 0;
		}
		Update();
		DrawStars();
		blur++;
		starVelocity = 0.0005;
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void DrawColors()
{
	vec3 topLeft(1,0,0); // red
	vec3 topRight(0,0,1); // blue
	vec3 bottomLeft(0,1,0); // green
	vec3 bottomRight(1,1,0); // yellow

	vector<vec3> leftSide( SCREEN_HEIGHT );
	vector<vec3> rightSide( SCREEN_HEIGHT );
	InterpolateVec3( topLeft, bottomLeft, leftSide );
	InterpolateVec3( topRight, bottomRight, rightSide );
	
	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{

		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vector<vec3> interpolated(SCREEN_WIDTH);
			InterpolateVec3(leftSide[y],rightSide[y],interpolated);
			PutPixelSDL( screen, x, y, interpolated[x] );

			// vec3 color(0,0,1);
			// PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void DrawStars()
{

	int f = SCREEN_HEIGHT / 2;
	for( int s=0; s<stars.size(); ++s ) {

		stars[s].z -= starVelocity * dt;
		if (stars[s].z <= 0)
			stars[s].z += 1;
		if (stars[s].z > 1)
			stars[s].z -= 1;

		float u = f * ((float)stars[s].x/stars[s].z) + (SCREEN_WIDTH/2);
		float v = f * ((float)stars[s].y/stars[s].z) + (SCREEN_HEIGHT/2);

		vec3 starColor = (0.2f*vec3(1,1,1)) / (stars[s].z*stars[s].z);
		PutPixelSDL( screen, u, v, starColor);
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void Assign( vec3 a, vector<vec3>& vectArray, int index) {
	vectArray[index].x = a.x;
	vectArray[index].y = a.y;
	vectArray[index].z = a.z;
}

void Update() {
	int t2 = SDL_GetTicks();
	dt = float(t2-starTime);
	starTime = t2;
}

void Interpolate( float a, float b, vector<float>& result) {
	int samples = result.size();
	float dist = b - a;
	result[0] = a;
	result[samples-1] = b;

	for (int i = 1; i <= samples; ++i) {
		float mag = i/samples;
		int L = (samples-mag)*a;
		int R = mag*b;
		result[i] = (int)(i*(dist/samples)+a);
		cout << result[i] << " ";
	}
	cout << "" << endl;
}

void InterpolateVec3( vec3 a, vec3 b, vector<vec3>& result) {
	int numSamples = result.size();
	int numPoints = 3;

	int dist;
	for (int i = 0; i < numSamples; ++i) {
		for (int j = 0; j < numPoints; ++j) {
			float dist = b[j] - a[j];
			float t = ((float)i/(numSamples-1)) * dist;
			result[i][j] = a[j] + t;
		}
	}
}