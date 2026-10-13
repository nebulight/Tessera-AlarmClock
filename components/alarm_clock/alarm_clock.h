#pragma once

#include <cmath>
#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/speaker/speaker.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/smart_display/plugin_api.h"
#include "esphome/components/smart_display/plugin_sound.h"

namespace esphome::alarm_clock {

// The plugin: one daily alarm. The alarm is checked here, in on_interval(), which runs every 250 ms whatever page the
// screen shows; the tile only draws it. When it goes off, the beep repeats until a tap on the screen, or two minutes.
class AlarmClock : public Component, public tessera::Plugin {
 public:
  float get_setup_priority() const override { return setup_priority::LATE; }
  void setup() override;
  void loop() override { sounds_.loop(); }
  void on_interval(uint32_t now_ms) override;
  void on_touch() override;
  bool settings(tessera::SettingsPage &page) override;
  void before_update() override { stop_ringing(); }

  void set_speaker(speaker::Speaker *s) { sounds_.set_speaker(s); }
  void set_alarm_on(switch_::Switch *s) { alarm_on_ = s; }
  void set_hour(number::Number *n) { hour_ = n; }
  void set_minute(number::Number *n) { minute_ = n; }
  void set_alarm(tessera::Sound s) { alarm_ = s; }

  bool enabled() const { return alarm_on_ != nullptr && alarm_on_->state; }
  bool ringing() const { return ringing_; }
  int hour() const { return value_of(hour_, 7, 23); }
  int minute() const { return value_of(minute_, 0, 59); }

 protected:
  static int value_of(number::Number *n, int fallback, int highest) {
    if (n == nullptr || std::isnan(n->state)) return fallback;
    const int v = static_cast<int>(n->state);
    return v < 0 ? 0 : v > highest ? highest : v;
  }
  static void set_number(number::Number *n, int v) {
    if (n == nullptr) return;
    auto call = n->make_call();
    call.set_value(static_cast<float>(v));
    call.perform();
  }
  void stop_ringing();

  switch_::Switch *alarm_on_{};
  number::Number *hour_{}, *minute_{};
  tessera::Sound alarm_;
  tessera::SoundPlayer sounds_;
  bool ringing_{false};
  uint32_t ring_started_ms_{0};
  uint32_t last_fired_minute_{0};  // the minute (seconds since 1970 / 60) the alarm last went off in
};

}  // namespace esphome::alarm_clock
