#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>

// ok so planning
// make menu:
//	play, palette, exit buttons
//		play lets you select difficulty that is based on size and ammount of bombs
//		palette lets you choose the colors for background, numbers, dug cells etc
// ???
// profit

class Button {
protected:
	olc::vf2d pos;
	olc::vf2d size;
	olc::vf2d scale;
	std::string text;
	int id;
	int state;
public:
	Button() {};
	Button(olc::vi2d p, int i, std::string t) {
		pos = p;
		size = { 90,15 };
		scale = { 4,4 };
		text = t;
		id = i;
		state = 0;
	}
	void update(int& nMenu, olc::PixelGameEngine* pge) {
		state = 0;
		olc::vf2d mouse = pge->GetMousePos();
		if (pos.x > mouse.x) return;
		if (pos.y > mouse.y) return;
		if (pos.x + size.x * scale.x < mouse.x) return;
		if (pos.y + size.y * scale.y < mouse.y) return;
		state = 1;//mouse over
		if (pge->GetMouse(0).bPressed) {
			state = 2;//mouse clicked
			nMenu = id;
		}
	}
	void draw(olc::Pixel colors[6], olc::PixelGameEngine* pge) {
		pge->FillRect(pos, scale*size, colors[state+1]);
		pge->FillRect(pos, scale*(size - olc::vi2d{ 1,1 }), colors[state+3]);
		pge->FillRect(pos + scale * (olc::vi2d{1, 1}), scale * (size - olc::vi2d{2,2}), colors[state+2]);
		pge->DrawStringDecal(pos + scale * (olc::vi2d{ 3,3 }), text, colors[state+4], scale);
	}
};
class ValueButton : Button {

public:
	bool bShowVal;
	int val[3];//min,max,curr
	ValueButton() {};
	ValueButton(olc::vi2d p, int i, bool b, std::array<int,3> v, std::string t) {
		pos = p;
		size = { 90,15 };
		scale = { 4,4 };
		text = t;
		id = i;
		bShowVal = b;
		for (int i = 0; i < 3; i++) val[i] = v[i];
		state = 0;
	}
	void update(olc::PixelGameEngine*pge){
		state = 0;
		olc::vf2d mouse = pge->GetMousePos();
		if (pos.x > mouse.x) return;
		if (pos.y > mouse.y) return;
		if (pos.x + size.x * scale.x < mouse.x) return;
		if (pos.y + size.y * scale.y < mouse.y) return;
		state = 1;//mouse over
		if (pge->GetMouse(0).bPressed) {
			val[2] += 1 * (val[2] < val[1]);
		}
		if (pge->GetMouse(1).bPressed) {
			val[2] -= 1 * (val[2] > val[0]);
		}
	}
	void draw(olc::Pixel colors[6], olc::PixelGameEngine* pge) {
		pge->FillRect(pos, scale * size, colors[state+1]);
		pge->FillRect(pos, scale * (size - olc::vi2d{ 1,1 }), colors[state + 3]);
		pge->FillRect(pos + scale * (olc::vi2d{ 1, 1 }), scale * (size - olc::vi2d{ 2,2 }), colors[state + 2]);
		pge->DrawStringDecal(pos + scale * (olc::vi2d{ 3,3 }), text + std::to_string(val[2]), colors[state + 4], scale);
	}
};
class Board {
private:
	bool bombs[30][16];
	int nBombsLeft;
	char grid[30][16];
	olc::vi2d size;
	olc::vi2d scale = { 30,30 };
	olc::Pixel palette[6];
public:
	Board() {};
	Board(olc::vi2d sz, int nBomb, olc::Pixel p[6]) {
		size = sz;
		for (int i = 0; i < 6; i++) palette[i] = p[i];
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				bombs[x][y] = false;
				grid[x][y] = ' ';
			}
		}
		nBombsLeft = nBomb;
		for (int b = 0; b < nBomb; b++)
		{
			int rndX = std::rand() % size.x;
			int rndY = std::rand() % size.y;
			if (!bombs[rndX][rndY]) bombs[rndX][rndY] = true;
			else b--;
		}
	}
	void Dig(olc::vi2d pos) {
		//count neighbors
		int neighBombs = 0;
		for (int i = -1 + 1 * (pos.x == 0); i <= 1 - 1 * (pos.x == size.x - 1); i++)
		{
			for (int j = -1 + 1 * (pos.y == 0); j <= 1 - 1 * (pos.y == size.y - 1); j++)
			{
				//if its a bomb and not itself, add to neighbors
				if (bombs[pos.x + i][pos.y + j]&&(i!=0||j!=0)) neighBombs++;
			}
		}
		//assign value
		grid[pos.x][pos.y] = '0' + neighBombs;
		//if empty cell, clear neighbors
		if(neighBombs==0){
			for (int i = -1 + 1 * (pos.x == 0); i <= 1 - 1 * (pos.x == size.x - 1); i++)
			{
				for (int j = -1 + 1 * (pos.y == 0); j <= 1 - 1 * (pos.y == size.y - 1); j++)
				{
					olc::vi2d newPos = { pos.x + i,pos.y + j };
					if ((newPos.x >= 0) && (newPos.y >= 0) && (newPos.x < size.x) && (newPos.y < size.y)){
						//if its not a bomb, not itself, and not dug already, dig
						if (!bombs[pos.x + i][pos.y + j] && (i != 0 || j != 0) && grid[pos.x + i][pos.y + j] == ' ') Dig(pos + olc::vi2d{ i,j });
					}
				}
			}
		}
	}
	void update(int& gameState, olc::PixelGameEngine* pge) {
		//get mouse and transform it into the grid
		olc::vi2d mouse = pge->GetMousePos();
		olc::vi2d gridPos = mouse / scale;
		if (gridPos.x < size.x && gridPos.y < size.y) {
			if (pge->GetMouse(0).bPressed) {
				//dig
				//if its a bomb, set it to be drawn and end game
				if (bombs[gridPos.x][gridPos.y]) {
					grid[gridPos.x][gridPos.y] = 'B';
					gameState++;
					return;
				}
				//if not, dig to calculate neighbors and write the number
				Dig(gridPos);
			}
			else if (pge->GetMouse(1).bPressed) {
				//if its a flag, set to not dug, vice versa
				grid[gridPos.x][gridPos.y] = (grid[gridPos.x][gridPos.y]=='F')?' ':'F';
				//if flagging a bomb -1 left, if unflagging +1 left
				if (bombs[gridPos.x][gridPos.y]) (grid[gridPos.x][gridPos.y] == 'F')? nBombsLeft--: nBombsLeft++;
				if (nBombsLeft == 0) {
					gameState += 2;
				}
			}
		}
	}
	void draw(olc::PixelGameEngine* pge) {
		olc::Pixel nums[9] = {
			olc::Pixel(128,128,128),
			olc::Pixel(128,0,0),
			olc::Pixel(128,64,0),
			olc::Pixel(128,128,0),
			olc::Pixel(0,128,0),
			olc::Pixel(0,128,128),
			olc::Pixel(0,0,128),
			olc::Pixel(64,0,128) 
		};
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				bool bDug = grid[x][y] != ' ' && grid[x][y] != 'F';

				olc::vi2d pos = scale * olc::vi2d{ x,y };
				pge->FillRect(pos, scale, palette[1+!bDug]);
				pge->FillRect(pos, scale - olc::vi2d{ 4, 4 }, palette[(bDug)?1:4]);
				pge->FillRect(pos + olc::vi2d{ 4, 4 }, scale  - olc::vi2d{ 8,8 }, palette[3-bDug]);
				//if flag, draw flag
				if (grid[x][y] == 'F') {
					pge->FillTriangle(pos + olc::vi2d{ scale.x/2,0 }, pos + olc::vi2d{ scale.x / 2,scale.y / 2 }, pos + olc::vi2d{ scale.x,scale.y/4 }, olc::RED);
					pge->FillRect(pos + olc::vi2d{ -4+scale.x/2,0 }, { 4,scale.y }, olc::RED);
				}
				//if bomb and not flagged nor covered, draw
				else if (bombs[x][y]&&grid[x][y]!='F'&&grid[x][y]!=' ') {
					pge->FillCircle(pos + scale / 2, scale.x / 4, olc::BLACK);
				}
				else if(grid[x][y]!=' '){
					std::string s(1, grid[x][y]);
					pge->DrawString(pos + scale/4, s, nums[grid[x][y] - '0'], 2);
				}
			}
		}
		pge->DrawString(scale.x*size.x,0, std::to_string(nBombsLeft), olc::BLACK, 2);
	}
};

class GameManager {
private:
	olc::PixelGameEngine* pge;
	olc::Pixel palettes[8][6] = {
		{olc::Pixel(96,96,96),olc::Pixel(128,128,128),olc::Pixel(160,160,160),olc::Pixel(192,192,192),olc::Pixel(224,224,224),olc::Pixel(255,255,255)},
		{olc::Pixel(96,0,0),olc::Pixel(128,0,0),olc::Pixel(160,0,0),olc::Pixel(192,0,0),olc::Pixel(224,0,0),olc::Pixel(255,0,0)},
		{olc::Pixel(96,48,0),olc::Pixel(128,64,0),olc::Pixel(160,80,0),olc::Pixel(192,96,0),olc::Pixel(224,112,0),olc::Pixel(255,128,0)},
		{olc::Pixel(96,96,0),olc::Pixel(128,128,0),olc::Pixel(160,160,0),olc::Pixel(192,192,0),olc::Pixel(224,224,0),olc::Pixel(255,255,0)},
		{olc::Pixel(0,96,0),olc::Pixel(0,128,0),olc::Pixel(0,160,0),olc::Pixel(0,192,0),olc::Pixel(0,224,0),olc::Pixel(0,255,0)},
		{olc::Pixel(0,96,96),olc::Pixel(0,128,128),olc::Pixel(0,160,160),olc::Pixel(0,192,192),olc::Pixel(0,224,224),olc::Pixel(0,255,255)},
		{olc::Pixel(0,0,96),olc::Pixel(0,0,128),olc::Pixel(0,0,160),olc::Pixel(0,0,192),olc::Pixel(0,0,224),olc::Pixel(0,0,255)},
		{olc::Pixel(48,0,96),olc::Pixel(64,0,128),olc::Pixel(80,0,160),olc::Pixel(96,0,192),olc::Pixel(112,0,224),olc::Pixel(128,0,255)}
	};
	int nMenu = 0;
	int nCurrPalette = 0;
	int gameState = 0;
	float fTimer = 5.0f;
	Button play, opt, optExit, exit;
	ValueButton colors, diff, bomb, width,height;
	Board board;
public:
	olc::Pixel background = palettes[nCurrPalette][0];
	GameManager() {};
	GameManager(olc::PixelGameEngine* pg) {
		pge = pg;
		play = Button({ 5,5 } , 1, "Start");
		opt = Button({ 5,85 } , 2, "Settings");
		exit = Button({ 5,165 } , -1, "Exit");

		colors = { { 5,5 }, 1, true, {0,7,0}, "Palette:" };
		diff = ValueButton({ 5,85 }, 0, true, { 1,3,3 }, "Level:  ");
		bomb = ValueButton({ 405,5 }, 0, true, { 1,99,99 }, "Bombs:  ");
		width = ValueButton({ 405,85 }, 0, true, { 1,30,30 }, "Width:  ");
		height = ValueButton({ 405,165 }, 0, true, { 1,30,16 }, "Height: ");

		optExit = Button({ 5,165 }, 0, "Back");
	};
	bool bGame() {
		return nMenu >= 0;
	}
	void update() {
		switch (nMenu) {
			case 0:
				//handle button inputs
				play.update(nMenu, pge);
				opt.update(nMenu, pge);
				exit.update(nMenu, pge);
				break;
			case 1:
				switch (gameState) {
				case 0:
					//update level to start generating game
					switch (diff.val[2]) {
					case 1:
						bomb.val[2] = 10;
						width.val[2] = 10;
						height.val[2] = 8;
						break;
					case 2:
						bomb.val[2] = 40;
						width.val[2] = 16;
						height.val[2] = 13;
						break;
					case 3:
						bomb.val[2] = 99;
						width.val[2] = 30;
						height.val[2] = 16;
						break;
					}
					board = Board({ width.val[2], height.val[2] }, bomb.val[2], palettes[nCurrPalette]);
					fTimer = 5.0f;
					gameState++;
					break;
				case 1:
					board.update(gameState, pge);
					break;
				case 2:
					//game over
					fTimer = (fTimer > 0.01f) ? fTimer - 0.1f : 0.0f;
					if (fTimer < 0.01f) {
						gameState = 0;
						nMenu = 0;
						fTimer = 5.0f;
					}
					break;
				case 3:
					//game won
					fTimer = (fTimer > 0.01f) ? fTimer - 0.1f : 0.0f;
					if (fTimer < 0.01f) {
						gameState = 0;
						nMenu = 0;
						fTimer = 5.0f;
					}
					break;
				}
				break;
			case 2:
				//update color button and inheriting values
				colors.update(pge);
				nCurrPalette = colors.val[2];
				background = palettes[nCurrPalette][0];
				//update level and inheriting values
				diff.update(pge);
				switch (diff.val[2]) {
				case 1:
					bomb.val[2] = 10;
					width.val[2] = 10;
					height.val[2] = 8;
					break;
				case 2:
					bomb.val[2] = 40;
					width.val[2] = 16;
					height.val[2] = 13;
					break;
				case 3:
					bomb.val[2] = 99;
					width.val[2] = 30;
					height.val[2] = 16;
					break;
				}
				//handle option menu exiting
				optExit.update(nMenu, pge);
				break;
		}
	}
	void draw() {
		switch (nMenu) {
			case 0:
				//draw main menu
				play.draw(palettes[nCurrPalette],pge);
				opt.draw(palettes[nCurrPalette],pge);
				exit.draw(palettes[nCurrPalette],pge);
				break;
			case 1:
				//draw game
				board.draw(pge);
				if(gameState == 2) pge->DrawString(0, 40 * height.val[2], "GAME OVER! ", olc::BLACK, 4);
				if (gameState == 3)pge->DrawString(0, 40 * height.val[2], "CONGRATULATIONS!", olc::BLACK, 4);
				break;
			case 2:
				//draw options menu
				colors.draw(palettes[nCurrPalette], pge);
				diff.draw(palettes[nCurrPalette], pge);
				bomb.draw(palettes[nCurrPalette], pge);
				width.draw(palettes[nCurrPalette], pge);
				height.draw(palettes[nCurrPalette], pge);
				optExit.draw(palettes[nCurrPalette], pge);
				break;
		}
		pge->DrawString(0, 0, std::to_string(nMenu), olc::WHITE);
	}
};

class Basics : public olc::PixelGameEngine
{
public:
	Basics()
	{
		sAppName = "Basic()";
	}
public:
	GameManager gm;
	bool OnUserCreate() override
	{
		gm = GameManager(this);
		SetPixelMode(olc::Pixel::Mode::ALPHA);
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(gm.background);
		gm.update();
		gm.draw();
		return gm.bGame();
	}
};

int main()
{
	Basics demo;
	if (demo.Construct(1000, 800, 1, 1))
	{
		demo.Start();
	}
	return 0;
}