#include "rendercommands.h"
#include <nanovg.h>
#include <tmx.h>
#include "assetloader.h"
#include "bitmapfont.h"

#include "console/console.h"

extern ClientInfo inf;
extern tmx_map *map;

const void *RB_SetColor(const void *data) {
	auto cmd = (const setColorCommand_t *)data;

	if (cmd->which == COLOR_FILL) {
		nvgFillColor(inf.nvg, nvgRGBA(cmd->color[0], cmd->color[1], cmd->color[2], cmd->color[3])); 
	}
	else {
		nvgStrokeColor(inf.nvg, nvgRGBA(cmd->color[0], cmd->color[1], cmd->color[2], cmd->color[3]));
	}

	return (const void *)(cmd + 1);
}

const void *RB_SetTransform(const void *data) {
	auto cmd = (const setTransformCommand_t *)data;

	if (cmd->absolute) {
		nvgResetTransform(inf.nvg);
	}

	nvgTransform(inf.nvg, cmd->transform[0], cmd->transform[1], cmd->transform[2], cmd->transform[3], cmd->transform[4], cmd->transform[5]);

	return (const void *)(cmd + 1);
}

const void *RB_Rotate(const void *data) {
	auto cmd = (const rotateCommand_t *)data;

	nvgRotate(inf.nvg, cmd->angle);

	return (const void *)(cmd + 1);
}

const void *RB_Translate(const void *data) {
	auto cmd = (const translateCommand_t *)data;

	nvgTranslate(inf.nvg, cmd->x, cmd->y);

	return (const void *)(cmd + 1);
}

const void *RB_SetScissor(const void *data) {
	auto cmd = (const setScissorCommand_t *)data;
	if (cmd->w <= 0 || cmd->h <= 0) {
		nvgResetScissor(inf.nvg);
	}
	else {
		nvgScissor(inf.nvg, cmd->x, cmd->y, cmd->w, cmd->h);
	}

	return (const void *)(cmd + 1);
}

const void *RB_DrawRect(const void *data) {
	auto cmd = (const drawRectCommand_t *)data;

	nvgBeginPath(inf.nvg);
	nvgRect(inf.nvg, cmd->x, cmd->y, cmd->w, cmd->h);
	if (cmd->outline) {
		nvgStroke(inf.nvg);
	}
	else {
		nvgFill(inf.nvg);
	}

	return (const void *)(cmd + 1);
}

const void *RB_DrawText(const void *data) {
	auto cmd = (const drawTextCommand_t *)data;

	nvgSave(inf.nvg);
	nvgTextAlign(inf.nvg, cmd->align);
	nvgText(inf.nvg, cmd->x, cmd->y, cmd->text, 0);
	nvgRestore(inf.nvg);

	return (const void *)(cmd + 1);
}

const void *RB_DrawBmpText(const void *data) {
	auto cmd = (const drawBmpTextCommand_t *)data;

	BMPFNT_DrawText(cmd->fntId, cmd->x, cmd->y, cmd->scale, cmd->text);

	return (const void *)(cmd + 1);
}

void DrawImage(float x, float y, float w, float h, float ox, float oy, float alpha, float scale, byte flipBits, Image *img, unsigned int shaderId) {
	float flipX = flipBits & FLIP_H ? -1.0f : 1.0f;
	float flipY = flipBits & FLIP_V ? -1.0f : 1.0f;
	bool flipDiag = flipBits & FLIP_DIAG;

	nvgSave(inf.nvg);

	nvgTranslate(inf.nvg, x, y);
	nvgScale(inf.nvg, scale, scale);

	if (flipDiag) {
		nvgTransform(inf.nvg, 0, 1, 1, 0, 0, 0);
	}

	if ((flipX == -1) ^ (flipY == -1) && flipDiag) {
		nvgScale(inf.nvg, flipY, flipX);
	}
	else {
		nvgScale(inf.nvg, flipX, flipY);
	}

	nvgTranslate(inf.nvg, -(w / 2), -(h / 2));

	auto paint = nvgImagePattern(inf.nvg, 0 - ox, 0 - oy, img->w, img->h, 0, img->hnd, alpha);
	paint.shader = shaderId;
	nvgGlobalCompositeBlendFuncSeparate(inf.nvg, NVG_SRC_ALPHA, NVG_ONE_MINUS_SRC_ALPHA, NVG_ONE, NVG_ONE_MINUS_SRC_ALPHA);
	nvgBeginPath(inf.nvg);
	nvgRect(inf.nvg, 0, 0, w, h);
	nvgFillPaint(inf.nvg, paint);
	nvgFill(inf.nvg);

	nvgRestore(inf.nvg);
}

const void *RB_DrawImage(const void *data) {
	auto cmd = (const drawImageCommand_t *)data;
	Image *image = Get_Img(cmd->imgId);
	float w = cmd->w == 0 ? image->w : cmd->w;
	float h = cmd->h == 0 ? image->h : cmd->h;
	DrawImage(cmd->x, cmd->y, w, h, cmd->ox, cmd->oy, cmd->alpha, cmd->scale, cmd->flipBits, image, cmd->shaderId);
	return (const void *)(cmd + 1);
}

const void *RB_DrawLine(const void *data) {
	auto cmd = (const drawLineCommand_t *)data;
	nvgBeginPath(inf.nvg);
	nvgMoveTo(inf.nvg, cmd->x1, cmd->y1);
	nvgLineTo(inf.nvg, cmd->x2, cmd->y2);
	nvgStroke(inf.nvg);

	return (const void *)(cmd + 1);
}

const void *RB_DrawCircle(const void *data) {
	auto cmd = (const drawCircleCommand_t *)data;

	nvgBeginPath(inf.nvg);
	nvgCircle(inf.nvg, cmd->x, cmd->y, cmd->radius);
	if (cmd->outline) {
		nvgStroke(inf.nvg);
	}
	else {
		nvgFill(inf.nvg);
	}

	return (const void *)(cmd + 1);
}

const void *RB_DrawTri(const void *data) {
	auto cmd = (const drawTriCommand_t *)data;

	nvgBeginPath(inf.nvg);
	nvgMoveTo(inf.nvg, cmd->x1, cmd->y1);
	nvgLineTo(inf.nvg, cmd->x2, cmd->y2);
	nvgLineTo(inf.nvg, cmd->x3, cmd->y3);
	nvgLineTo(inf.nvg, cmd->x1, cmd->y1);
	
	if (cmd->outline) {
		nvgStroke(inf.nvg);
	}
	else {
		nvgFill(inf.nvg);
	}

	return (const void *)(cmd + 1);
}

const void *RB_DrawMapLayer(const void *data) {
	auto cmd = (const drawMapCommand_t *)data;

	tmx_layer *layer = map->ly_head;
	unsigned int i = 0;
	while (layer != nullptr && i++ != cmd->layer) {
		layer = layer->next;
	}

	assert(layer != nullptr);

	unsigned int cellW = cmd->cellW == 0 ? map->width : cmd->cellW;
	unsigned int cellH = cmd->cellH == 0 ? map->height : cmd->cellH;

	for (unsigned int y = cmd->cellY; y < cellH; y++) {
		for (unsigned int x = cmd->cellX; x < cellW; x++) {
			unsigned int raw = layer->content.gids[(y*map->width) + x];
			unsigned int gid = raw & TMX_FLIP_BITS_REMOVAL;

			if (gid == 0) {
				continue;
			}

			byte flipBits = (raw & TMX_FLIPPED_HORIZONTALLY ? FLIP_H : 0) | (raw & TMX_FLIPPED_VERTICALLY ? FLIP_V : 0) | (raw & TMX_FLIPPED_DIAGONALLY ? FLIP_DIAG : 0);

			tmx_tile *tile = map->tiles[gid];
			tmx_tileset *ts = tile->tileset;
			Asset *asset = (Asset*) tile->tileset->image->resource_image;

			DrawImage(
				cmd->x + x*ts->tile_width + (ts->tile_width / 2), cmd->y + y*ts->tile_height + (ts->tile_height / 2),
				ts->tile_width, ts->tile_height,
				tile->ul_x, tile->ul_y,
				1.0f, 1.0f, flipBits, (Image*) asset->resource, 0
			);
		}
	}

	return (const void *)(cmd + 1);
}

void SubmitRenderCommands(renderCommandList_t * list) {
	const void *data = list->cmds;

	nvgSave(inf.nvg);
	while (1) {
		switch (*(const byte *)data) {

		case RC_SET_COLOR:
			data = RB_SetColor(data);
			break;

		case RC_SET_TRANSFORM:
			data = RB_SetTransform(data);
			break;

		case RC_ROTATE:
			data = RB_Rotate(data);
			break;

		case RC_TRANSLATE:
			data = RB_Translate(data);
			break;

		case RC_SET_SCISSOR:
			data = RB_SetScissor(data);
			break;

		case RC_DRAW_RECT:
			data = RB_DrawRect(data);
			break;

		case RC_DRAW_TEXT:
			data = RB_DrawText(data);
			break;

		case RC_DRAW_BMPTEXT:
			data = RB_DrawBmpText(data);
			break;

		case RC_DRAW_IMAGE:
			data = RB_DrawImage(data);
			break;

		case RC_DRAW_LINE:
			data = RB_DrawLine(data);
			break;

		case RC_DRAW_CIRCLE:
			data = RB_DrawCircle(data);
			break;

		case RC_DRAW_TRI:
			data = RB_DrawTri(data);
			break;

		case RC_DRAW_MAP_LAYER:
			data = RB_DrawMapLayer(data);
			break;

		case RC_END_OF_LIST:
			nvgRestore(inf.nvg);
			return;

		default:
			Com_Error(ERR_FATAL, "Bad render command byte id %i", *(const int *)data);
			return;
		}
	}
}
