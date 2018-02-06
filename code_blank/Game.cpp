#include "Game.h"


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

	delete comp;
	delete cam;
	delete player;
	delete pControl;
}

// Intialization of Game components
int32_t Game::Init()
{
	uint32_t ret;

	// Create a camera
	cam = new Camera();

	// Create a Player Controller
	pControl = new PController();
	pControl->SetCamera(cam);

	comp = NULL;
	player = NULL;

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

// Main upate loop
int32_t Game::Update()
{
	uint32_t ret;

	pControl->Update();

	// should pause?
	if (pControl->GetPause())
	{
		//Temp for Testing
		Engine::instance()->Quit();

		 //TODO
		 //Pause the game and load the pasue menu
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
		cam->Update();
		pControl->UpdateP(Time::DeltaTime);

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
			/*{
				Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeSkybox);
				TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
				skyboxMat->SetTexture(tex);

				cam->SetSkybox(skyboxMat);
				cam->SetClearColor(math::float4{ 0.0f, 0.0f, 0.0f, 1.0f });
			}*/
			/*{
				Entity e = Entity::Create();

				tBox6 = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* cubeModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* cubeMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				cubeMat->SetTexture(diffuse);

				r->SetMaterial(cubeMat);
				r->SetModel(cubeModel);

				tBox6->SetLocalPosition(math::float3{ -2.0f, 1.0f, 0.0f });
				PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}*/
			/*{
				Entity e = Entity::Create();

				tBox7 = e.AddComponent<TransformComponent>();
				tBox7->SetLocalScale(math::float3{ 25, .5, 25 });
				//tBox7->SetLocalPosition(math::float3{ 0, 10, 10 });

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				tBox7->SetLocalPosition(math::float3{ 0.0f, 0.0f, 0.0f });

				PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
				ph->SetStatic(true);
				ph->SetBoxCollider(math::float3{ 25.0f, 0.5f, 25.0f });
				ph->SetColliderOrigin(math::float3{ 0.0f, -0.5f, 0.0f });
			}*/
			
			// Setup the Scene
			CHECKED(sceneMgr.Init());

			CHECKED(sceneMgr.LoadScene("assets/scenes/Tutorial.json"));

			// Setup the Player
			if (player == NULL)
			{
				player = new Player();
				pControl->SetPlayer(player);
			}
			assert(player != NULL);

			// Setup the Player's Companion
			if (comp == NULL)
			{
				comp = new Companion(player->GetPosition());
				pControl->SetCompanion(comp);
			}
			assert(player != NULL);

			//*********************************************************************************************
			//temp for test
			/*enemy01 = new Enemy(math::float3{ 10.0f, 1.0f, 0.0f });
			enemy02 = new Enemy(math::float3{ -10.0f, 1.0f, -10.0f });
			enemy03 = new Enemy(math::float3{ -10.0f, 1.0f, 10.0f });*/
			//*********************************************************************************************

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