#include<iostream>
#define OLC_PGE_APPLICATION
#include"olcPixelGameEngine.h"

using namespace std;

class SpriteWorks : public olc::PixelGameEngine {
private:
	olc::Sprite* mySprite;
public:

	SpriteWorks() {
		sAppName = "SpriteWorks";
	}

	bool OnUserCreate() override {
		mySprite = new olc::Sprite("car_top1.png");
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override {
		Clear(olc::DARK_CYAN);
		DrawSprite(20, 20, mySprite);
		return true;
	}
};

int main() {
	SpriteWorks demo;
	if (demo.Construct(256, 240, 4, 4)) demo.Start();
	return 0;
}