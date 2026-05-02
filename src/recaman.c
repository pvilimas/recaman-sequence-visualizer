#include "recaman.h"

void init() {
	APP.window_size = (Vector2){1920, 1080};
	APP.window_title = "recaman sequence visualizer";
	APP.term_start = 1;
	APP.term_end   = 100;
	APP.x_axis_start_px = (Vector2){30., APP.window_size.y / 2};
	APP.x_axis_end_px = (Vector2){APP.window_size.x-30, APP.window_size.y / 2};
	APP.theme = 1;
	APP.show_numbers = false;
	APP.show_fps = false;
	APP.draw_x_marks = false;
	populate_sequence();
	compute_value_range();

	APP.display_value_min = (float)APP.x_axis_value_min;
	APP.display_value_max = (float)APP.x_axis_value_max;

	for (u64 i = APP.term_start; i <= APP.term_end; i++)
	    APP.sequence[i].alpha = 1.0f;
	
	// debug_print_sequence();
	SetTraceLogLevel(LOG_WARNING);
	SetTargetFPS(60);
	InitWindow(APP.window_size.x, APP.window_size.y, APP.window_title);
}

void populate_sequence() {
    u64 n = 100000;
    APP.seq_len = n + 1;
    APP.sequence = malloc(sizeof(SequenceTerm) * APP.seq_len);
    u64* a = malloc(sizeof(u64) * APP.seq_len);
    bool* seen = calloc(APP.seq_len * 10, sizeof(bool));
    a[0] = 0;
    seen[0] = true;
    for (u64 i = 1; i <= n; i++) {
        u64 back = a[i-1] - i;
        if (a[i-1] >= i && !seen[back]) {
            a[i] = back;
        } else {
            a[i] = a[i-1] + i;
        }
        seen[a[i]] = true;
    }
    for (u64 i = 0; i <= n; i++) {
        APP.sequence[i].x_value    = a[i];
        APP.sequence[i].prev_value = (i == 0) ? ST_NONE : a[i-1];
        APP.sequence[i].next_value = (i == n) ? ST_NONE : a[i+1];
        APP.sequence[i].alpha      = 0.0f;
    }
    free(a);
    free(seen);
}

void compute_value_range() {
    u64 mn = UINT64_MAX, mx = 0;
    for (u64 i = APP.term_start; i <= APP.term_end; i++) {
        u64 v = APP.sequence[i].x_value;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
    }
    u64 padding = (mx - mn) / 4;
    APP.x_axis_value_min = mn > padding ? mn - padding : 0;
    APP.x_axis_value_max = mx + padding;
}

void run_visualization() {

	// scales lerp speed by a constant factor
	float absolute_speed_factor = 0.3f;

	int frames = 0;
	while (!WindowShouldClose() && !IsKeyPressed(KEY_Q)) {
		if (IsKeyPressed(KEY_ONE))  APP.theme = 1;
		if (IsKeyPressed(KEY_TWO))	APP.theme = 2;
		if (IsKeyPressed(KEY_N)) 	APP.show_numbers = !APP.show_numbers;
		if (IsKeyPressed(KEY_F)) 	APP.show_fps = !APP.show_fps;
		
	    float frames_in_window = (float)(APP.term_end - APP.term_start);
		// int step_size = log10_step_size(frames_in_window);
		int step_size = 1;
		
	    APP.term_start += step_size;
	    APP.term_end += step_size;
	    APP.sequence[APP.term_end].alpha = 1.0f;
	    compute_value_range();

	    float decay = powf(0.1f, 2.0f / frames_in_window);
	    for (u64 i = APP.term_start; i < APP.term_end; i++)
	        APP.sequence[i].alpha *= decay;

		float dt = GetFrameTime();
		float stiffness = 8.0f;
		float damping   = 2.0f * sqrtf(stiffness); // critical damping
		
		float f_min = (APP.x_axis_value_min - APP.display_value_min) * stiffness;
		float f_max = (APP.x_axis_value_max - APP.display_value_max) * stiffness;
		
		APP.vel_min += (f_min - APP.vel_min * damping) * dt;
		APP.vel_max += (f_max - APP.vel_max * damping) * dt;
		
		APP.display_value_min += APP.vel_min * dt * absolute_speed_factor;
		APP.display_value_max += APP.vel_max * dt * absolute_speed_factor;
	
		BeginDrawing();
		ClearBackground(BLACK);
		if (APP.theme == 2) {
			DrawRectangle(0, APP.window_size.y/2,
			APP.window_size.x, APP.window_size.y/2,
			WHITE);
		}
		draw_rings();
		if (APP.show_fps) {
			DrawFPS(40, 40);
		}
		// draw_x_axis();
		EndDrawing();
		frames++;
	}
	CloseWindow();
}

void draw_x_axis() {
	DrawLineEx(APP.x_axis_start_px, APP.x_axis_end_px, 2.0, WHITE);
	if (!APP.draw_x_marks) return;
	u64 n_start = APP.term_start;
	u64 n_end   = APP.term_end;
	float x_mark_distance = (float)(APP.x_axis_end_px.x - APP.x_axis_start_px.x)
	                      / (float)(n_end - n_start);
	float mark_height = 10.0f;
	float offset = 0.0f;
	for (u64 n = n_start; n <= n_end; n++) {
		DrawLineEx((Vector2){APP.x_axis_start_px.x + offset,
		                     (APP.window_size.y / 2) - mark_height / 2},
		           (Vector2){APP.x_axis_start_px.x + offset,
		                     (APP.window_size.y / 2) + mark_height / 2}, 2.0, WHITE);
		offset += x_mark_distance;
	}
}

void draw_rings() {
	float cy = APP.window_size.y / 2;
	float visible_range = (float)(APP.x_axis_value_max - APP.x_axis_value_min);
	for (u64 i = APP.term_start; i < APP.term_end; i++) {
		u64 from = APP.sequence[i].x_value;
		u64 to   = APP.sequence[i].next_value;
		if (to == ST_NONE) break;
		float x1 = value_to_px(from < to ? from : to);
		float x2 = value_to_px(from < to ? to : from);
		float radius = (x2 - x1) / 2.0f;
		float cx = (x1 + x2) / 2.0f;
		if (x2 < APP.x_axis_start_px.x || x1 > APP.x_axis_end_px.x) continue;
		// if (radius < 0.5f) continue;
		bool goes_forward = to > from;
		float thickness = 4.0f;
		float median_radius = visible_range / (float)(APP.term_end - APP.term_start);
		u8 alpha;
		if (APP.theme == 2) {
		    alpha = 255;
		} else {
		    alpha = (u8)(255.0f * APP.sequence[i].alpha);
		}
		int num_segments = (int)fminf(200.0f, fmaxf(20.0f, radius * 0.5f));

		float alpha_theme_factor = (APP.theme == 1) ? 0.1f : 1.0f;

		Color c_fwd, c_back, c_fwd_fill, c_back_fill;
		if (APP.theme == 1) {
		    c_fwd       = (Color){119, 125, 167, alpha};
		    c_back      = (Color){50,  50,  75,  alpha};
		    c_fwd_fill  = (Color){119, 125, 167, (u8)(alpha * alpha_theme_factor)};
		    c_back_fill = (Color){40,  40,  62,  (u8)(alpha * alpha_theme_factor)};
		} else {
		    c_fwd       = (Color){255, 255, 255, alpha};
		    c_back      = (Color){0,   0,   0,   alpha};
		    c_fwd_fill  = (Color){255, 255, 255, (u8)(alpha * alpha_theme_factor)};
		    c_back_fill = (Color){0,   0,   0,   (u8)(alpha * alpha_theme_factor)};
		};

		DrawRing((Vector2){cx, cy},
		         radius - thickness/2,
		         radius + thickness/2,
		         goes_forward ? 180 : 0,
		         goes_forward ? 360 : 180,
		         num_segments, goes_forward ? c_fwd : c_back);
		         // num_segments, goes_forward ? c_blue : c_red);

		// // endpoint
		// {
		// 	float ex = goes_forward ? cx + radius : cx - radius;
		// 	float dw = 4.0f;  // half-width horizontal
		// 	float dh = 12.0f; // half-height vertical
		// 	Color dc = (Color){255, 255, 255, alpha};
		// 	
		// 	DrawTriangle(
		// 	    (Vector2){ex,      cy - dh},
		// 	    (Vector2){ex + dw, cy},
		// 	    (Vector2){ex - dw, cy},
		// 	    dc);
		// 	DrawTriangle(
		// 	    (Vector2){ex,      cy + dh},
		// 	    (Vector2){ex - dw, cy},
		// 	    (Vector2){ex + dw, cy},
		// 	    dc);
		// }


		if (APP.theme == 1) {
			BeginBlendMode(BLEND_ALPHA);
		}

		// filled interior
		DrawCircleSector((Vector2){cx, cy}, radius,
		                 goes_forward ? 180 : 0,
		                 goes_forward ? 360 : 180,
		                 num_segments, goes_forward ? c_fwd_fill : c_back_fill);

		// arc outline
		DrawRing((Vector2){cx, cy},
		         radius - thickness/2,
		         radius + thickness/2,
		         goes_forward ? 180 : 0,
		         goes_forward ? 360 : 180,
		         num_segments, goes_forward
		         	? (APP.theme == 1?c_fwd:c_back)
		         	: (APP.theme == 1?c_back:c_fwd));

		if (APP.theme == 1) {
			EndBlendMode();
		}
		
	}

	if (APP.show_numbers) {
		const char* line1 = TextFormat("width = %"PRIu64"",
			APP.term_end - APP.term_start + 1);
		const char* line2 = TextFormat("[%"PRIu64", %"PRIu64"]",
			APP.term_start,
			APP.term_end);
		DrawText(line1, 10, APP.window_size.y - 70, 30,
			APP.theme==1?WHITE:BLACK);
		DrawText(line2, 10, APP.window_size.y - 40, 30,
			APP.theme==1?WHITE:BLACK);
	}
}

int log10_step_size(int value) {
    return (int)pow(10, floor(log10(value)));
}

float value_to_px(u64 value) {
    float scale = (APP.x_axis_end_px.x - APP.x_axis_start_px.x)
                / (APP.display_value_max - APP.display_value_min);
    return APP.x_axis_start_px.x + ((float)value - APP.display_value_min) * scale;
}

void debug_print_sequence() {
	for (u64 i = APP.term_start; i <= APP.term_end; i++) {
		SequenceTerm t = APP.sequence[i];
		printf("{%"PRIu64", %"PRIu64", %"PRIu64"}\n",
		       t.prev_value == ST_NONE ? (u64)-1 : t.prev_value,
		       t.x_value,
		       t.next_value == ST_NONE ? (u64)-1 : t.next_value);
	}
}
