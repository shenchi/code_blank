#include "Game.h"
#include "PhysicsSystem.h"

using namespace tofu;

typedef const rapidjson::Value& value_t;

InputSystem* input;

Game::~Game()
{
	//*********************************************************************************************
	//temp for test
	/*delete enemy01;
	delete enemy02;
	delete enemy03;*/
	//*********************************************************************************************

	if (enemyList != nullptr)
	{
		delete enemyList;
	}
	delete comp;
	delete cam;
	delete player;
	delete pControl;
	delete DebugPlayer;
}

// Intialization of Game components
int32_t Game::Init()
{
	uint32_t ret;	

	debugMode = false;

	if (!debugMode)
	{
		// Create a camera
		cam = new Camera();

		// Create a Player Controller
		pControl = new PController();
		pControl->SetCamera(cam);

		comp = NULL;
		player = NULL;
		enemyList = nullptr;
	}

	// Load the initial scene (Defalut is Intro)
	// Load other scenes here for fast testing
	currentScene = level;

	// Set up scene
	ret = LoadScene(currentScene);
	assert(ret == kOK);

	//*********************************************************************************************
	//temp for test
	timePassed = 0.0f;
	loopStart = true;
	//*********************************************************************************************

	return kOK;
}

// Exit the Game
int32_t Game::Shutdown()
{
	return kOK;
}

int32_t Game::FixedUpdate()
{
	uint32_t ret;
	switch (currentScene)
	{
		case 5:	// Tutorial
		{
			break;
		}
		case 6:	// Level
		{
			if (debugMode)
			{
				//DebugPlayer->Update();
			}
			else
			{
				comp->FixedUpdate(Time::DeltaTime, player->GetPosition(), player->GetForward());
				cam->FixedUpdate(Time::FixedDeltaTime);
				pControl->FixedUpdate(Time::FixedDeltaTime);
			}
			break;
		}
	}

	return kOK;
}


// Main upate loop
int32_t Game::Update()
{
	uint32_t ret;

	if (!debugMode)
	{
		pControl->Update();

		// should pause?
		if (pControl->GetPause())
		{
			//Temp for Testing
			Engine::instance()->Quit();

			//TODO
			//Pause the game and load the pasue menu
		}
	}

	// Switch for game state
	// Only handles fully loaded Scenes
	// Will be overhauled to use JSON file names and possibley multi-threading
	switch (currentScene)
	{
	case 0:	// Intro
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = menu;
			ret = LoadScene(menu);
			assert(ret == kOK);

			// Unload last scene
			lastScene = intro;
			ret = UnloadScene(intro);
			assert(ret == kOK);
		}
		break;
	}
	case 1:	// Menu
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = loading;
			ret = LoadScene(loading);
			assert(ret == kOK);

			// Unload last scene
			lastScene = menu;
			ret = UnloadScene(menu);
			assert(ret == kOK);
		}
		break;
	}
	case 4:	// Loading
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = tutorial;
			ret = LoadScene(tutorial);
			assert(ret == kOK);

			// Unload last scene
			lastScene = loading;
			ret = UnloadScene(loading);
			assert(ret == kOK);
		}
		break;
	}
	case 5:	// Tutorial
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = level;
			ret = LoadScene(level);
			assert(ret == kOK);

			// Unload last scene
			lastScene = tutorial;
			ret = UnloadScene(tutorial);
			assert(ret == kOK);
		}
		break;
	}
	case 6:	// Level
	{
		if (debugMode)
		{
			DebugPlayer->Update();
		}
		else
		{
			//if (input->IsButtonDown(ButtonId::kKeyEnter))
			//{
			//	// Load next scene
			//	currentScene = credits;
			//	ret = LoadScene(credits);
			//	assert(ret);

			//	// Unload last scene
			//	lastScene = level;
			//	ret = UnloadScene(level);
			//	assert(ret);
			//}

			// May also need to run player update here
			comp->Update(Time::DeltaTime, player->GetPosition(), player->GetForward());
			cam->Update(Time::DeltaTime);
			pControl->UpdateP(Time::DeltaTime);
			player->Update(Time::DeltaTime);

			//*******************************************************************************
			// Temp Enemy control code
			if (loopStart)
			{
				startTime = Time::TotalTime;
				loopStart = false;
			}

			timePassed = Time::TotalTime - startTime;

			//if (timePassed > 0 && timePassed <= 10)	// Move Forward
			//{
			//	enemy01->Move(Time::DeltaTime, false, math::float3{0, 0, 1});
			//}
			//else if (timePassed > 10 && timePassed <= 20) // Move Left
			//{
			//	enemy01->Move(Time::DeltaTime, false, math::float3{-1, 0, 0 });
			//}
			//else if (timePassed > 20 && timePassed <= 30) // Move Backward
			//{
			//	enemy01->Move(Time::DeltaTime, false, math::float3{0, 0, -1});
			//}
			//else if (timePassed > 30 && timePassed <= 40) // Move Right
			//{
			//	enemy01->Move(Time::DeltaTime, false, math::float3{1, 0, 0});
			//}

			if (timePassed > 40)
			{
				timePassed = 0.0f;
				loopStart = true;
			}
		}
		assert(!(timePassed < 0) && timePassed < 50);
		//*******************************************************************************

		break;
	}
	case 9: // End of Level
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = levelEnd;
			ret = LoadScene(credits);
			assert(ret == kOK);

			// Unload last scene
			lastScene = levelEnd;
			ret = UnloadScene(levelEnd);
			assert(ret == kOK);
		}
		break;
	}
	case 10: // Credits
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = menu;
			ret = LoadScene(menu);
			assert(ret == kOK);

			// Unload last scene
			lastScene = credits;
			ret = UnloadScene(credits);
			assert(ret == kOK);
		}
		break;
	}
	default:
		assert(true);
		//return 1; // Not OK
		break;
	}


	return kOK;
}// End of Update Loop

// Load a scene
// Will load from JSON files later
bool Game::LoadScene(sceneType num)
{
	// Temp code for testing
	switch (num)
	{
		case 0:
		{
			// TODO
			// Intro Scene
			// Fully Loaded
			break;
		}
		case 1:
		{
			// TODO
			// Menu Scene
			// Fully Loaded
			break;
		}
		case 2:
		{
			// TODO
			// Options Scene
			// Loaded On Top
			break;
		}
		case 3:
		{
			// TODO
			// Help Scene
			// Loaded On Top
			break;
		}
		case 4:
		{
			// TODO
			// Loading Scene
			// Fully Loaded
			break;
		}
		case 5:
		{
			// TODO
			// Tutorial Scene
			// Fully Loaded
			break;
		}
		case 6:
		{
			// Setup the Scene
			CHECKED(sceneMgr.Init());

			//CHECKED(sceneMgr.LoadScene("assets/scenes/Node_Export.json"));
			//CHECKED(sceneMgr.LoadScene("assets/scenes/Engine_Tut.json"));
			CHECKED(sceneMgr.LoadScene("assets/scenes/OptiTest.json"));

			int pathLength = sceneMgr.GetPathNodesLength();
			if (pathLength > 0)
			{
				pathNodes = new std::vector<PathNode*>();
				pathNodes = sceneMgr.GetPathNodes();
				assert(pathNodes != NULL);

				// loop through nodes and link up references to nearby nodes
				// TODO this could be slow and need to be redone later or elsewhere
				for (uint32_t i = 0; i < pathNodes->size(); i++)
				{
					std::string name = pathNodes->at(i)->name;
					for (uint32_t j = 0; j < pathNodes->size(); j++)
					{
						if (pathNodes->at(j)->nearby_1->name == name)
						{
							pathNodes->at(j)->nearby_1 = pathNodes->at(i);
						}
						else if (pathNodes->at(j)->nearby_2->name == name)
						{
							pathNodes->at(j)->nearby_2 = pathNodes->at(i);
						}
						else if (pathNodes->at(j)->nearby_3->name == name)
						{
							pathNodes->at(j)->nearby_3 = pathNodes->at(i);
						}
						else if (pathNodes->at(j)->nearby_4->name == name)
						{
							pathNodes->at(j)->nearby_4 = pathNodes->at(i);
						}
					}
				}
			}// end if path length > 0

			if (debugMode)
			{
				DebugPlayer = new Utility::GhostPlayer(tofu::math::float3{ -80.0f, 13.0f, 887.0f });
			}
			else
			{
				CharacterDetails playerDetails = {};
				playerDetails.modelName = "assets/archer.model";
				playerDetails.diffuseName = "assets/archer_0.texture";
				playerDetails.normalMapName = "assets/archer_1.texture";
				playerDetails.capsuleColliderSize = { 50.0f, 80.0f };
				playerDetails.colliderOrigin = { 0.0f, 100.0f, 0.0f };
				playerDetails.health = 200.0f;
				playerDetails.jumpPower = 2.5f;
				playerDetails.position = sceneMgr.GetPlayerSpawnPoint();
				playerDetails.scale = { 0.01f, 0.01f, 0.01f };
				playerDetails.sprintSpeed = 10.0f;
				playerDetails.tag = "player";
				playerDetails.walkSpeed = 5.0f;
				playerDetails.rollDodgeCost = 10.0f;
				playerDetails.acceleration = 6.67f;
				playerDetails.deacceleration = 10.0f;


				// Setup the Player's Companion
				if (comp == NULL)
				{
					comp = new Companion(playerDetails.position);
					pControl->SetCompanion(comp);
				}

				// Setup the Player
				if (player == NULL)
				{
					player = new Player(playerDetails, comp);
					pControl->SetPlayer(player);
				}
				assert(player != NULL);

				cam->SetPosition(playerDetails.position);
				cam->SetTarget(playerDetails.position);				
				
				//*********************************************************************************************
				//temp for test
				/*enemy01 = new Enemy(math::float3{ 10.0f, 1.0f, 0.0f });
				enemy02 = new Enemy(math::float3{ -10.0f, 1.0f, -10.0f });
				enemy03 = new Enemy(math::float3{ -10.0f, 1.0f, 10.0f });*/
				//*********************************************************************************************

				// Add each enemy to the enemy list
				// enemyList = new std::vector<Character*>();
				// enemyList.pushback(enemy01);

				player->GetCombatManager()->SetEnemyList(enemyList);
			}
			break;
		}
		case 7:
		{
			// TODO
			// Pause Scene
			// Loaded On Top
			break;
		}
		case 8:
		{
			// TODO
			// Death Scene
			// Fully Loaded
			break;
		}
		case 9:
		{
			// TODO
			// Credits Scene
			// Fully Loaded
			break;
		}
		case 10:
		{
			// TODO
			// Credits Scene
			// Fully Loaded
			break;
		}

		default:
			return 1; // Not OK
			break;
	}// End of Switch

	//return kOK;
	return kOK;
}

// Unload a scene
bool Game::UnloadScene(sceneType num)
{
	// Temp code until JSON is implemented
	switch (num)
	{
	case 0:
	{
		//tIntro.Destroy();
		break;
	}
	case 1:
	{
		//tCube.Destroy();
		//tBox.Destroy();
		break;
	}
	case 4:
	{
		//tBox2.Destroy();
		//tBox3.Destroy();
		break;
	}
	case 5:
	{
		//tBox4.Destroy();
		//tBox5.Destroy();
		break;
	}
	case 6:
	{
		//tBox6.Destroy();
		//tBox7.Destroy();
		break;
	}
	case 10:
	{
		//tBox2.Destroy();
		//tBox3.Destroy();
		break;
	}
	default:
		return 1; // Not OK
		break;
	}

	return kOK;
}

// Load a scene on top of another
bool Game::LoadOnTop(sceneType num)
{
	// Temp code until JSON is implemented
		switch (num)
		{
		case 2:
		{
			// TODO
			// Top Load Options Scene
			break;
		}
		case 3:
		{
			// TODO
			// Top Load Help Scene
			break;
		}
		case 7:
		{
			// TODO
			// Top Load Pause Scene
			break;
		}
		case 8:
		{
			// TODO
			// Top Load Death Scene
			break;
		}
		default:
			return 1; // Not OK
			break;
		}

	return kOK;
}

// Unload a top loaded scene
bool Game::UnloadOffTop(sceneType num)
{
	// Temp code until JSON is implemented
	switch (num)
	{
	case 2:
	{
		// TODO
		//  Unload Options Scene
		break;
	}
	case 3:
	{
		// TODO
		// Unload Help Scene
		break;
	}
	case 7:
	{
		// TODO
		// Unload Pause Scene
		break;
	}
	case 8:
	{
		// TODO
		// Unload Death Scene
		break;
	}
	default:
		return 1; // Not OK
		break;
	}

	return kOK;
}