#include <stdbool.h>
#include <stdint.h>

#include <dragonruby.h>

/* ReSharper disable CppLocalVariableMayBeConst */
static drb_api_t *drb_api;

struct image {
	int16_t width;
	int16_t height;
	uint32_t *pixels;
};

struct point {
	int16_t x;
	int16_t y;
};

struct curve {
	struct point points[4];
	uint32_t color;
	uint16_t line_thickness;
	uint16_t segments;
};

struct line {
	struct point points[2];
	uint32_t color;
	uint16_t thickness;
};

struct polygon {
	const struct point *points;
	size_t count;
	uint32_t color;
};

struct renderer {
	mrb_value grid;
	mrb_sym w_px;
	mrb_sym h_px;
	int16_t width;
	int16_t height;
	bool resized;
};

static const uint32_t CLEAR_VALUE = 0x00000000;
static const uint32_t BLACK_VALUE = 0xFF000000;
static const uint32_t WHITE_VALUE = 0xFFFFFFFF;
static const uint32_t RED_VALUE = 0xFF0000FF;
static const uint32_t GREEN_VALUE = 0xFF00FF00;
static const uint32_t BLUE_VALUE = 0xFFFF0000;

static void
image_clear(struct image *img)
{
	/* ReSharper disable once CppRedundantCastExpression */
	static const int clear_value = (int)CLEAR_VALUE;
	memset(img->pixels, clear_value, img->width * img->height);
}

static inline void
image_set_pixel(struct image *self, const int16_t x, const int16_t y,
    const uint32_t color)
{
	if (x < 0 || x >= self->width || y < 0 || y >= self->height) return;
	self->pixels[y * self->width + x] = color;
}

static void
image_resize(struct image *self, mrb_state *mrb,
    const struct renderer *renderer)
{
	self->width = renderer->width;
	self->height = renderer->height;
	const size_t new_size = self->width * self->height * sizeof(uint32_t);
	self->pixels = drb_api->mrb_realloc(mrb, self->pixels, new_size);
}

static void
renderer_update_size(struct renderer *self, mrb_state *mrb)
{
	const int32_t old_width = self->width;
	const int32_t old_height = self->height;
	const mrb_value w_val
	    = drb_api->mrb_funcall_argv(mrb, self->grid, self->w_px, 0, NULL);
	self->width = (int16_t)mrb_fixnum(w_val);
	const mrb_value h_val
	    = drb_api->mrb_funcall_argv(mrb, self->grid, self->h_px, 0, NULL);
	self->height = (int16_t)mrb_fixnum(h_val);
	const bool width_changed = old_width == self->width;
	const bool height_changed = old_height == self->height;
	self->resized = width_changed || height_changed;
}

static void
renderer_init(mrb_state *mrb, struct renderer *renderer)
{
	renderer->h_px = drb_api->mrb_intern_cstr(mrb, "h_px");
	renderer->w_px = drb_api->mrb_intern_cstr(mrb, "w_px");
	struct RClass *grid_cls = drb_api->mrb_class_get(mrb, "Grid");
	renderer->grid = drb_api->mrb_obj_value(grid_cls);
	renderer_update_size(renderer, mrb);
	renderer->resized = false;
}

static inline bool
undef_or_nil_p(const mrb_value v)
{
	return mrb_undef_p(v) || mrb_nil_p(v);
}

static struct RClass *
drb_argument_error(mrb_state *mrb)
{
	const mrb_sym cls = drb_api->mrb_intern_cstr(mrb, "ArgumentError");
	return drb_api->mrb_exc_get_id(mrb, cls);
}

static uint32_t
read_color(mrb_state *mrb, const mrb_value value)
{
	if (undef_or_nil_p(value)) return BLACK_VALUE;
	if (mrb_integer_p(value)) return mrb_fixnum(value);
	if (!mrb_symbol_p(value)) {
		drb_api->mrb_raise(mrb, drb_argument_error(mrb),
		    "color keyword argument must be an integer or symbol");
	}
	const char *str = drb_api->mrb_sym_name(mrb, mrb_symbol(value));
	if (strcmp(str, "clear") == 0) return CLEAR_VALUE;
	if (strcmp(str, "black") == 0) return BLACK_VALUE;
	if (strcmp(str, "white") == 0) return WHITE_VALUE;
	if (strcmp(str, "red") == 0) return RED_VALUE;
	if (strcmp(str, "green") == 0) return GREEN_VALUE;
	if (strcmp(str, "blue") == 0) return BLUE_VALUE;
	drb_api->mrb_raise(mrb, drb_argument_error(mrb),
	    "unexpected color symbol, expected one of: clear, black, white, "
	    "red, green, blue");
	/* unreachable */
	return BLACK_VALUE;
}

static void
image_draw_thin_line(struct image *self, mrb_state *mrb, const struct line *cmd)
{
	(void)mrb;
	int16_t x0 = cmd->points[0].x;
	int16_t y0 = cmd->points[0].y;
	int16_t x1 = cmd->points[1].x;
	int16_t y1 = cmd->points[1].y;
	int16_t dy = y1 - y0;
	int16_t dx = x1 - x0;
	const uint32_t color = cmd->color;

	if (dy == 0) {
		if (dx == 0) {
			image_set_pixel(self, x0, y0, color);
			return;
		}
		if (x0 > x1) {
			const int16_t tmp = x0;
			x0 = x1;
			x1 = tmp;
		}
		for (int16_t x = x0; x <= x1; ++x)
			image_set_pixel(self, x, y0, color);
		return;
	}

	int16_t step_y = 1, step_x = 1;
	if (dy < 0) {
		dy = -dy;
		step_y = -1;
	}
	if (dx < 0) {
		dx = -dx;
		step_x = -1;
	}
	dy <<= 1;
	dx <<= 1;
	image_set_pixel(self, x0, y0, color);
	if (dx > dy) {
		int16_t fraction = dy - (dx >> 1);
		while (x0 < x1) {
			if (fraction >= 0) {
				y0 += step_y;
				fraction -= dx;
			}
			x0 += step_x;
			fraction += dy;
			image_set_pixel(self, x0, y0, color);
		}
	} else {
		int16_t fraction = dx - (dy >> 1);
		while (y0 < y1) {
			if (fraction >= 0) {
				x0 += step_x;
				fraction -= dy;
			}
			y0 += step_y;
			fraction += dx;
			image_set_pixel(self, x0, y0, color);
		}
	}
}

static inline int16_t
round_i16(int16_t x)
{
	return (int16_t)((float)x + 0.5f);
}

static inline void
count_node(const struct point *pnts, int16_t *nodes_x, size_t *nodes,
    int16_t pixelY, size_t i, size_t j)
{
	if ((pnts[i].y >= pixelY || pnts[j].y < pixelY)
	    && (pnts[j].y >= pixelY || pnts[i].y < pixelY))
		return;
	nodes_x[(*nodes)++] = (int16_t)((float)pnts[i].x
	    + ((float)pixelY - (float)pnts[i].y)
	        / ((float)pnts[j].y - (float)pnts[i].y)
	        * ((float)pnts[j].x - (float)pnts[i].x));
}

struct polygon_scanline_info {
	int16_t *nodes_x;
	int16_t left;
	int16_t right;
	int16_t bottom;
	int16_t top;
};

static void
image_apply_scanline(struct image *self, const struct polygon *cmd,
    const struct polygon_scanline_info *scanline, const int16_t y)
{
	const struct point *points = cmd->points;
	size_t nodes = 0;
	const size_t count = cmd->count;
	int16_t *nodes_x = scanline->nodes_x;
	int16_t left = scanline->left;
	int16_t right = scanline->right;
	for (size_t i = 0, j = count - 1; i < count; ++i) {
		count_node(points, nodes_x, &nodes, y, i, j);
		j = i;
	}

	for (size_t i = 0; i < nodes - 1;) {
		if (nodes_x[i] <= nodes_x[i + 1]) {
			++i;
			continue;
		}
		int16_t swap = nodes_x[i];
		nodes_x[i] = nodes_x[i + 1];
		nodes_x[i + 1] = swap;
		if (i != 0) --i;
	}
	for (size_t i = 0; i < nodes; i += 2) {
		if (nodes_x[i] >= right) break;
		if (nodes_x[i + 1] <= left) continue;
		if (nodes_x[i] < left) nodes_x[i] = left;
		if (nodes_x[i + 1] > right) nodes_x[i + 1] = right;
		for (int16_t x = nodes_x[i]; x < nodes_x[i + 1]; ++x)
			image_set_pixel(self, x, y, cmd->color);
	}
}

static void
image_draw_polygon(struct image *self, mrb_state *mrb,
    const struct polygon *cmd)
{
#define SMALL_BUFFER_SIZE 64
	/* modification of Darel Rex Finley's public domain polygon fill
	 * algorithm to accept any number of points*/
	const size_t count = cmd->count;
	if (count == 0) return;
	int16_t nodes_x_small_buffer[SMALL_BUFFER_SIZE] = {};
	const size_t nodes_x_size = count * sizeof(int16_t);
	int16_t *nodes_x = NULL;
	if (count <= SMALL_BUFFER_SIZE) nodes_x = nodes_x_small_buffer;
	else nodes_x = drb_api->mrb_malloc(mrb, nodes_x_size);
	int16_t left = INT16_MAX, right = 0, top = INT16_MAX, bottom = 0;
	for (size_t i = 0; i < count; ++i) {
		if (left > cmd->points[i].x) left = cmd->points[i].x;
		if (right < cmd->points[i].x) right = cmd->points[i].x;
		if (bottom > cmd->points[i].y) bottom = cmd->points[i].y;
		if (top < cmd->points[i].y) top = cmd->points[i].y;
	}

	const struct point *pnts = cmd->points;
	struct polygon_scanline_info scanline = {
		.nodes_x = nodes_x,
		.left = left,
		.right = right,
		.bottom = top,
		.top = bottom,
	};
	for (int16_t y = top; y < bottom; ++y)
		image_apply_scanline(self, cmd, &scanline, y);

	if (nodes_x != nodes_x_small_buffer) drb_api->mrb_free(mrb, nodes_x);
}

static void
image_draw_line(struct image *self, mrb_state *mrb, const struct line *cmd)
{
	if (cmd->thickness == 1) {
		image_draw_thin_line(self, mrb, cmd);
		return;
	}
	const int16_t dx = cmd->points[1].x - cmd->points[0].x;
	const int16_t dy = cmd->points[1].y - cmd->points[0].y;
	const float theta = drb_api->SDL_atan2f((float)dy, (float)dx);
	const float half_thickness = (float)cmd->thickness / 2.0f;
	const float sin_theta = drb_api->SDL_sinf(theta);
	const float cos_theta = drb_api->SDL_cosf(theta);
	const float offset_x = half_thickness * sin_theta;
	const float offset_y = half_thickness * cos_theta;
	const struct point points[4] = {
		(struct point) {
		    .x = round_i16(cmd->points[0].x - offset_x),
		    .y = round_i16(cmd->points[0].y + offset_y),
		},
		(struct point) {
		    .x = round_i16(cmd->points[0].x + offset_x),
		    .y = round_i16(cmd->points[0].y - offset_y),
		},
		(struct point) {
		    .x = round_i16(cmd->points[1].x + offset_x),
		    .y = round_i16(cmd->points[1].y - offset_y),
		},
		(struct point) {
		    .x = round_i16(cmd->points[1].x - offset_x),
		    .y = round_i16(cmd->points[1].y + offset_y),
		},
	};
	const struct polygon polygon = {
		.points = points,
		.count = 4,
	};
	image_draw_polygon(self, mrb, &polygon);
}

static void
image_draw_curve(struct image *self, mrb_state *mrb, const struct curve *cmd)
{
	const struct point p1 = cmd->points[0];
	const struct point p2 = cmd->points[1];
	const struct point p3 = cmd->points[2];
	const struct point p4 = cmd->points[3];
	struct point last = p1;
	const float t_step = 1.0f / (float)cmd->segments;
	for (int i = 1; i <= cmd->segments; i++) {
		const float t = t_step * (float)i;
		const float u = 1.0f - t;
		const float w1 = u * u * u;
		const float w2 = 3 * u * u * t;
		const float w3 = 3 * u * t * t;
		const float w4 = t * t * t;
		const float x = w1 * (float)p1.x + w2 * (float)p2.x
		    + w3 * (float)p3.x + w4 * (float)p4.x;
		const float y = w1 * (float)p1.y + w2 * (float)p2.y
		    + w3 * (float)p3.y + w4 * (float)p4.y;
		const struct point pn = {
			.x = (int16_t)x,
			.y = (int16_t)y,
		};
		const struct line line_cmd = {
			.points = { last, pn },
			.thickness = cmd->line_thickness,
		};
		image_draw_line(self, mrb, &line_cmd);
		last = pn;
	}
}

static struct curve
read_curve_args(mrb_state *mrb, mrb_sym *key)
{
	enum {
		POINTS,
		THICKNESS,
		COLOR,
		SEGMENTS,
		KWARGS_COUNT,
	};
	const mrb_sym kwargs_table[KWARGS_COUNT] = {
		[POINTS] = drb_api->mrb_intern_cstr(mrb, "points"),
		[THICKNESS] = drb_api->mrb_intern_cstr(mrb, "line_thickness"),
		[COLOR] = drb_api->mrb_intern_cstr(mrb, "color"),
		[SEGMENTS] = drb_api->mrb_intern_cstr(mrb, "segments"),
	};
	mrb_value values[KWARGS_COUNT] = {
		mrb_nil_value(),
		mrb_undef_value(),
		mrb_undef_value(),
		mrb_undef_value(),
	};
	mrb_kwargs kwargs = {
		.num = KWARGS_COUNT,
		.required = 1,
		.table = kwargs_table,
		.values = values,
		.rest = NULL,
	};

	drb_api->mrb_get_args(mrb, "n:", key, &kwargs);
	if (undef_or_nil_p(values[POINTS])) {
		drb_api->mrb_raise(mrb, drb_argument_error(mrb),
		    "points keyword argument is required");
	}
	if (!mrb_array_p(values[POINTS])) {
		drb_api->mrb_raise(mrb, drb_argument_error(mrb),
		    "points keyword argument must be an array");
	}
	if (undef_or_nil_p(values[THICKNESS]))
		values[THICKNESS] = drb_api->mrb_int_value(mrb, 1);
	if (undef_or_nil_p(values[SEGMENTS]))
		values[SEGMENTS] = drb_api->mrb_int_value(mrb, 22);
	struct curve self = { 0 };
	self.segments = mrb_fixnum(values[SEGMENTS]);
	self.color = read_color(mrb, values[COLOR]);
	return self;
}

#define IMAGE_SYM "$_ext_image_raw_pointer"

static void
img_delete(mrb_state *mrb)
{
	const mrb_sym img_gv = drb_api->mrb_intern_cstr(mrb, IMAGE_SYM);
	const mrb_value img_val = drb_api->mrb_gv_get(mrb, img_gv);
	if (mrb_nil_p(img_val)) return;
	const uintptr_t img_ptr = (uintptr_t)mrb_fixnum(img_val);
	struct image *img = (struct image *)img_ptr;
	drb_api->mrb_free(mrb, img->pixels);
}

DRB_FFI mrb_value
update_curve(mrb_state *mrb, const mrb_value value)
{
	static struct image img = {};
	static struct renderer renderer = {};
	static bool initialized = false;
	(void)value;
	if (!initialized) {
		renderer_init(mrb, &renderer);
		renderer.resized = false;
		img.width = renderer.width;
		img.height = renderer.height;
		const size_t size = img.width * img.height * sizeof(uint32_t);
		drb_api->drb_log_write("update_curve", 2, "initializing image");
		img.pixels = drb_api->mrb_malloc(mrb, size);
		// image_clear(&img);
		const uintptr_t img_ptr = (uintptr_t)img.pixels;
		const mrb_sym img_gv = drb_api->mrb_intern_cstr(mrb, IMAGE_SYM);
		const mrb_value ptr_val = mrb_fixnum_value(img_ptr);
		drb_api->mrb_gv_set(mrb, img_gv, ptr_val);
		drb_api->mrb_state_atexit(mrb, img_delete);
	}
	renderer_update_size(&renderer, mrb);
	initialized = true;
	mrb_sym key_sym;
	const struct curve curve_cmd = read_curve_args(mrb, &key_sym);
	const char *key = drb_api->mrb_sym_name(mrb, key_sym);
	if (renderer.resized) image_resize(&img, mrb, &renderer);
	image_clear(&img);
	image_draw_curve(&img, mrb, &curve_cmd);
	drb_api->drb_upload_pixel_array(key, img.width, img.height, img.pixels);
	return drb_api->mrb_nil_value();
}

DRB_FFI
mrb_value
update_scanner_texture(mrb_state *state, mrb_value value)
{
#define dimension 10

	static int pos = 0;
	static int posinc = 1;

	// Set up our "scanner" pixel array and fill it with black pixels.

	// You could make this faster by making this array static (which will
	//  initialize it all to zero at startup), and then blanking the
	//  previous line and drawing the next, and not touching the rest.
	uint32_t pixels[dimension * dimension];
	for (int i = 0; i < (dimension * dimension); i++) {
		pixels[i] = 0xFF000000; // full alpha, full black
	}

	// Draw a green line that bounces up and down the sprite.
	uint32_t *line = pixels + (pos * dimension);
	for (int i = 0; i < dimension; i++) {
		line[i] = 0xFF00FF00; // full alpha, full green
	}

	// Adjust position for next frame.
	pos += posinc;
	if ((posinc > 0) && (pos >= dimension)) {
		posinc = -1;
		pos = dimension - 1;
	} else if ((posinc < 0) && (pos < 0)) {
		posinc = 1;
		pos = 1;
	}

	// Send it to the renderer to create/update a sprite.
	drb_api->drb_upload_pixel_array("scanner", dimension, dimension,
	    pixels);
	return mrb_nil_value();
}

DRB_FFI_EXPORT
void
drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api)
{
	drb_api = api;
	struct RClass *FFI = drb_api->mrb_module_get(state, "FFI");
	struct RClass *module
	    = drb_api->mrb_define_module_under(state, FFI, "CExt");
	drb_api->mrb_define_module_function(state, module,
	    "update_scanner_texture", update_scanner_texture, MRB_ARGS_REQ(0));
	drb_api->mrb_define_module_function(state, module, "update_curve",
	    update_curve, MRB_ARGS_KEY(1, 0));
}
