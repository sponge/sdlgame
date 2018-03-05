#include "scene_game.h"
#include <tmx.h>
#include "draw.h"
#include "public.h"

AssetHandle dog, sprites, music, speech, font;
Sprite spr;
tmx_map *map;

void GameScene::Startup(ClientInfo* info) {
	inf = info;
	dog = trap->Asset_Create(ASSET_IMAGE, "dog", "gfx/dog.png");
	sprites = trap->Asset_Create(ASSET_IMAGE, "sprites", "gfx/sprites.gif");
	music = trap->Asset_Create(ASSET_MOD, "music", "music/frantic_-_dog_doesnt_care.it");
	speech = trap->Asset_Create(ASSET_SPEECH, "speech", "great job! you are a good dog!");
	font = trap->Asset_Create(ASSET_BITMAPFONT, "font", "gfx/good_neighbors.png");
	trap->BMPFNT_Set(font, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", -1, 7, 16);

	map = trap->Map_Load(mapFileName);

	trap->Asset_LoadAll();

	trap->Snd_Play(music, 1.0f, 0.0f, true);
	trap->Snd_Play(speech, 1.0f, 0.0f, false);

	spr = DC_CreateSprite(sprites, 8, 8, 0, 0);
}

void GameScene::Update(float dt) {

}

void GameScene::Render() {
	DC_Clear();
	DC_SetTransform(true, inf->width / inf->gameWidth, 0, 0, inf->width / inf->gameWidth, 0, 0);

	DC_DrawMapLayer(0, 0.0f, -150.0f);

	DC_SetColor(COLOR_FILL, 255, 0, 0, 255);
	DC_DrawRect(5, 3, 16, 16);

	DC_DrawImage(120, 120, 154, 16, 0, 0, 1.0, 0, dog, 0);
	DC_DrawBmpText(32, 50, 1.0f, "Good Dog!", font);

	DC_DrawSprite(spr, 265, 200, 200, 1.0f, 0, 3, 3);
	DC_DrawSprite(spr, 265, 180, 200, 0.25f, 0, 3, 3); // FIXME: alpha is broke

	DC_SetColor(COLOR_FILL, 60, 0, 90, 255);
	DC_SetScissor(0, 0, 110, 210);
	DC_DrawCircle(100, 200, 20);
	DC_ResetScissor();

	DC_SetColor(COLOR_STROKE, 0, 255, 0, 255);
	DC_DrawLine(0, 0, 320, 180);

	DC_SetColor(COLOR_STROKE, 130, 140, 150, 255);
	DC_DrawRect(10, 30, 64, 64, OUTLINE);

	DC_SetColor(COLOR_STROKE, 255, 255, 0, 255);
	DC_DrawCircle(200, 25, 16, OUTLINE);

	DC_SetColor(COLOR_FILL, 140, 90, 40, 255);
	DC_DrawCircle(200, 70, 16);

	DC_SetColor(COLOR_FILL, 40, 90, 40, 255);
	DC_DrawTri(150, 150, 160, 160, 130, 160);

	DC_SetColor(COLOR_STROKE, 0, 255, 255, 255);
	DC_DrawTri(150, 180, 170, 170, 180, 180, OUTLINE);

	DC_Submit();
}

GameScene::~GameScene()
{
	trap->Map_Free(map);
}
