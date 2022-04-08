#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>

class Basics : public olc::PixelGameEngine
{
public:
	Basics()
	{
		sAppName = "Basic()";
	}
public:
	bool OnUserCreate() override
	{
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);
		return true;
	}
};

int main()
{
	Basics demo;
	if (demo.Construct(200, 100, 4, 4))
	{
		demo.Start();
	}
	return 0;
}