// Game_3_Desert_Racer .cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <iostream>
using namespace tle;
using namespace std;
#include <iostream>

const int kCheckpointAmount = 3;
const int kWallAmount = 4;
const int kIsleAmount = 6;

// sky position
const float skyYPos = -960.0f;

// hover positions
const float hoverXPos = 0.5f;
const float hoverYPos = 0.5f;
const float hoverZPos = -30.0f;






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
	IModel* hover = hoverMesh->CreateModel( hoverXPos, hoverYPos, hoverZPos );

	// create checkpoints
	IMesh* checkpointMesh = myEngine->LoadMesh("checkpoint.x");
	
	IModel* checkpoint[kCheckpointAmount];
	float checkpointXs[kCheckpointAmount] = {0, 10, 25};
	float checkpointYs[kCheckpointAmount] = { 0, 0, 0 };
	float checkpointZs[kCheckpointAmount] = { 0, 120, 56 };
	for (int i = 0; i < kCheckpointAmount; i++)
	{
		checkpoint[i] = checkpointMesh->CreateModel(checkpointXs[i], checkpointYs[i], checkpointZs[i]);
		
	}
	checkpoint[1]->RotateY(90.0f);

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
	myCamera->AttachToParent(hover);

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/

		if (myEngine->KeyHeld(Key_W))
		{
			hover->MoveLocalZ(0.05);
		}
		else if (myEngine->KeyHeld(Key_S))
		{
			hover->MoveLocalZ(-0.05);
		}
		else if (myEngine->KeyHit(Key_D))
		{
			hover->MoveLocalX(0.01);
			hover->RotateLocalY(10);
		}
		else if (myEngine->KeyHit(Key_A))
		{
			hover->MoveLocalX(-0.01);
			hover->RotateLocalY(-10.0);
		}

		if (myEngine->KeyHit(Key_Escape))
		{
			myEngine->Stop();
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}