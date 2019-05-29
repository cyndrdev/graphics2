#pragma once
#include "DirectXCore.h"

// === debug === //
const bool	SHOW_DEBUG_CONSOLE =			true;

// === camera === //
const float CAMERA_DISTANCE =				30.0f;
const float CAMERA_YOFFSET =				5.0f;

// === day night cycle === //
const float DAY_SCALE =						15.0f;

// === player === //
const float PLAYER_SCALE =					0.05f;
const float PLAYER_MODEL_SIZE =				200.0f;

const float PLAYER_SIZE =					PLAYER_SCALE 
											* PLAYER_MODEL_SIZE;

const int	PLAYER_JUMP_LENIENCE_FRAMES =	3;
const float PLAYER_JUMP_LENIENCE_Y =		0.2f;

const float PLAYER_JUMP_POWER =				1.2f;

// === terrain === //
const float GRID_MAGNITUDE =				1000.0f;
const float GRID_STEP =						10.0f;
const UINT	GRID_SIZE =						1024;

const int	GRID_NODES =					(GRID_SIZE - 1)
											* (GRID_SIZE - 1);
		
const int	VERTEX_TARGET =					4 * GRID_NODES;
const int	INDEX_TARGET =					6 * GRID_NODES;

// === window === //
const UINT	WINDOW_WIDTH =					1280;
const UINT	WINDOW_HEIGHT =					720;
const UINT	WINDOW_FRAMERATE =				60;

// === input === //
const UINT	MOVE_LEFT =			0x41;		// A
const UINT	MOVE_RIGHT =		0x44;		// D

const UINT	MOVE_FORWARD =		0x57;		// W
const UINT	MOVE_BACKWARD =		0x53;		// S

const UINT	MOVE_UP =						VK_SPACE;
const UINT	MOVE_DOWN =						VK_SHIFT;

const UINT	MOVE_SPEEDY =					VK_CONTROL;
	
const UINT	PLAYER_JUMP =					VK_SPACE;

const UINT	TOGGLE_CAMERA =		0x54;		// T

const float	MOUSE_SENSITIVITY =				0.08f;

// === physics === //
const float	FRAME_DELTA =					1.0f 
											/ (float)WINDOW_FRAMERATE;

const float	GRAVITY_SCALE =					0.5f;

const float	FRAME_GRAVITY =					-9.81f 
											* FRAME_DELTA
											* GRAVITY_SCALE;

const float	SPEED_NORMAL =					0.5f;
const float	SPEED_SPEEDY =					1.5f;

// === file paths === //

const std::wstring	HEIGHTMAP =				L"data\\heightmap.raw";

const std::wstring	TERRAIN_SHADER =		L"shader\\multiTexture.hlsl";
const std::wstring	SKYBOX_SHADER =			L"shader\\SkyShader.hlsl";
const std::wstring	MESH_SHADER =			L"shader\\TexturedShaders.hlsl";

const std::wstring	PALM_MODEL =			L"model\\palm4\\scene.gltf";
const std::wstring	DOG_MODEL =				L"model\\dog\\scene.gltf";
const std::wstring	FOX_MODEL =				L"model\\fox\\scene.gltf";

const std::wstring	MUSIC_PATH =			L"audio\\floral_shoppe.wav";

const std::wstring	SKYBOX =				L"texture\\test.dds";

const std::wstring	TERRAIN_TEXTURES[5] = {
											L"texture\\grass.dds",
											L"texture\\darkdirt.dds",
											L"texture\\stone.dds",
											L"texture\\lightdirt.dds",
											L"texture\\snow.dds"
};