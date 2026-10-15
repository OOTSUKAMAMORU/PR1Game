#pragma once
#include "KamataEngine.h"
#include "WhiteCircle.h"

class WhiteCircle {
public:
	void Initialize();
	void Update();
	void Draw();

private:
	KamataEngine::Sprite* sprite_ = nullptr;

	KamataEngine::Vector2 position_ = {640.0f, 360.0f};

	WhiteCircle* whiteCircle_ = nullptr;

};