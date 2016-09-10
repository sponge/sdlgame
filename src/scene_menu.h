#pragma once
#include <SDL/SDL.h>
#include <vector>
#include "scene.h"
#include "local.h"

class MenuScene : public Scene {
public:
	void Startup(ClientInfo* i) override;
	void Teardown() override;
	void Update(double dt) override;
	void Render() override;

private:
	ClientInfo* inf;
};