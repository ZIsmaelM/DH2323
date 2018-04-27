#include <iostream>
#include "glm/glm/glm.hpp"
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <unistd.h>
#include <cmath>

using namespace std;
using glm::vec3;
using glm::mat3;

struct Intersection
{
    vec3 position;
    float distance;
    int triangleIndex;
};

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
float alpha = 2 * atan(0.5f);

float fLength = SCREEN_HEIGHT / (2 * tan(alpha/2));
vec3 cameraPos;
vec3 camPos;


//vec3 rayOrigin = cameraPos;
vec3 rayDir;
Intersection closestIntersection;

float cameraAngle = 0;
float yaw = 0;

vec3 lightPos( 0, -0.5, -0.7 ); // Light position
vec3 lightColor = 14.f * vec3( 1, 1, 1 ); // Power for each color component
vec3 indirectLight = 0.5f * vec3(1,1,1);


int bounces = 1;
float r = 2;



// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
void triangleIntersect();
void RotateVec(vec3& in);
vec3 DirectLight( const Intersection& i );
vec3 lightBounce(vec3 n, vec3 v);

bool triangleIntersect( vec3 point )
{
    float t = point.x;
    float u = point.y;
    float v = point.z;
    
    if (t >= 0 && u >= 0 && v >= 0 && u + v <= 1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}




float Vec_distance(vec3 v1, vec3 v2)
{
    return sqrt( pow (v2.x - v1.x, 2) + pow (v2.y - v1.y, 2) + pow (v2.z - v1.z, 2) );
}






bool ClosestIntersection(
        vec3 start,
        vec3 dir,
        const vector<Triangle>& triangles,
        Intersection& closestIntersection
                         )
{
    bool intersection = false;
   
    float m = std::numeric_limits<float>::max();
    
    for (int i = 0; i < triangles.size(); i++)
    {
        vec3 v0 = triangles[i].v0;
        vec3 v1 = triangles[i].v1;
        vec3 v2 = triangles[i].v2;
       
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        
        vec3 b = start - v0;
        mat3 A( -dir, e1, e2 );
        
        vec3 x = glm::inverse( A ) * b;
        
        if(triangleIntersect(x))
        {
            vec3 r = start + dir * x.x;
            
            float d = Vec_distance(start, r);
            
            if( d < m )
            {
                m = d;
                closestIntersection.position = r;
                closestIntersection.triangleIndex = i;
                closestIntersection.distance = d;
            }
            intersection = true;
        }
    }
    return intersection;
}





int main( int argc, char* argv[] )
{
   
   
	
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    
    LoadTestModel ( triangles );
    
    camPos = vec3(0, 0, -3);
    cameraPos = vec3(0, 0, -3);
    
	t = SDL_GetTicks();	// Set start value for timer.

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
        //usleep(50000000);
        
	}

	//SDL_SaveBMP( screen, "screenshot.bmp" );
  
	return 0;
}


void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
    
    Uint8* keystate = SDL_GetKeyState( 0 );
    
    float velocity = 0.00001;
 
 
    if( keystate[SDLK_UP] )
    {
        // Move camera forward
        float distance = velocity * t;
        //std::cout << camPos.y << std::endl;
        //camPos = cameraPos;
        cameraPos.z += distance;
        //std::cout << camPos.y << std::endl;
        //yaw -= 0.05;
        camPos = cameraPos;
        //RotateVec(camPos);
        
    }
    if( keystate[SDLK_DOWN] )
    {
        // Move camera backward
        float distance = velocity * t;
        //camPos = cameraPos; //(0,0,-3)
        cameraPos.z -= distance;
        //yaw += 0.05;
        camPos = cameraPos;
        //RotateVec(camPos);
        
    }
    if( keystate[SDLK_LEFT] )
    {
        // Move camera to the left
        //float distance = velocity * t;
        //cameraPos.x -= distance;
        //yaw -= distance;
        cameraAngle += 0.05;
        camPos = cameraPos;
        RotateVec(camPos);
        
        
    }
    if( keystate[SDLK_RIGHT] )
    {
        // Move camera to the right
        //float distance = velocity * t;
        //cameraPos.x += distance;
        cameraAngle -= 0.05;
        camPos = cameraPos;
        RotateVec(camPos);
       
        
    }
    if( keystate[SDLK_r] )
    {
      
        camPos = cameraPos;
        
        
        
    }
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
    
    Intersection closest;


	for( int j=0; j<SCREEN_HEIGHT; ++j )
	{
		for( int i=0; i<SCREEN_WIDTH; ++i )
		{
            rayDir.x = i - SCREEN_WIDTH/2;
            rayDir.y = j - SCREEN_HEIGHT/2;
            rayDir.z = fLength;
            
            rayDir = glm::normalize(rayDir);
            
            RotateVec(rayDir);
            
            //vec3 color = vec3(0,0,0);
            
            
            bool a = ClosestIntersection(camPos, rayDir, triangles, closest);
            if (a)
            {
                vec3 Dlight = DirectLight(closest);
                
                vec3 color = (Dlight) * triangles[closest.triangleIndex].color;
                
                Intersection bounce;
                rayDir = lightBounce(triangles[closest.triangleIndex].normal, rayDir);
                
                if(bounces > 0 && ClosestIntersection(closest.position, rayDir, triangles, bounce))
                {
                    color += color*r*(DirectLight(bounce))*triangles[bounce.triangleIndex].color;
                    
                    for (int i = 0; i < bounces - 1; i++)
                    {
                        rayDir = lightBounce(triangles[bounce.triangleIndex].normal, rayDir);
                        if (ClosestIntersection(bounce.position, rayDir, triangles, bounce))
                        {
                            color += color*(r)*(DirectLight(bounce))*triangles[bounce.triangleIndex].color;
                        }
                    }
                }
                PutPixelSDL( screen, i, j, (indirectLight)*color);
            }
			
            else
            {
                PutPixelSDL( screen, i, j, vec3(0,0,0) );
            }
            
            
        }
        SDL_UpdateRect( screen, 0, 0, 0, 0 );
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
    
   
}

void RotateVec(vec3& in)
{
  

    // Rotate around y axis
    vec3 rights(cos(cameraAngle), 0, sin(cameraAngle));
    vec3 downs(0, 1, 0);
    vec3 front(-sin(cameraAngle),0, cos(cameraAngle));
    
    float x = glm::dot(rights, in);
    float y = glm::dot(downs, in);
    float z = glm::dot(front, in);
    
    in.x = x;
    in.y = y;
    in.z = z;
    
    
    // Rotate around x axis
    rights = vec3(1, 0, 0);
    downs = vec3(0, cos(yaw), -sin(yaw));
    front = vec3(0, sin(yaw), cos(yaw));
    
    x = glm::dot(rights, in);
    y = glm::dot(downs, in);
    z = glm::dot(front, in);
    
    in.x = x;
    in.y = y;
    in.z = z;
}


vec3 DirectLight( const Intersection& i )
{
    vec3 normal = triangles[i.triangleIndex].normal;
    vec3 r = i.position - lightPos;
    
    float d = sqrt(pow(r.x, 2) + pow(r.y, 2) + pow(r.z, 2));
    float P;
    
    if(glm::dot(normal, r) < 0)
    {
        Intersection sh;
        bool a = ClosestIntersection(lightPos, glm::normalize(r), triangles, sh);
        if(a)
        {
            if (sh.distance + 0.001 <= d)
            {
                P = 0;
            }
            else
            {
                P = (1.f / (4.f * M_PI * d * d));
            }
        }
        else
        {
            P = (1.f / (4.f * M_PI * d * d));
        }
        vec3 light = P * lightColor;
        return light;
    }
    return vec3(0,0,0);

   
}

vec3 lightBounce(vec3 n, vec3 v)
{
    float over = glm::dot(n, v);
    float under = glm::dot(n, n);
    vec3 proj = (over / under) * n;
    vec3 rest = proj- v;
    
    return glm::normalize(-rest - proj);
}



