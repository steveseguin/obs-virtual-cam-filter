/* Minimal OBS 26 API double for the lifecycle regression test, not a plugin SDK. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define UNUSED_PARAMETER(x) (void)(x)
#define MAX_AV_PLANES 8
#define VIDEO_OUTPUT_SUCCESS 0
#define GS_BGRA 0
#define GS_ZS_NONE 0
#define GS_CLEAR_COLOR 0
#define GS_BLEND_ONE 1
#define GS_BLEND_ZERO 0
#define VIDEO_FORMAT_BGRA 0
#define VIDEO_CS_DEFAULT 0
#define VIDEO_RANGE_DEFAULT 0
#define OBS_SOURCE_TYPE_FILTER 0
#define OBS_SOURCE_VIDEO 0
#define OBS_DECLARE_MODULE()
#define OBS_MODULE_USE_DEFAULT_LOCALE(a,b)
#define MODULE_EXPORT
struct obs_source { uint32_t width, height; bool enabled; struct obs_source *target; };
typedef struct obs_source obs_source_t;
typedef struct video { uint32_t width, height; bool attached; } video_t;
typedef struct output { video_t *video; bool active, stopping; } obs_output_t;
typedef struct surface { bool mapped; uint8_t *data; uint32_t stride; } gs_stagesurf_t;
typedef struct texrender { uint32_t width, height; } gs_texrender_t;
typedef void obs_data_t;
typedef void obs_properties_t;
typedef void gs_effect_t;
struct obs_video_info { uint32_t fps_den, fps_num; };
struct video_output_info { int format; uint32_t width, height, fps_den, fps_num, cache_size; int colorspace, range; const char *name; };
struct vec4 { float data[4]; };
struct video_frame;
static unsigned opens, closes, starts, stops, locks, renders, unmaps, destroyed_surfaces;
static bool fail_open, fail_start;
static unsigned graphics_depth;
static obs_output_t *output;
static uint8_t frame_data[1024 * 1024];
static void *bzalloc(size_t n) { return calloc(1, n); }
#define bfree free
static obs_source_t *obs_filter_get_target(obs_source_t *s) { return s->target; }
static uint32_t obs_source_get_base_width(obs_source_t *s) { return s->width; }
static uint32_t obs_source_get_base_height(obs_source_t *s) { return s->height; }
static bool obs_source_enabled(obs_source_t *s) { return s->enabled; }
static const char *obs_source_get_name(obs_source_t *s) { (void)s; return "test"; }
static obs_output_t *obs_output_create(const char *a, const char *b, void *c, void *d) { (void)a;(void)b;(void)c;(void)d; output = calloc(1,sizeof(*output)); return output; }
static void obs_output_set_media(obs_output_t *o, video_t *v, void *a) { (void)a; assert(!o->active); o->video = v; }
static bool obs_output_start(obs_output_t *o) { assert(!o->active && !o->stopping); if (fail_start) return false; assert(o->video); o->active = o->video->attached = true; starts++; return true; }
/* Stop deliberately stays active until the test completes the async work. */
static void obs_output_stop(obs_output_t *o) { assert(o->active && !o->stopping); o->stopping = true; stops++; }
static bool obs_output_active(obs_output_t *o) { return o->active; }
static void finish_stop(obs_output_t *o) { assert(o->stopping); o->video->attached = false; o->active = o->stopping = false; }
static void obs_output_release(obs_output_t *o) { if (!o) return; if (o->active) { if (!o->stopping) obs_output_stop(o); finish_stop(o); } free(o); output = NULL; }
static int video_output_open(video_t **v, const struct video_output_info *i) { assert(i->width && i->height); if (fail_open) return -1; *v = calloc(1,sizeof(**v)); (*v)->width = i->width; (*v)->height = i->height; opens++; return 0; }
static void video_output_close(video_t *v) { if (!v) return; assert(!v->attached); if (output) assert(output->video != v); closes++; free(v); }
static bool video_output_lock_frame(video_t *, struct video_frame *, int, uint64_t);
static void video_output_unlock_frame(video_t *v) { (void)v; }
static uint64_t obs_get_video_frame_time(void) { return 0; }
static void obs_enter_graphics(void) { graphics_depth++; }
static void obs_leave_graphics(void) { assert(graphics_depth); graphics_depth--; }
static gs_stagesurf_t *gs_stagesurface_create(uint32_t w, uint32_t h, int f) { (void)f; gs_stagesurf_t *s=calloc(1,sizeof(*s)); s->stride=w*4; s->data=calloc(h,s->stride); return s; }
static void gs_stagesurface_unmap(gs_stagesurf_t *s) { assert(s && s->mapped); s->mapped=false; unmaps++; }
static void gs_stagesurface_destroy(gs_stagesurf_t *s) { if (!s) return; assert(!s->mapped); free(s->data); free(s); destroyed_surfaces++; }
static void gs_stagesurface_map(gs_stagesurf_t *s, uint8_t **d, uint32_t *l) { assert(!s->mapped); s->mapped=true; *d=s->data; *l=s->stride; }
static void gs_stage_texture(gs_stagesurf_t *s, gs_texrender_t *t) { assert(s && s->stride==t->width*4); }
static gs_texrender_t *gs_texrender_create(int a, int b) { (void)a;(void)b; return calloc(1,sizeof(gs_texrender_t)); }
static void gs_texrender_destroy(gs_texrender_t *t) { free(t); }
static void gs_texrender_reset(gs_texrender_t *t) { (void)t; }
static bool gs_texrender_begin(gs_texrender_t *t, uint32_t w, uint32_t h) { t->width=w; t->height=h; renders++; return true; }
static gs_texrender_t *gs_texrender_get_texture(gs_texrender_t *t) { return t; }
static void gs_texrender_end(gs_texrender_t *t) { (void)t; }
static void vec4_zero(struct vec4 *v) { memset(v,0,sizeof(*v)); }
static void gs_clear(int a, struct vec4 *b, float c, int d) { (void)a;(void)b;(void)c;(void)d; }
static void gs_ortho(float a,float b,float c,float d,float e,float f) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f; }
static void gs_blend_state_push(void) {}
static void gs_blend_state_pop(void) {}
static void gs_blend_function(int a,int b) { (void)a;(void)b; }
static void obs_source_skip_video_filter(obs_source_t *s) { (void)s; }
static void obs_get_video_info(struct obs_video_info *i) { i->fps_num=30; i->fps_den=1; }
static void obs_add_main_render_callback(void (*fn)(void*,uint32_t,uint32_t),void *p) { (void)fn;(void)p; }
static void obs_remove_main_render_callback(void (*fn)(void*,uint32_t,uint32_t),void *p) { (void)fn;(void)p; }
static obs_properties_t *obs_properties_create(void) { return NULL; }
struct obs_source_info {
 const char *id; int type, output_flags;
 const char *(*get_name)(void *); void *(*create)(obs_data_t *,obs_source_t *);
 void (*destroy)(void *); void (*update)(void *,obs_data_t *); void (*get_defaults)(obs_data_t *);
 void (*video_render)(void *,gs_effect_t *); void (*video_tick)(void *,float);
 obs_properties_t *(*get_properties)(void *); void (*filter_remove)(void *,obs_source_t *);
 uint32_t (*get_width)(void *), (*get_height)(void *);
};
static void obs_register_source(struct obs_source_info *i) { (void)i; }
