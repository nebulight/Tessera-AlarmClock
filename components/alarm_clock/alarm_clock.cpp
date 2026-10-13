#include "alarm_clock.h"
#include <cstdio>
#include <string>

namespace esphome::alarm_clock {

using tessera::Font;
namespace ui = tessera::ui;

static const uint32_t RING_MAX_MS = 2 * 60 * 1000;  // an alarm nobody answers stops by itself after two minutes

// The tile: the time in a big font, and under it when the alarm goes off. While the alarm rings, both flash in the
// accent colour (once a second, only while the tile is on the glass) and the words say "Wake up!".
class ClockTile : public tessera::Tile {
 public:
  explicit ClockTile(AlarmClock *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    width_ = c.width;
    height_ = c.height;
    time_ = ui::label(c.parent, Font::VALUE, theme::INK);
    lv_obj_set_width(time_, width_);
    lv_obj_set_style_text_align(time_, LV_TEXT_ALIGN_CENTER, 0);
    note_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    lv_obj_set_width(note_, width_);
    lv_obj_set_style_text_align(note_, LV_TEXT_ALIGN_CENTER, 0);
  }

  void on_tick(uint32_t epoch) override {
    std::string big, small;
    if (epoch) big = tessera::clock_text(epoch);  // empty until the screen's clock is set
    if (plugin_->ringing()) {
      small = plugin_->text("wake_up");
      flash_ = (epoch & 1) != 0;
    } else if (plugin_->enabled()) {
      char when[8];
      snprintf(when, sizeof(when), "%02d:%02d", plugin_->hour(), plugin_->minute());
      small = tessera::fill(plugin_->text("alarm_at"), "time", std::string(when));
      flash_ = false;
    } else {
      small = plugin_->text("alarm_off");
      flash_ = false;
    }
    // The largest of the screen's fonts the time fits in.
    Font face = Font::VALUE;
    for (Font f : {Font::VALUE, Font::HEADLINE, Font::TITLE}) {
      face = f;
      if (ui::text_width(big, f) <= width_) break;
    }
    ui::set_font(time_, face);
    ui::set_text(time_, big);
    ui::set_text(note_, small);
    paint_colors();
    // The time and the words under it, together in the middle of the card.
    const int th = ui::line_height(face), nh = small.empty() ? 0 : ui::line_height(Font::BODY);
    const int top = (height_ - th - nh) / 2;
    lv_obj_set_pos(time_, 0, top);
    lv_obj_set_height(time_, th);
    lv_obj_set_pos(note_, 0, top + th);
    lv_obj_set_height(note_, nh ? nh : 1);
  }

  // Light and dark: the theme roles have other values now.
  void on_theme() override { paint_colors(); }

 private:
  void paint_colors() {
    ui::set_color(time_, flash_ ? theme::ACCENT : theme::INK);
    ui::set_color(note_, flash_ ? theme::ACCENT : theme::MUTED);
  }

  AlarmClock *plugin_;
  int width_ = 0, height_ = 0;
  bool flash_ = false;
  lv_obj_t *time_{}, *note_{};
};

void AlarmClock::setup() {
  // The id is the tile's id in the manifest; the core makes a ClockTile for every card that shows one.
  add_tile("clock", [this]() { return new ClockTile(this); });
}

void AlarmClock::stop_ringing() {
  ringing_ = false;
  sounds_.stop();
  tessera::refresh();
}

void AlarmClock::on_interval(uint32_t now_ms) {
  if (ringing_) {
    if (now_ms - ring_started_ms_ > RING_MAX_MS) {
      stop_ringing();
    } else if (!sounds_.playing(alarm_)) {
      sounds_.play(alarm_);  // the beep is under a second: start it again until someone answers
    }
    return;
  }
  const uint32_t epoch = tessera::epoch();
  if (!epoch || !enabled()) return;  // the clock is not set yet, or the alarm is off
  const tessera::LocalTime now = tessera::local_time(epoch);
  if (now.hour != hour() || now.minute != minute()) return;
  const uint32_t this_minute = epoch / 60;
  if (this_minute == last_fired_minute_) return;  // it already went off in this minute (and was answered)
  last_fired_minute_ = this_minute;
  ringing_ = true;
  ring_started_ms_ = now_ms;
  sounds_.play(alarm_);
  tessera::refresh();
}

// Any tap the screen takes while the alarm rings switches it off.
void AlarmClock::on_touch() {
  if (ringing_) stop_ringing();
}

bool AlarmClock::settings(tessera::SettingsPage &page) {
  page.icon = "\U000F0020";
  page.toggle(text("setting_alarm"), [this] { return enabled(); },
              [this](bool on) { if (alarm_on_ != nullptr) on ? alarm_on_->turn_on() : alarm_on_->turn_off(); });
  page.number(text("setting_hour"), 0, 23, 1, "", [this] { return hour(); }, [this](int v) { set_number(hour_, v); });
  page.number(text("setting_minute"), 0, 59, 1, "", [this] { return minute(); },
              [this](int v) { set_number(minute_, v); });
  return true;
}

}  // namespace esphome::alarm_clock
