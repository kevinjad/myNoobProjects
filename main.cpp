#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include"escapi.h"
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;


class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Example";
	}

	union RGBint
	{
		int rgb;
		unsigned char c[4];
	};

	int nCameras = 0;
	SimpleCapParams capture;

	float* dispCameraR = nullptr;
	float* dispCameraG = nullptr;
	float* dispCameraB = nullptr;
	float* dispCameraAlpha = nullptr;
	float* fOldCamera = nullptr;		// Previous raw frame from camera
	float* fNewCamera = nullptr;		// Recent raw frame from camera
	float* fFilteredCamera = nullptr;	// low-pass filtered image
	float* fOldFilteredCamera = nullptr;	// low-pass filtered image
	float* fOldMotionImage = nullptr;	// previous motion image
	float* fMotionImage = nullptr;		// recent motion image
	float* fFlowX = nullptr;			// x-component of flow field vector
	float* fFlowY = nullptr;			// y-component of flow field vector

	// Object Physics Variables
	float fBallX = 0.0f;				// Ball position 2D
	float fBallY = 0.0f;
	float fBallVX = 0.0f;				// Ball Velocity 2D
	float fBallVY = 0.0f;

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		nCameras = setupESCAPI();
		if (nCameras == 0)	return false;
		capture.mWidth = ScreenWidth();
		capture.mHeight = ScreenHeight();
		capture.mTargetBuf = new int[ScreenWidth() * ScreenHeight()];
		if (initCapture(0, &capture) == 0)	return false;

		dispCameraR = new float[ScreenHeight() * ScreenWidth()];
		dispCameraG = new float[ScreenHeight() * ScreenWidth()];
		dispCameraB = new float[ScreenHeight() * ScreenWidth()];
		dispCameraAlpha = new float[ScreenHeight() * ScreenWidth()];
		fOldCamera = new float[ScreenWidth() * ScreenHeight()];
		fNewCamera = new float[ScreenWidth() * ScreenHeight()];
		fFilteredCamera = new float[ScreenWidth() * ScreenHeight()];
		fOldFilteredCamera = new float[ScreenWidth() * ScreenHeight()];
		fFlowX = new float[ScreenWidth() * ScreenHeight()];
		fFlowY = new float[ScreenWidth() * ScreenHeight()];
		fOldMotionImage = new float[ScreenWidth() * ScreenHeight()];
		fMotionImage = new float[ScreenWidth() * ScreenHeight()];

		memset(dispCameraR, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(dispCameraG, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(dispCameraB, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(dispCameraAlpha, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fOldCamera, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fNewCamera, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fFilteredCamera, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fOldFilteredCamera, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fFlowX, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fFlowY, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fOldMotionImage, 0, sizeof(float) * ScreenWidth() * ScreenHeight());
		memset(fMotionImage, 0, sizeof(float) * ScreenWidth() * ScreenHeight());

		fBallX = ScreenWidth() / 2.0f;
		fBallY = ScreenHeight() / 2.0f;

		return true;
	}

	void DrawFrame(float* R,float* G,float* B,float* Alpha) {
		for (int i = 0; i < ScreenWidth(); i++) {
			for (int j = 0; j < ScreenHeight(); j++) {
				int temp = 0.0f < R[j * ScreenWidth() + i] * 255.0f ? R[j * ScreenWidth() + i] * 255.0f : 0.0f;
				int r = temp < 255.0f ? temp : 255;
				temp = 0.0f < G[j * ScreenWidth() + i] * 255.0f ? G[j * ScreenWidth() + i] * 255.0f : 0.0f;
				int g = temp < 255.0f ? temp : 255;
				temp = 0.0f < B[j * ScreenWidth() + i] * 255.0f ? B[j * ScreenWidth() + i] * 255.0f : 0.0f;
				int b = temp < 255.0f ? temp : 255;
				temp = 0.0f < Alpha[j * ScreenWidth() + i] * 255.0f ? Alpha[j * ScreenWidth() + i] * 255.0f : 0.0f;
				int alpha = temp < 255.0f ? temp : 255;
				Draw(i, j, olc::Pixel(g, g, g));
			}
		}
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
		auto get_pixel = [&](float* image, int x, int y)
		{
			if (x >= 0 && x < ScreenWidth() && y >= 0 && y < ScreenHeight())
				return image[y * ScreenWidth() + x];
			else
				return 0.0f;
		};
		doCapture(0); while (isCaptureDone(0) == 0) {}

		for (int y = 0; y < capture.mHeight; y++)
			for (int x = 0; x < capture.mWidth; x++)
			{
				RGBint col;
				int id = y * capture.mWidth + x;
				col.rgb = capture.mTargetBuf[id];
				int r = col.c[2], g = col.c[1], b = col.c[0];
				float fR = (float)r / 255.0f;
				float fG = (float)g / 255.0f;
				float fB = (float)b / 255.0f;
				dispCameraR[y * ScreenWidth() + x] = (float)col.c[0] / 255.0f;
				dispCameraG[y * ScreenWidth() + x] = (float)col.c[1] / 255.0f;
				dispCameraB[y * ScreenWidth() + x] = (float)col.c[2] / 255.0f;
				dispCameraAlpha[y * ScreenWidth() + x] = (float)col.c[3] / 255.0f;
				// Store previous camera frame for temporal processing
				fOldCamera[y * ScreenWidth() + x] = fNewCamera[y * ScreenWidth() + x];

				// Store previous camera frame for temporal processing
				fOldFilteredCamera[y * ScreenWidth() + x] = fFilteredCamera[y * ScreenWidth() + x];

				// Store previous motion only frame
				fOldMotionImage[y * ScreenWidth() + x] = fMotionImage[y * ScreenWidth() + x];

				// Calculate luminance (greyscale equivalent) of pixel
				float fLuminance = 0.2987f * fR + 0.5870f * fG + 0.1140f * fB;
				fNewCamera[y * ScreenWidth() + x] = fLuminance;

				// Low-Pass filter camera image, to remove pixel jitter
				fFilteredCamera[y * ScreenWidth() + x] += (fNewCamera[y * ScreenWidth() + x] - fFilteredCamera[y * ScreenWidth() + x]) * 0.8f;

				// Create motion image as difference between two successive camera frames
				float fDiff = fabs(get_pixel(fFilteredCamera, x, y) - get_pixel(fOldFilteredCamera, x, y));

				// Threshold motion image to remove filter out camera noise
				fMotionImage[y * ScreenWidth() + x] = (fDiff >= 0.05f) ? fDiff : 0.0f;
			}

		int nPatchSize = 9;
		int nSearchSize = 7;

		for (int x = 0; x < ScreenWidth(); x++)
		{
			for (int y = 0; y < ScreenHeight(); y++)
			{
				// Initialise serach variables
				float fPatchDifferenceMax = INFINITY;
				float fPatchDifferenceX = 0.0f;
				float fPatchDifferenceY = 0.0f;
				fFlowX[y * ScreenWidth() + x] = 0.0f;
				fFlowY[y * ScreenWidth() + x] = 0.0f;

				// Search over a given rectangular area for a "patch" of old image
				// that "resembles" a patch of the new image.
				for (int sx = 0; sx < nSearchSize; sx++)
				{
					for (int sy = 0; sy < nSearchSize; sy++)
					{
						// Search vector is centre of patch test
						int nSearchVectorX = x + (sx - nSearchSize / 2);
						int nSearchVectorY = y + (sy - nSearchSize / 2);

						float fAccumulatedDifference = 0.0f;

						// For each pixel in search patch, accumulate difference with base patch						
						for (int px = 0; px < nPatchSize; px++)
							for (int py = 0; py < nPatchSize; py++)
							{
								// Work out search patch offset indices
								int nPatchPixelX = nSearchVectorX + (px - nPatchSize / 2);
								int nPatchPixelY = nSearchVectorY + (py - nPatchSize / 2);

								// Work out base patch indices
								int nBasePixelX = x + (px - nPatchSize / 2);
								int nBasePixelY = y + (py - nPatchSize / 2);

								// Get adjacent values for each patch
								float fPatchPixel = get_pixel(fNewCamera, nPatchPixelX, nPatchPixelY);
								float fBasePixel = get_pixel(fOldCamera, nBasePixelX, nBasePixelY);

								// Accumulate difference
								fAccumulatedDifference += fabs(fPatchPixel - fBasePixel);
							}

						// Record the vector offset for the search patch that is the
						// least different to the base patch
						if (fAccumulatedDifference <= fPatchDifferenceMax)
						{
							fPatchDifferenceMax = fAccumulatedDifference;
							fFlowX[y * ScreenWidth() + x] = (float)(nSearchVectorX - x);
							fFlowY[y * ScreenWidth() + x] = (float)(nSearchVectorY - y);
						}
					}
				}
			}
		}

		// Modulate Optic Flow Vector Map with motion map, to remove vectors that
		// errornously indicate large local motion
		for (int i = 0; i < ScreenWidth() * ScreenHeight(); i++)
		{
			fFlowX[i] *= fMotionImage[i] > 0 ? 1.0f : 0.0f;
			fFlowY[i] *= fMotionImage[i] > 0 ? 1.0f : 0.0f;
		}

		// === Update Ball Physics ========================================================

		// Ball velocity is updated by optic flow vector field
		fBallVX += 100.0f * fFlowX[(int)fBallY * ScreenWidth() + (int)fBallX] * fElapsedTime;
		fBallVY += 100.0f * fFlowY[(int)fBallY * ScreenWidth() + (int)fBallX] * fElapsedTime;

		// Ball position is updated by velocity
		fBallX += 1.0f * fBallVX * fElapsedTime;
		fBallY += 1.0f * fBallVY * fElapsedTime;

		// Add "drag" effect to ball velocity
		fBallVX *= 0.85f;
		fBallVY *= 0.85f;

		// Wrap ball around screen
		if (fBallX >= ScreenWidth()) fBallX -= (float)ScreenWidth();
		if (fBallY >= ScreenHeight()) fBallY -= (float)ScreenHeight();
		if (fBallX < 0) fBallX += (float)ScreenWidth();
		if (fBallY < 0) fBallY += (float)ScreenHeight();

		DrawFrame(dispCameraR,dispCameraG, dispCameraB,dispCameraAlpha);
		FillRect(fBallX - 4, fBallY - 4,8,8,olc::CYAN);
		return true;
	}
};


int main()
{
	Example demo;
	if (demo.Construct(80, 60, 16, 16))
		demo.Start();

	return 0;
}