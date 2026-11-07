#include "../virtual-cam-filter.c"
#include <stdio.h>

static bool video_output_lock_frame(video_t *v, struct video_frame *f,
                                   int count, uint64_t time)
{
 (void)count; (void)time;
 assert(v && output && output->video == v && output->active);
 assert(v->width * v->height * 4 <= sizeof(frame_data));
 f->data[0] = frame_data;
 f->linesize[0] = v->width * 4;
 locks++;
 return true;
}

static void tick(struct virtual_cam_filter_context *c)
{
 virtual_cam_filter_source_tick(c, 0);
}
static void render(struct virtual_cam_filter_context *c)
{
 virtual_cam_filter_offscreen_render(c, 0, 0);
}

int main(void)
{
 obs_source_t target = {.width=64, .height=48};
 obs_source_t source = {.enabled=true, .target=&target};
 struct virtual_cam_filter_context *c = virtual_cam_filter_source_create(NULL, &source);

 /* First tick/render must use exactly the stream attached to the output. */
 tick(c);
 assert(opens == 1 && starts == 1 && c->width == 64 && c->height == 48);
 video_t *first = c->video_output;
 render(c);
 assert(locks == 1 && opens == 1 && closes == 0 && first == c->video_output);
 assert(c->video_data && c->stagesurface->mapped);

 /* Render may see a resize before tick, but must not touch the old buffers. */
 target.width = 80;
 render(c);
 assert(locks == 1 && renders == 1);
 tick(c);
 assert(stops == 1 && closes == 0 && c->output_stopping);
 for (int i=0; i<3; i++) { tick(c); render(c); }
 assert(stops == 1 && closes == 0 && opens == 1 && locks == 1);
 finish_stop(output);
 tick(c);
 assert(closes == 1 && opens == 2 && starts == 2);
 assert(c->width == 80 && !c->video_data && !c->stagesurface);
 assert(unmaps == 1 && destroyed_surfaces == 1);
 render(c);
 assert(locks == 2);

 /* Rapid disable/re-enable still waits for the pending stop. */
 source.enabled = false;
 tick(c);
 source.enabled = true;
 tick(c);
 assert(c->output_stopping && starts == 2);
 finish_stop(output);
 tick(c);
 assert(starts == 3 && opens == 2 && closes == 1);

 /* Staying disabled closes only once, after raw capture disconnects. */
 source.enabled = false;
 tick(c); render(c);
 assert(closes == 1 && locks == 2);
 finish_stop(output);
 tick(c); tick(c);
 assert(closes == 2 && !c->video_output && !c->video_data && !c->width);

 /* Failed opens and starts are retried without claiming an active stream. */
 source.enabled = true;
 fail_open = true;
 tick(c); render(c);
 assert(!c->video_output && !c->output_active && !c->width);
 fail_open = false; fail_start = true;
 tick(c); render(c);
 assert(c->video_output && !c->output_active && locks == 2);
 fail_start = false;
 tick(c); render(c);
 assert(c->output_active && locks == 3);

 /* Zero-sized or missing targets stop capture; they are never opened. */
 target.height = 0;
 tick(c); render(c);
 assert(c->output_stopping && locks == 3);
 finish_stop(output); tick(c);
 unsigned previous_opens = opens;
 tick(c); render(c);
 source.target = NULL;
 tick(c); render(c);
 assert(opens == previous_opens && !c->video_output);
 source.target = &target; target.height = 48;
 tick(c); render(c);

 /* Destroy while a stop is pending must release output before its stream. */
 target.width = 96;
 tick(c);
 assert(c->output_stopping);
 virtual_cam_filter_source_destroy(c);
 assert(!output && opens == closes);

 /* Destruction of an actively capturing output also waits for disconnect. */
 c = virtual_cam_filter_source_create(NULL, &source);
 tick(c); render(c);
 assert(c->output_active);
 virtual_cam_filter_source_destroy(c);
 assert(!output && opens == closes);

 /* Also cover destruction without any initialized stream/surface. */
 source.target = NULL;
 c = virtual_cam_filter_source_create(NULL, &source);
 tick(c);
 virtual_cam_filter_source_destroy(c);
 assert(graphics_depth == 0);
 puts("lifecycle regression tests passed");
 return 0;
}
