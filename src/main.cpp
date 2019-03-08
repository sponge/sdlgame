#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#define NO_SDL_GLEXT
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

extern "C" {
#include "rlgl.h"
	extern bool initGL(int width, int height);
}

#include <imgui.h>
#include "imgui_impl_sdl.h"

#include "files.h"
#include "input.h"
#include "cvar_main.h"

#include "gamedll.h"

#include "scene_console.h"

#include "../game/public.h"

#include <soloud.h>
#include <soloud_thread.h>

#include "filewatcher.h"
#include "crunch_frontend.h"

extern "C" {
#include "consoleng/console.h"
}

conState_t console;

SoLoud::Soloud soloud;
ClientInfo inf;
int64_t frame_musec = 0, com_frameTime = 0;
//float frame_accum;
bool frameAdvance = false;
bool errorVisible = false;

gameExportFuncs_t * gexports;
SDL_Window *window;
ConsoleScene *consoleScene;

void SetWindowTitle(const char *title) {
	SDL_SetWindowTitle(window, title);
}

void Cmd_FrameAdvance_f(void) {
	if (!com_pause->integer) {
		Con_SetVar("com_pause", "1");
	}
	else {
		frameAdvance = true;
	}
}

void Cmd_ToggleConsole_f(void) {
	if (consoleScene == nullptr) {
		return;
	}
	consoleScene->consoleActive = !consoleScene->consoleActive;
}

void Cmd_Vid_Restart_f(void) {
	inf.width = vid_width->integer;
	inf.height = vid_height->integer;

	SDL_SetWindowSize(window, inf.width, inf.height);
	SDL_GL_SetSwapInterval(vid_swapinterval->integer);
	SDL_SetWindowFullscreen(window, vid_fullscreen->integer == 2 ? SDL_WINDOW_FULLSCREEN : vid_fullscreen->integer == 1 ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void DropToMenu() {
	errorVisible = true;
	gexports->Error(ERR_GAME, com_errorMessage->string);
}

void ConH_Print(const char *line) {
	IMConsole()->AddLog("%s", line);
	printf(line);
}

void ConH_Error(int level, const char *message) {
	Con_Print(message);

#if defined(_WIN32) && defined(DEBUG)
	if (level == ERR_FATAL) {
		__debugbreak();
	}
#else
	if (level == ERR_FATAL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", errorMessage, NULL);
	}
#endif

	if (level == ERR_FATAL) {
		exit(1);
	}
	else {
		Con_SetVar("com_errorMessage", message);
		DropToMenu();
	}
}

static bool loop = true;

auto start = std::chrono::steady_clock::now();

static inline long long measure_now() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
}

void main_loop() {

	auto now = measure_now();
	frame_musec = now - com_frameTime;
	com_frameTime = now;

	if (s_volume->modified) {
		soloud.setGlobalVolume(s_volume->value);
		s_volume->modified = false;
	}

	FileWatcher_Tick();

	SDL_Event ev;
	ImGuiIO &io = ImGui::GetIO();
	while (SDL_PollEvent(&ev)) {
		ImGui_ImplSdl_ProcessEvent(&ev);

		switch (ev.type) {
		case SDL_QUIT:
			loop = false;
			return;

		case SDL_KEYUP:
			KeyEvent(ev.key.keysym.scancode, false, com_frameTime);
			break;

		case SDL_KEYDOWN:
			if (ev.key.keysym.sym == SDLK_BACKQUOTE) {
				consoleScene->consoleActive = !consoleScene->consoleActive;
				ImGui::SetWindowFocus(nullptr);
				break;
			}
			if (io.WantCaptureKeyboard) {
				break;
			}
			KeyEvent(ev.key.keysym.scancode, true, com_frameTime);
			break;

		case SDL_CONTROLLERDEVICEADDED: {
			if (ev.cdevice.which > MAX_CONTROLLERS) {
				break;
			}

			SDL_GameController *controller = SDL_GameControllerOpen(ev.cdevice.which);
			Con_Printf("Using controller at device index %i: %s\n", ev.cdevice.which, SDL_GameControllerName(controller));
			break;
		}

		case SDL_CONTROLLERDEVICEREMOVED: {
			SDL_GameController* controller = SDL_GameControllerFromInstanceID(ev.cdevice.which);
			Con_Printf("Closing controller instance %i: %s\n", ev.cdevice.which, SDL_GameControllerName(controller));
			SDL_GameControllerClose(controller);
			break;
		}

		case SDL_MOUSEBUTTONUP:
			MouseEvent(ev.button.button, false, com_frameTime);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (io.WantCaptureKeyboard || io.WantCaptureMouse) {
				break;
			}
			MouseEvent(ev.button.button, true, com_frameTime);
			break;


		case SDL_CONTROLLERBUTTONDOWN:
			JoyEvent(ev.jbutton.which, ev.jbutton.button, true, com_frameTime);
			break;

		case SDL_CONTROLLERBUTTONUP:
			JoyEvent(ev.jbutton.which, ev.jbutton.button, false, com_frameTime);
			break;
		}
	}
	
	//Cbuf_Execute(); // FIXME: the events would add keypresses to a buffer and then execute them all at once

	ImGui_ImplSdl_NewFrame(window);

	if (errorVisible && com_errorMessage->string[0] == '\0') {
		errorVisible = 0;
	}
	else if (errorVisible) {
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("Error", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("%s", com_errorMessage->string);
		ImGui::Text("%s", com_lastErrorStack->string);
		ImGui::NewLine();
		if (ImGui::Button("Close")) {
			errorVisible = false;
			Con_SetVar("com_errorMessage", nullptr);
			Con_SetVar("com_lastErrorStack", nullptr);
		}
		ImGui::End();
	}

	rlClearColor(0, 0, 0, 255);
	rlClearScreenBuffers();

	rlMatrixMode(RL_PROJECTION);                            // Enable internal projection matrix
	rlLoadIdentity();                                       // Reset internal projection matrix
	rlOrtho(0.0, inf.width, inf.height, 0.0, 0.0, 1.0); // Recalculate internal projection matrix
	rlMatrixMode(RL_MODELVIEW);                             // Enable internal modelview matrix
	rlLoadIdentity();                                       // Reset internal modelview matrix

	gexports->Frame(!com_pause->integer || frameAdvance ? frame_musec / 1E6 : 0);
	consoleScene->Update(frame_musec / 1E6);

	if (!com_pause->integer || frameAdvance) {
		frameAdvance = false;
	}

	consoleScene->Render();

	ImGui::Render();
	ImGui_ImplSdl_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(window);

	// OSes seem to not be able to sleep for shorter than a millisecond. so let's sleep until
	// we're close-ish and then burn loop the rest. we get a majority of the cpu/power gains
	// while still remaining pretty accurate on frametimes.
	if (vid_maxfps->integer > 0) {
		long long target = now + (long long) (1000.0f / vid_maxfps->integer * 1000);
		long long currentSleepTime = measure_now();
		while (currentSleepTime <= target) {
			long long amt = (target - currentSleepTime) - 2000;
			if (amt > 0) {
				std::this_thread::sleep_for(std::chrono::microseconds(amt));
			}
			currentSleepTime = measure_now();
		}
	}

}

int main(int argc, char *argv[]) {
	IMConsole();
	console.handlers.print = &ConH_Print;

	Con_Init(&console);

	// handle command line parsing. combine into one string and pass it in.
	
	if (argc > 1) {
		sds cmdline = sdsempty();
		for (int i = 1; i < argc; i++)
		{
			if (i > 1) {
				cmdline = sdscat(cmdline, " ");
			}
			cmdline = sdscat(cmdline, argv[i]);
		}
		Con_ParseCommandLine(cmdline);
		sdsfree(cmdline);
	}

	Con_SetVarFromStartup("fs_basepath");
	Con_SetVarFromStartup("fs_basegame");
	Con_SetVarFromStartup("fs_game");
	FS_Init(argv[0]);

	Con_AddCommand("vid_restart", Cmd_Vid_Restart_f);
	Con_AddCommand("toggleconsole", Cmd_ToggleConsole_f);
	Con_AddCommand("frame_advance", Cmd_FrameAdvance_f);

	RegisterMainCvars();
	CL_InitKeyCommands();
	FileWatcher_Init();
	Crunch_Init();

	if (!FS_Exists("default.cfg")) {
		Con_Error(ERR_FATAL, "Filesystem error, check fs_basepath is set correctly. (Could not find default.cfg)");
	}

	Con_Execute("exec default.cfg\n");
	if (FS_Exists("autoexec.cfg")) {
		Con_Execute("exec autoexec.cfg\n");
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		Con_Error(ERR_FATAL, "There was an error initing SDL2: %s", SDL_GetError());
	}

	atexit(SDL_Quit);
#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GLprofile::SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

	inf.width = vid_width->integer;
	inf.height = vid_height->integer;
	window = SDL_CreateWindow("Slate2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, inf.width, inf.height, SDL_WINDOW_OPENGL);

	if (window == NULL) {
		Con_Error(ERR_FATAL, "There was an error creating the window: %s", SDL_GetError());
	}

	SDL_SetWindowFullscreen(window, vid_fullscreen->integer == 2 ? SDL_WINDOW_FULLSCREEN : vid_fullscreen->integer == 1 ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!initGL(inf.width, inf.height)) {
		Con_Error(ERR_FATAL, "Could not init GL.");
	}

	SDL_GL_SetSwapInterval(vid_swapinterval->integer);

	if (context == NULL) {
		Con_Error(ERR_FATAL, "There was an error creating OpenGL context: %s", SDL_GetError());
	}

	const unsigned char *version = glGetString(GL_VERSION);
	if (version == NULL) {
		Con_Error(ERR_FATAL, "There was an error with OpenGL configuration.");
	}

	soloud.init();
	// FIXME: check result code?

	SDL_GL_MakeCurrent(window, context);

	ImGui::CreateContext();
	ImGui_ImplSdl_Init(window);

	ImGui::StyleColorsDark();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0);

	Con_ExecuteCommandLine();

	consoleScene = new ConsoleScene();
	consoleScene->Startup(&inf);

#ifdef _WIN32
	static const char *lib = "game.dll";
#elif defined MACOS
	static const char *lib = "libgame.dylib";
#else
	static const char *lib = "libgame.so";
#endif

	Sys_LoadDll(lib, (void **)(&gexports));
	gexports->Init((void*)&inf, (void*)ImGui::GetCurrentContext());
	console.handlers.unhandledCommand = gexports->Console;

// not working in emscripten for some reason? assert on ImGuiKey_Space not being mapped
#ifndef __EMSCRIPTEN__	
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#endif

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, 1);
#else
	while (loop) {
		main_loop();
	}
#endif

	ImGui_ImplSdl_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(context);

	return 0;
}
