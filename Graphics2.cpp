#include "Graphics2.h"
#include "CubeRendererNode.h"
#include "TerrainNode.h"
#include "MeshNode.h"
#include "PlayerNode.h"
#include "AudioNode.h"
#include "SkyboxNode.h"
#include "GameConstants.h"
#include <cmath>
#include <iostream>

Graphics2 app;

const float PALM_SCALE = 5.0f;
const int PALM_COUNT = 20;

const float FOX_SCALE = 0.05f;

Graphics2::Graphics2() :
	DirectXFramework(WINDOW_WIDTH, WINDOW_HEIGHT)
{
}

void Graphics2::CreateSceneGraph()
{
	// show a debug console
	if (SHOW_DEBUG_CONSOLE)
		InitDebugConsole();

	std::cout << ":: === welcome to paradise === ::" << std::endl;
	std::cout << "initialising scene graph...\t";

	// set our lighting state
	XMVECTOR lightDirection = XMVector4Normalize(XMVectorSet(0.0f, -1.0f, 1.0f, 0.0f));
	XMStoreFloat4(&GetLighting()->DirectionalLightVector, lightDirection);

	GetLighting()->DirectionalLightColor	= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	GetLighting()->AmbientLight				= XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	// initialise our camera
	GetCamera()->SetCameraPosition(0.0f, 300.0f, -1000.0f);

	// create our scene graph
	SceneGraphPointer sceneGraph = GetSceneGraph();

	// add the terrain
	terrain_ = std::make_shared<TerrainNode>(L"terrainboi");
	sceneGraph->Add(terrain_);
	GetCamera()->SetTerrain(terrain_);

	// add a bunch o palms
	SceneNodePointer palm;
	for (int i = 0; i < PALM_COUNT; i++) {
		palm = std::make_shared<MeshNode>(L"palm_" + std::to_wstring(i), PALM_MODEL);

		palm->GetTransform()->Rotate(XM_PIDIV2, XM_2PI * ((float)rand() / RAND_MAX), 0.0f);
		palm->GetTransform()->SetScale(PALM_SCALE);

		palm->CreateCollider(100.0f, 2.0f, XMFLOAT3(0, 0, 0), false);

		sceneGraph->Add(palm);
	}

	// add a dog
	SceneNodePointer dog = std::make_shared<MeshNode>(L"dog", DOG_MODEL);
	sceneGraph->Add(dog);
	dog->GetTransform()->SetPosition(-1350.0f, -500.0f, 450.0f);
	dog->GetTransform()->SetRotation(XM_PIDIV2, XM_PI, 0);
	dog->GetTransform()->SetScale(1.0f);
	dog->CreateCollider(0.0f, 6.0f, XMFLOAT3(0, 0, 0), true);

	// add a skybox
	SceneNodePointer skybox = std::make_shared<SkyboxNode>(L"skybox", SKYBOX, 1000.0f, 30);
	sceneGraph->Add(skybox);

	// create our player
	player_ = std::make_shared<PlayerNode>(L"fox", FOX_MODEL);
	player_->GetTransform()->SetScale(PLAYER_SCALE);
	player_->SetTerrain(terrain_);
	player_->CreateCollider(0.0f, 3.0f, XMFLOAT3(0, 0, 0), true);
	sceneGraph->Add(player_);
	GetCamera()->FollowNode(player_, CAMERA_DISTANCE, CAMERA_YOFFSET);

	// add some tunes
	SceneNodePointer music = std::make_shared<AudioNode>(L"ambiance", MUSIC_PATH);
	sceneGraph->Add(music);

	std::cout << "done." << std::endl;
}

void Graphics2::UpdateSceneGraph()
{
	// === reference getting === //
	SceneGraphPointer sceneGraph = GetSceneGraph();

	SceneNodePointer fox;
	fox = sceneGraph->Find(L"fox");

	SceneNodePointer palm;

	// set our light direction
	float t = a_ * (XM_PI / DAY_SCALE);
	if (fmodf(t, XM_2PI) < XM_PI) {
		GetLighting()->AmbientLight = XMFLOAT4(0.2f, 0.15f, 0.15f, 1.0f);
		GetLighting()->DirectionalLightColor = XMFLOAT4(0.8f, 0.7f, 0.7f, 1.0f);
	}
	else {
		t -= XM_PI;
		GetLighting()->AmbientLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 2.0f);
		GetLighting()->DirectionalLightColor = XMFLOAT4(0.3f, 0.3f, 0.4f, 1.0f);
	}
	XMVECTOR lightDirection = XMVector4Normalize(XMVectorSet(0.0f, -sin(t), -cos(t), 0.0f));
	XMStoreFloat4(&GetLighting()->DirectionalLightVector, lightDirection);


	// === check our collisions === //
	if (!firstFrame_) {
		std::shared_ptr<Collider> currentCollider;
		std::shared_ptr<Collider> otherCollider;

		CollisionInfo info;

		for (size_t i = 0; i < sceneGraph->GetChildCount(); i++) {
			SceneNodePointer current = sceneGraph->GetChild(i);
			currentCollider = current->GetCollider();

			// don't collide with uncollidable objects
			if (currentCollider == nullptr) continue;

			for (size_t j = i + 1; j < sceneGraph->GetChildCount(); j++) {
				SceneNodePointer other = sceneGraph->GetChild(j);
				otherCollider = other->GetCollider();

				// don't collide with uncollidable objects
				if (otherCollider == nullptr) continue;

				bool collided = (currentCollider->IsIntersecting(*otherCollider, info));

				if (collided) {
					std::wcout << current->GetName() << L" collided with " << other->GetName() << L"!" << std::endl;

					// if both are pushable, push each half as far
					float offsetScale = (currentCollider->IsPushable() && otherCollider->IsPushable())
						? 0.5f
						: 1.0f;

					if (currentCollider->IsPushable()) {
						current->GetTransform()->Translate(XMFLOAT3(
							info.offset.x * offsetScale,
							info.offset.y * offsetScale,
							info.offset.z * offsetScale
						));
					}

					if (otherCollider->IsPushable()) {
						other->GetTransform()->Translate(XMFLOAT3(
							info.offset.x * -offsetScale,
							info.offset.y * -offsetScale,
							info.offset.z * -offsetScale
						));
					}
				}
			}
		}
	}

	// === keep the good boy on the ground === //
	SceneNodePointer goodBoy = sceneGraph->Find(L"dog");
	XMFLOAT3 position;
	XMStoreFloat3(&position, goodBoy->GetTransform()->GetPosition());
	goodBoy->GetTransform()->SetPosition(
		position.x,
		terrain_->GetHeightAtPoint(
			position.x,
			position.z
		) + 5.0f,
		position.z
	);

	// === handle input === //

	a_ += FRAME_DELTA;
	bool foxSelected = GetCamera()->IsFollowingNode();

	float lr =
		GetInput()->IsKeyPressed(MOVE_LEFT)		? -1.0f :
		GetInput()->IsKeyPressed(MOVE_RIGHT)	? +1.0f :
													0.0f;
	float fb =
		GetInput()->IsKeyPressed(MOVE_FORWARD)	? +1.0f :
		GetInput()->IsKeyPressed(MOVE_BACKWARD)	? -1.0f :
													0.0f;

	// normalise our velocity
	float sum = sqrtf((lr * lr) + (fb * fb));
	if (sum > 1.0f) {
		lr /= sum;
		fb /= sum;
	}

	float ud =
		GetInput()->IsKeyPressed(MOVE_DOWN)		? -1.0f :
		GetInput()->IsKeyPressed(MOVE_UP)		? +1.0f :
													0.0f;

	float mod =
		GetInput()->IsKeyPressed(MOVE_SPEEDY) ? SPEED_SPEEDY : SPEED_NORMAL;

	bool swapCam = GetInput()->IsKeyDown(TOGGLE_CAMERA);

	if (swapCam) {
		// toggle our selection
		foxSelected ^= true;

		std::cout << "swapping cam!" << std::endl;

		GetCamera()->FollowNode(
			GetCamera()->IsFollowingNode()
			? nullptr
			: fox,
			CAMERA_DISTANCE,
			CAMERA_YOFFSET
		);
	}

	// === player state updates === //
	bool foxJump = GetInput()->IsKeyDown(PLAYER_JUMP);

	PlayerControlState state{
		lr,
		fb,
		mod,
		foxJump
	};

	player_->SetActive(foxSelected);
	player_->SetControlState(state);

	// === camera movement === //
	if (!foxSelected) {
		GetCamera()->SetLeftRight(lr * mod);
		GetCamera()->SetForwardBack(fb * mod);
		GetCamera()->SetRelativeY(ud * mod);
	}

	XMFLOAT2 mouseInput = GetInput()->GetMouseDelta();
	GetCamera()->SetYaw(-mouseInput.x * MOUSE_SENSITIVITY);
	GetCamera()->SetPitch(-mouseInput.y * MOUSE_SENSITIVITY);

	// === spinny palms === //
	for (int i = 0; i < PALM_COUNT; i++) {
		palm = sceneGraph->Find(L"palm_" + std::to_wstring(i));
		palm->GetTransform()->Rotate(0.0f, 0.015f, 0.0f);

		// set their positions randomly. we have to do this in
		// update, because in CreateSceneGraph() the terrain
		// hasn't been generated yet.
		if (firstFrame_) {
			XMFLOAT3 position = {
				(((float)rand() / (RAND_MAX + 1)) - 0.5f) * (GRID_SIZE * GRID_STEP),
				0,
				(((float)rand() / (RAND_MAX + 1)) - 0.5f) * (GRID_SIZE * GRID_STEP)
			};
			position.y = terrain_->GetHeightAtPoint(position.x, position.z) - 4.0f;

			palm->GetTransform()->SetPosition(position);
		}
	}

	firstFrame_ = false;
}
