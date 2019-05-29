#pragma once

#include "SceneNode.h"
#include <windows.h>

#pragma comment(lib, "winmm.lib")

class AudioNode
	: public virtual SceneNode
{
public:
	AudioNode(std::wstring name, std::wstring soundFile) :
		SceneNode(name),
		soundFile_(soundFile)
	{}

	bool Initialise(void) { return true; };
	void Start(void) { PlaySound(soundFile_.c_str(), NULL, SND_LOOP | SND_ASYNC); };
	void Render(void) {};
	void Shutdown(void) {};
private:
	std::wstring soundFile_;
};