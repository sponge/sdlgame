extern "C" {
#include <quickjs.h>
}

JSModuleDef *js_init_module_draw(JSContext *ctx, const char *module_name);
JSModuleDef *js_init_module_slt(JSContext *ctx, const char *module_name);