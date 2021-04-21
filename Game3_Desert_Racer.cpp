// Game_3_Desert_Racer .cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <iostream>  // enables the use of cout and end line
#include <time.h>	    // Used for random number generating and timers
#include <sstream> // enables the use of string stream
#include <vector>	    // Dynamic Arrays - Better than standard arrays if values in the sequence need changing
using namespace tle;
using namespace std; // Standard definitions

enum hoverCarStates { waiting, racing, destroyedCar };

enum gameStates { playing, paused, over};

// Create a 2D Vector

struct vector2D
{
	float x;
	float z;
};

vector2D scalar(float s, vector2D v)
{
	return{ s * v.x, s * v.z };
}

vector2D sum3(vector2D v1, vector2D v2, vector2D v3)
{
	return{v1.x + v2.x + v3.x, v1.z + v2.z + v3.z};
}

enum collision {none, xAxis, zAxis };

collision Sphere2BoxCollision2D(float sphereOldX, float sphereOldZ, float sphereX, float sphereZ, float sphereRadius,
	float boxX, float boxZ, float halfBoxWidth, float halfBoxDepth)
{
	// set up bounding box
	float maxX = boxX + halfBoxWidth + sphereRadius;
	float minX = boxX - halfBoxWidth - sphereRadius;
	float maxZ = boxZ + halfBoxWidth + sphereRadius;
	float minZ = boxZ - halfBoxWidth - sphereRadius;

	// check if centre of sphere is within bounding box
	if (sphereX > minX && sphereX < maxX && sphereZ > minZ && sphereZ < maxZ);
	{
		// collision - test for axis of collission
		if (sphereOldX < minX || sphereOldX > maxX)
		{
			return xAxis; // colliding parallel to the x axis
		}
		else
		{
			return zAxis; // colliding parallel to the z axis

		}
	}
	//else
	{
		return none;
	}
}

// EKeyCode's for movement of the car

const EKeyCode kMoveForwardsKey = Key_W;
const EKeyCode kMoveBackwardsKey = Key_S;
const EKeyCode kMoveLeftKey = Key_A;
const EKeyCode kMoveRightKey = Key_D;
const EKeyCode kPauseKey = Key_P;
const EKeyCode kQuitKey = Key_Escape;

// set amount of walls, checkpoints and Isles

const int kCheckpointAmount = 3;
const int kWallAmount = 4;
const int kIsleAmount = 6;

// sky position
const float skyYPos = -960.0f;

// hover positions
const float hoverCarXPos = 0.5f;
const float hoverCarYPos = 0.5f;
const float hoverCarZPos = -30.0f;
const float hoverCarRot = 5.0f;
const float kSpeedInc = 0.05f;
const float kHoverCarSpeed = 0.05f;

// checkpoint Rotation
const float checkpointRot = 90.0f;

// EKeyCode's for pausinng/ unpause game

const EKeyCode kPauseGame = Key_P;

// EKeyCode's for quitting the game
const EKeyCode kQuitGame = Key_Escape;

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("media");

	/**** Set up your scene here ****/

	// create ground model
	IMesh* groundMesh = myEngine->LoadMesh("ground.x");
	
	// create sky model
	IMesh* skyMesh = myEngine->LoadMesh("SkyBox 07.x");
	IModel* sky = skyMesh->CreateModel(0, skyYPos, 0);

	// create the hover car model
	IMesh* hoverMesh = myEngine->LoadMesh("race2.x");
	IModel* hoverCar = hoverMesh->CreateModel( hoverCarXPos, hoverCarYPos, hoverCarZPos );

	// create checkpoints
	IMesh* checkpointMesh = myEngine->LoadMesh("checkpoint.x");
	
	IModel* checkpoint[kCheckpointAmount];
	float checkpointXs[kCheckpointAmount] = { 0, 10, 25 };
	float checkpointYs[kCheckpointAmount] = { 0, 0, 0 };
	float checkpointZs[kCheckpointAmount] = { 0, 120, 56 };
	for (int i = 0; i < kCheckpointAmount; i++)
	{
		checkpoint[i] = checkpointMesh->CreateModel(checkpointXs[i], checkpointYs[i], checkpointZs[i]);
		
	}
	checkpoint[1]->RotateY(checkpointRot);

	// create walls

	IMesh* wallMesh = myEngine->LoadMesh("wall.x");

	IModel* wall[kWallAmount];
	float wallXs[kWallAmount] = { -10, 10, -10, 10 };
	float wallYs[kWallAmount] = { 0, 0, 0, 0 };
	float wallZs[kWallAmount] = { 48, 48, 64, 64 };
	for (int i = 0; i < kWallAmount; i++)
	{
		wall[i] = wallMesh->CreateModel(wallXs[i], wallYs[i], wallZs[i]);

	}
	
	const float halfCubeDimension = 1.0f * 1.0f;
	const float carCollisionRad = 5.0f;
	// create isles

	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");

	IModel* isle[kIsleAmount];
	float isleXs[kIsleAmount] = { -10, 10, -10, 10, -10, 10 };
	float isleYs[kIsleAmount] = { 0, 0, 0, 0, 0, 0 };
	float isleZs[kIsleAmount] = { 40, 40, 56, 56, 72, 72 };
	for (int i = 0; i < kIsleAmount; i++)
	{
		isle[i] = isleMesh->CreateModel(isleXs[i], isleYs[i], isleZs[i]);

	}

	// load mesh
	//IModel* checkpoint = checkpointMesh->CreateModel();
	IModel* ground = groundMesh->CreateModel();

	// create camera
	ICamera* myCamera = myEngine->CreateCamera(kFPS);
	myCamera->AttachToParent(hoverCar);

	// create momentum, thrust

	vector2D momentum{0.0f, 0.0f};
	vector2D thrust{ 0.0f, 0.0f };
	vector2D drag{ 0.0f, 0.0f };

	float matrix[4][4];
	const float thrustFactor = 0.0005f;
	const float dragCoefficient = -0.001f;

	// display text
	stringstream outText;
	IFont* myFont = myEngine->LoadFont("Comic Sans MS", 24);
	int health = 100;
	int position = 1;

	//Allow game to check if keys are pressed/held.
	bool isPaused = false; //P key, pause game.
	float speedController = 1.0f;   // define constant float for final submission

	gameStates currentState = playing;
	hoverCarStates hoverTracker = waiting;

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		// display game text
		switch (currentState)
		{
		case playing:
			outText << " Health: " << health;
			myFont->Draw(outText.str(), 20, 600);
			outText.str("");
			outText << " Position: " << position;
			myFont->Draw(outText.str(), 20, 650);
			outText.str("");
			break;
		case paused:
			outText << "Paused: ";
			myFont->Draw(outText.str(), 20, 20);
			outText.str("");
			break;
		case over:
			outText << "Game Over: ";
			myFont->Draw(outText.str(), 20, 20);
			outText.str("");
			break;
		}

		// store current position before we do anything
		float xPos = hoverCar->GetX();
		float zPos = hoverCar->GetZ();

		// get the facing vector
		hoverCar->GetMatrix(&matrix[0][0]);
		vector2D facingVector = { matrix[2][0], matrix[2][2]};

		float deltaTime = myEngine->Timer(); // returns difference in time


		/**** Update your scene each frame here ****/

		//check if player presses pause key, invert pause variable to make game pause.
		if (isPaused == false)
		{
			if (myEngine->KeyHit(Key_Right))
			{
				myCamera->MoveX(25);
			}
			else if (myEngine->KeyHit(kMoveForwardsKey))
			{
				speedController += kSpeedInc;
			}
			else if (myEngine->KeyHit(kMoveBackwardsKey))
			{
				speedController -= kSpeedInc;
			}

			// calculate thrust (based on keyboard input)

			if (myEngine->KeyHit(kMoveRightKey))
			{
				hoverCar->MoveLocalX(kHoverCarSpeed);
				hoverCar->RotateY(hoverCarRot);
			}
			else if (myEngine->KeyHit(kMoveLeftKey))
			{
				hoverCar->MoveLocalX(-kHoverCarSpeed);
				hoverCar->RotateY(-hoverCarRot);
			}
			else if (myEngine->KeyHeld(kMoveForwardsKey))
			{
				thrust = scalar(thrustFactor, facingVector);
				//hoverCar->MoveLocalZ(kHoverCarSpeed);
			}
			else if (myEngine->KeyHeld(kMoveBackwardsKey))
			{
				thrust = scalar(-thrustFactor, facingVector);
			}
			else
			{
				thrust = {0.0f, 0.0f};
			}

			// calculate drag (based on previous momentum)

			drag = scalar(dragCoefficient, momentum);

			// calculate momentum (based on thrust, drag and previous momentum)
			momentum = sum3(momentum, thrust, drag);

			// calculate new position
			float newXPos = xPos + momentum.x;
			float newZPos = zPos + momentum.z;

			// check for collision

			collision collisionResponse = Sphere2BoxCollision2D(xPos, zPos, newXPos, newZPos, carCollisionRad,
							  0.0 , 0.0, halfCubeDimension, halfCubeDimension);

			// collission resolution
			switch (collisionResponse)
			{
			case xAxis:
				momentum.x = -momentum.x / 2.0f;
				break;
			case zAxis:
				momentum.z = -momentum.z / 2.0f;
				break;
			case none:
				hoverCar->SetPosition(newXPos, 0.0f, newZPos);
			}
			
			// move the hover car (according to the new momentum)
			hoverCar->Move(momentum.x, 0.0f, momentum.z);

		} // end of if (isPaused == false)

		// check if P key is hit then pause/ unpause the game
		if (myEngine->KeyHit(kPauseGame)) 
		{     // Change to pauseKey for final submission
			if (isPaused == true) 
			{
				isPaused = false;
			}
			else 
			{
				isPaused = true;
			}
		}

		if (myEngine->KeyHit(kQuitGame))
		{
			myEngine->Stop();
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
