#pragma once
#include <entityx/entityx.h>
#include <SDL/SDL.h>
#include <vector>
#include "scene.h"
#include <tmx.h>
#include "components.h"

class GameWorld : public entityx::EntityX {
public:
	explicit GameWorld();
	bool Load(const char *filename);
	void update(entityx::TimeDelta dt);
	TileMap *tmap;
	const char *error;
	~GameWorld();
};

class MapScene : public Scene {
public:
	MapScene(const char *filename);
	void Startup(ClientInfo* i) override;
	void Update(float dt) override;
	void Render() override;
	//bool Event(SDL_Event *ev) override;
	void* nvg_img_load_func(const char *path);
	void nvg_img_free_func(void *address);
	void* physfs_file_read_func(const char *path, int *outSz);
	~MapScene();

private:
	const char *fileName;
	ClientInfo* inf;
	entityx::SystemManager *rendSys;
	GameWorld *world;
};