/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_STATE_H
#define EULER_UTIL_STATE_H

#include <thread>

#include "euler/util/object.h"
#include "euler/util/ruby_state.h"

#ifdef EULER_GUI
namespace euler::gui {
class Context;
}
#endif

#ifdef EULER_GRAPHICS
namespace euler::graphics {
class Image;
class ImageLoader;
class RenderTarget;
class Color;
class Window;
}
#endif

namespace euler::util {
class Error;
class Logger;
class Storage;
class Version;
struct Config;

class State : public Object {
public:
	~State() override;
	State();
	typedef uint64_t tick_t;
	typedef decltype(std::thread::hardware_concurrency()) nthread_t;

	enum class Runtime {
		Native,
		DragonRuby,
	};

	enum class Phase {
		Load,
		Input,
		Update,
		Draw,
		Quit,
	};

	struct Modules {
		RClass *mod = nullptr;
		struct {
			RClass *mod = nullptr;
			RClass *state = nullptr;
#ifdef EULER_NATIVE
			RClass *window = nullptr;
#endif
		} app;

#ifdef EULER_GRAPHICS
		struct {
			RClass *mod = nullptr;
			RClass *render_target = nullptr;
			RClass *font = nullptr;
			RClass *image = nullptr;
			RClass *window = nullptr;
			RClass *color = nullptr;
		} graphics;
#endif

#ifdef EULER_GUI
		struct {
			RClass *mod = nullptr;
			RClass *button = nullptr;
			RClass *checkbox = nullptr;
			RClass *color_picker = nullptr;
			RClass *dropdown = nullptr;
			RClass *element = nullptr;
			RClass *knob = nullptr;
			RClass *progress_bar = nullptr;
			RClass *radio_button = nullptr;
			RClass *row = nullptr;
			RClass *selectable = nullptr;
			RClass *slider = nullptr;
			RClass *style = nullptr;
			RClass *text = nullptr;
			RClass *user_interface = nullptr;
			RClass *widget = nullptr;
			RClass *window = nullptr;
		} gui;
#endif

#ifdef EULER_NET
		struct {
			RClass *mod = nullptr;
			RClass *sockets = nullptr;
		} net;
#endif

#ifdef EULER_INPUT
		struct {
			RClass *event = nullptr;
			RClass *display_event = nullptr;
			RClass *window_event = nullptr;
			RClass *keyboard_device_event = nullptr;
			RClass *keyboard_event = nullptr;
			RClass *text_editing_event = nullptr;
			RClass *text_editing_candidates_event = nullptr;
			RClass *text_input_event = nullptr;
			RClass *mouse_device_event = nullptr;
			RClass *mouse_motion_event = nullptr;
			RClass *mouse_button_event = nullptr;
			RClass *mouse_wheel_event = nullptr;
			RClass *joystick_device_event = nullptr;
			RClass *joystick_axis_motion_event = nullptr;
			RClass *joystick_ball_motion_event = nullptr;
			RClass *joystick_hat_motion_event = nullptr;
			RClass *joystick_button_event = nullptr;
			RClass *joystick_battery_updated_event = nullptr;
			RClass *gamepad_device_event = nullptr;
			RClass *gamepad_axis_motion_event = nullptr;
			RClass *gamepad_button_event = nullptr;
			RClass *gamepad_touchpad_event = nullptr;
			RClass *gamepad_sensor_event = nullptr;
			RClass *audio_device_event = nullptr;
			RClass *camera_device_event = nullptr;
			RClass *sensor_event = nullptr;
			RClass *quit_event = nullptr;
			RClass *user_event = nullptr;
			RClass *touch_finger_event = nullptr;
			RClass *pen_proximity_event = nullptr;
			RClass *pen_touch_event = nullptr;
			RClass *pen_motion_event = nullptr;
			RClass *pen_button_event = nullptr;
			RClass *pen_axis_event = nullptr;
			RClass *render_event = nullptr;
			RClass *drop_event = nullptr;
			RClass *clipboard_event = nullptr;
		} input;
#endif

#ifdef EULER_MATH
		struct {
			RClass *mod = nullptr;
			RClass *nonscalar = nullptr;
			RClass *cube = nullptr;
			RClass *matrix = nullptr;
			RClass *row_vector = nullptr;
			RClass *running_stat = nullptr;
			RClass *sparse_matrix = nullptr;
			RClass *sparse_solve_factorizer = nullptr;
			RClass *size = nullptr;
			RClass *vector = nullptr;
		} math;
#endif

#ifdef EULER_PHYSICS
		struct {
			RClass *mod = nullptr;
			RClass *body = nullptr;
			RClass *chain = nullptr;
			RClass *contact = nullptr;
			RClass *distance_joint = nullptr;
			RClass *filter_joint = nullptr;
			RClass *joint = nullptr;
			RClass *motor_joint = nullptr;
			RClass *prismatic_joint = nullptr;
			RClass *revolute_joint = nullptr;
			RClass *shape = nullptr;
			RClass *weld_joint = nullptr;
			RClass *wheel_joint = nullptr;
			RClass *world = nullptr;
			RClass *capsule = nullptr;
			RClass *circle = nullptr;
			RClass *polygon = nullptr;
			RClass *segment = nullptr;
		} physics;
#endif

		struct {
			RClass *mod = nullptr;
			RClass *color = nullptr;
			RClass *version = nullptr;
			RClass *logger = nullptr;
			RClass *file = nullptr;
			RClass *storage = nullptr;
		} util;
	};

#ifdef EULER_GUI
	[[nodiscard]] virtual Reference<gui::Context> gui() const = 0;
#endif

#ifdef EULER_GRAPHICS
	[[nodiscard]] virtual Reference<graphics::ImageLoader> image_loader()
	    = 0;
#endif

#ifdef EULER_NATIVE
	[[nodiscard]] virtual std::optional<uint32_t> preferred_gpu() const = 0;
#endif

	[[nodiscard]] virtual Runtime runtime() const = 0;
	// [[nodiscard]] virtual Reference<RubyState> mrb() const = 0;
	[[nodiscard]] virtual const RubyState &rb() const = 0;
	[[nodiscard]] virtual RubyState &rb() = 0;
	[[nodiscard]] virtual RClass *object_class() const = 0;
	[[nodiscard]] virtual Reference<Logger> log() const = 0;
	[[nodiscard]] virtual nthread_t available_threads() const = 0;
	[[nodiscard]] virtual tick_t ticks() const = 0;
	[[nodiscard]] virtual tick_t last_tick() const = 0;
	[[nodiscard]] virtual float fps() const = 0;
	[[nodiscard]] virtual float dt() const = 0;
	[[nodiscard]] virtual tick_t total_ticks() const = 0;
	[[nodiscard]] virtual mrb_value gv_state() = 0;
	[[nodiscard]] static Reference<State> get(const mrb_state *mrb);
	[[nodiscard]] virtual Phase phase() const = 0;
	virtual void set_phase(Phase phase) = 0;
	[[nodiscard]] virtual const std::string &progname() const = 0;
	[[nodiscard]] virtual const std::string &title() const = 0;
	virtual bool initialize(std::string_view progname, const Config &config)
	    = 0;
	virtual const Config &config() const = 0;
	virtual void tick() = 0;
	virtual mrb_value self_value() = 0;
	[[nodiscard]] virtual const Modules &modules() const = 0;
	[[nodiscard]] virtual Modules &modules() = 0;
	[[nodiscard]] virtual void *unwrap(mrb_value value,
	    const mrb_data_type *type)
	    = 0;
	virtual Version app_version() const = 0;
	virtual Version engine_version() const = 0;

	// virtual void draw_at(const Reference<Image> &, Vec2 position, Vec2
	// scale) = 0;
	// virtual mrb_value image_to_ruby(const Reference<Image> &) = 0;
	// virtual Reference<Image> image_from_ruby(mrb_value) = 0;

	template <typename T>
	[[nodiscard]] Reference<T>
	unwrap(mrb_value value)
	{
		auto ptr = unwrap(value, &T::TYPE);
		if (ptr == nullptr) return Reference<T>(nullptr);
		return Reference<T>::unwrap(ptr);
	}

	template <typename T>
	[[nodiscard]] mrb_value
	wrap(Reference<T> &obj)
	{
		if (obj == nullptr) return mrb_nil_value();
		auto self = Reference(const_cast<State *>(this));
		return obj.wrap(rb().mrb(), T::fetch_class(self), &T::TYPE);
	}

protected:
	virtual const mrb_data_type *data_type() const = 0;
};

#define EULER_SYM_LIT(S)                                                       \
	(::euler::util::State::get(mrb)->rb().intern_static(S, sizeof(S) - 1))

#define EULER_SYM(SYM) EULER_SYM_LIT(#SYM)

#define EULER_IVSYM(SYM) EULER_SYM_LIT("@" #SYM)

#define EULER_SYM_VAL(SYM) (mrb_symbol_value(EULER_SYM(SYM)))

} /* namespace euler::util */

#endif /* EULER_UTIL_STATE_H */
