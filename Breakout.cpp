#include<iostream>
#include<string>
#define OLC_PGE_APPLICATION
#include"olcPixelGameEngine.h"

using namespace std;


class BreakOut : public olc::PixelGameEngine {
private:
	string map;
	int screenWidth = 16;
	int screenHeight = 15;
	int blockWidth = 8;
	float bat = 8;
	float batWidth = 2;
	float ang = .6;
	float ballx = 8*blockWidth;
	float bally = 7*blockWidth;
	float ballDx = cosf(ang);
	float ballDy = sinf(ang);
	float ballSpeed = 70;
	float oldballx;
	float oldbally;
public:
	bool OnUserCreate() override {
		map += "################";
		map += "#..............#";
		map += "#..1111111111..#";
		map += "#....1111......#";
		map += "#..1...........#";
		map += "#.....1....11..#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		map += "#..............#";
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override {


		//DRAW
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

		

		if (GetKey(olc::Key::LEFT).bHeld) bat = bat - (15 * fElapsedTime);
		if (GetKey(olc::Key::RIGHT).bHeld) bat = bat + (15 * fElapsedTime);

		oldballx = ballx;
		oldbally = bally;

		ballx += ballDx * fElapsedTime * ballSpeed;
		bally += ballDy * fElapsedTime * ballSpeed;

		int cellNewX = (int)ballx / blockWidth;
		int cellNewY = (int)bally / blockWidth;

		int cellOldX = (int)oldballx / blockWidth;
		int cellOldY = (int)oldbally / blockWidth;

		char newCell = map[(cellNewY * screenWidth) + cellNewX];
		char oldCell = map[(cellOldY * screenWidth) + cellOldX];

		if (newCell != '.') {
			if (newCell == '1')
				map[(cellNewY * screenWidth) + cellNewX] = '.';
			if (cellNewX != cellOldX) ballDx = ballDx * (-1);
			if (cellNewY != cellOldY) ballDy = ballDy * (-1);
		}

		if (bally > ((screenHeight)* (blockWidth) - 2)) {
			if (ballx > (bat - batWidth)*blockWidth && ballx < (bat + batWidth)*blockWidth) ballDy *= -1;
			else {
				float ballx = 8 * blockWidth;
				float bally = 7 * blockWidth;
			}
		}

		for (int i = 0; i < screenHeight; i++) {
			for (int j = 0; j < screenWidth;j++) {
				switch (map[(i * screenWidth) + j]) {
				case '#':
					FillRect(j*blockWidth, i*blockWidth, (j+1) * blockWidth, (i+1) * blockWidth, olc::WHITE);
					break;
				case '.':
					FillRect(j * blockWidth, i * blockWidth, (j + 1) * blockWidth, (i + 1) * blockWidth, olc::BLACK);
					break;
				case '1':
					FillRect(j * blockWidth, i * blockWidth, (j + 1) * blockWidth, (i + 1) * blockWidth, olc::BLUE);
					break;
				}
			}
		}

		FillCircle(ballx , bally , 2, olc::GREEN);

		DrawLine((bat-batWidth) * blockWidth, screenHeight * blockWidth -2 , (bat+batWidth) * blockWidth, screenHeight * blockWidth -2, olc::WHITE);
		return true;
	}
};

int main() {
	BreakOut game;
	if (game.Construct(128, 120, 4, 4))
		game.Start();
	return 0;
}