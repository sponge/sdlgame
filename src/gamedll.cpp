#include <SDL/SDL.h>
#include <tmx.h>

#include "gamedll.h"
#include "../game/public.h"
#include "console/console.h"
#include "main.h"
#include "image.h"
#include "files.h"
#include "scene.h"

extern SceneManager *sm;
extern ClientInfo inf;

void trap_SendConsoleCommand(const char *text) {
	Cbuf_ExecuteText(EXEC_NOW, text);
}

void trap_Scene_Switch(Scene *newScene) {
	sm->Switch(newScene);
}

Scene * trap_Scene_Get(int i) {
	return sm->Get(i);
}

void trap_Scene_Replace(int i, Scene *newScene) {
	sm->Replace(i, newScene);
}

Scene * trap_Scene_Current() {
	return sm->Current();
}

tmx_map * trap_Map_Load(const char *filename) {

	tmx_img_load_func = [](const char *path) {
		Img *img = Img_Create(path, path);
		Img_Load(inf.nvg, *img);
		return img;
	};

	tmx_img_free_func = [](void *address) {
		Img *img = (Img*)address;
		Img_Free(img->path);
		delete img;
	};

	tmx_file_read_func = [](const char *path, int *outSz) -> void* {
		void *xml;

		*outSz = FS_ReadFile(path, &xml);

		if (outSz < 0) {
			Com_Error(ERR_DROP, "Couldn't load file while parsing map %s", path);
			return nullptr;
		}

		return xml;
	};

	tmx_map *map = tmx_load(filename);

	if (map == nullptr) {
		Com_Error(ERR_DROP, "Failed to load tmx");
		return nullptr;
	}

	return map;
}

void trap_Map_Free(tmx_map *map) {
	tmx_map_free(map);
}

static gameImportFuncs_t GAMEtraps = {
	trap_SendConsoleCommand,
	Com_Printf,
	Com_Error,
	Cvar_Get,
	Cvar_FindVar,
	Cvar_Set,
	Cmd_AddCommand,
	Cmd_Argc,
	Cmd_Argv,
	Cmd_ArgsFrom,
	Cmd_Cmd,
	FS_ReadFile,
	FS_Exists,
	FS_List,
	FS_FreeList,
	Img_Create,
	Img_Load,
	Img_LoadAll,
	Img_Find,
	Img_Free,
	trap_Scene_Switch,
	trap_Scene_Get,
	trap_Scene_Replace,
	trap_Scene_Current,
	trap_Map_Load,
	trap_Map_Free
};

void Sys_LoadDll(const char * module, void ** exports, int * version) {
	auto gameDLL = SDL_LoadObject(module);

	void(*gameDllEntry)(void ** exports, const void * imports, int * version) = (void(*)(void **, const void *, int *)) SDL_LoadFunction(gameDLL, "dllEntry");

	if (gameDllEntry == nullptr) {
		Com_Error(ERR_FATAL, "Couldn't find dllEntry in %s", module);
	}

	gameDllEntry(exports, &GAMEtraps, version);

}
