#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <unistd.h>

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
float focalLength = 1;//(SCREEN_WIDTH/2) / tan(45/2);
vec3 cameraPos(0,0,0.99);
float maxFloat = std::numeric_limits<float>::max();
bool test = true;
int pixelX;
int pixelY;

// ----------------------------------------------------------------------------
// FUNCTIONS

struct Ray
{
	vec3 origin;
	vec3 direction;
};
// Object that stores intersection data
struct Intersection
{
	vec3 position;
	float distance;
	int triIndex;
};

void Update();
void Draw();
void TriangleIntersect();
void PrintVec3(vec3 v);
bool IsEqual(vec3 A, vec3 B);
vec3 ComputeIntersectPoint(Ray, float t);
bool ClosestIntersection(Ray, const vector<Triangle>& triangles, Intersection& ClosestIntersection);

int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
		cout << "render finished" << endl;
		usleep(500000000);	// delay for 500s
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	// generate triangles
	LoadTestModel(triangles);
	// for (int z = 0; z < triangles.size(); z++) {
	// 	PrintVec3(triangles[z].color);
	// }
	for( int y=0; y<SCREEN_HEIGHT; ++y ) {
		for( int x=0; x<SCREEN_WIDTH; ++x ) {
			pixelX = x;
			pixelY = y;
			// set ray direction and origin
			Ray ray;
			ray.origin = cameraPos;
			ray.direction = vec3(x-(SCREEN_WIDTH/2), y-(SCREEN_HEIGHT/2), focalLength);
			Intersection intersect;

			// check if an intersection was found
			bool foo = ClosestIntersection(ray, triangles, intersect);
			vec3 color(0,0,0);
			if (foo) {
				// cout << "rayOrig: (" << rayOrig.x << "," << rayOrig.y << "," << rayOrig.z << ")" << endl;
				// cout << "rayDir: (" << rayDir.x << "," << rayDir.y << "," << rayDir.z << ")" << endl;	
				// cout << "index: " << intersect.triangleIndex << endl;	
				color = triangles[intersect.triIndex].color;
			}

			//vec3 color( 1, 0.5, 0.5 );
			PutPixelSDL( screen, x, y, color );
		}
		//cout << "Finished row: " << y << endl;
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

Intersection TriangleIntersect(const vector<Triangle>& triangles, int index, Ray ray) {

	// points on a ray: r = s + td
	// s = ray orig
	// d = ray dir
	// t = pos on line (t >= 0)

	// points on a triangle: r = v0 + u*e1 + v*e2
	// v0 = coordinate system origin
	// e1 & e2 = vectors parallel to triangle edges
	// u & v = scalar values between 0 and 1, and u + v < 1

	Triangle t = triangles[index];
	Intersection rayTri;
	rayTri.distance = maxFloat;
	// calculate vectors parallel to triangle edges
	vec3 e1 = t.v1 - t.v0;
	vec3 e2 = t.v2 - t.v0;

	// s + td = v0 + u*e1 + v*e2
	// s - v0 = u*e1 + v*e2 - td
	vec3 b = ray.origin - t.v0;
	//cout << b[0] << "	" << b[1] << "	" << b[2] << endl;
	mat3 A( -ray.direction, e1, e2);
	vec3 x = glm::inverse(A) * b; // (t u v)

	// do not update intersection if ray does not intersect triangle
	if (x[0] < 0 || x[1] < 0 || x[2] < 0 || x[1] + x[2] > 1)
		return rayTri;

	rayTri.position = ComputeIntersectPoint(ray, x[0]);
	rayTri.distance = x[0];
	rayTri.triIndex = index;
	return rayTri;

}

Intersection CramerTriIntersection(const vector<Triangle>& triangles, int index, Ray ray) {
    //Programming TASK 1: Implement this method
    //Your code should compute the intersection between a ray and a triangle.
    //
    //If you detect an intersection, the return type should look similar to this:
    //if(rayIntersectsTriangle)
    //{
    //  intersection = RayIntersection(ray,shared_from_this(),lambda,ray.normal,uvw);
    //  return true;
    //} 
    //
    // Hints :lambda
    // Ray origin p_r : ray.getOrigin()
    // Ray direction t_r : ray.getDirection()
    // Compute the intersection point using ray.pointOnRay(lambda) 

	Triangle tri = triangles[index];
	Intersection intersect;
	intersect.distance = maxFloat;

	vec3 t1 = tri.v1 - tri.v0;
	vec3 t2 = tri.v2 - tri.v0;
	vec3 normal = glm::normalize(glm::cross(t1, t2));

	double d = glm::dot(ray.direction, tri.normal);
	//if (fabs(d) < 0.0001) return intersect;

	double a = glm::dot((tri.v0 - ray.origin), tri.normal);
	double lambda = a / d;
	if (lambda < 0 || lambda + 0.0001 > 500) return intersect;

	vec3 intersectPoint = ComputeIntersectPoint(ray, lambda);

	// inside outside test
	vec3 edge0 = tri.v1 - tri.v0;
	vec3 edge1 = tri.v2 - tri.v1;
	vec3 edge2 = tri.v0 - tri.v2;
	vec3 intersectionV0 = intersectPoint - tri.v0;
	vec3 intersectionV1 = intersectPoint - tri.v1;
	vec3 intersectionV2 = intersectPoint - tri.v2;

	vec3 C = glm::cross(edge0, intersectionV0);
	if (glm::dot(normal, C) < 0) return intersect;
	
	C = glm::cross(edge1, intersectionV1);
	if (glm::dot(normal, C) < 0) return intersect;

	C = glm::cross(edge2, intersectionV2);
	if (glm::dot(normal, C) < 0) return intersect;

	intersect.position = intersectPoint;
	intersect.distance = lambda;
	intersect.triIndex = index;
	return intersect;
}

bool ClosestIntersection(Ray ray, const vector<Triangle>& triangles, Intersection& ClosestIntersection) {

	bool intersectDetected = false;

	// Intersection arr[triangles.size()];
	// check if ray intersects any of the triangles
	float closestDistance = maxFloat;
	Intersection newIntersection;
	for (int i = 0; i < triangles.size(); i++) {
		//Intersection newIntersection = TriangleIntersect(triangles, i, ray);
		if (pixelX == 200 && pixelY == 300) {
			PrintVec3(triangles[i].color);
		}
		newIntersection = TriangleIntersect(triangles, i, ray);
		
		float newDist = newIntersection.distance;
		//cout << newDist << endl;
		if (newIntersection.distance < closestDistance) {
			intersectDetected = true;
			closestDistance = newIntersection.distance;
			ClosestIntersection = newIntersection;
			if (IsEqual(vec3(0.15,0.15,0.75),triangles[i].color))
				cout << "blue" << endl;
		}
	}

	return intersectDetected;
}

bool IsEqual(vec3 A, vec3 B) {
	if (A[0] == B[0] && A[1] == B[1] && A[2] == B[2])
		return true;

	return false;
}

vec3 ComputeIntersectPoint(Ray ray, float t) {
	vec3 point(0,0,0);
	point.x = ray.origin.x + (t * ray.direction.x);
	point.y = ray.origin.y + (t * ray.direction.y);
	point.z = ray.origin.z + (t * ray.direction.z);

	return point;
}

void PrintVec3(vec3 v) {
	cout << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")" << endl;
}