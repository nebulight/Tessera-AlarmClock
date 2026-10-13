"""Alarm clock: a daily alarm that beeps on the screen's speaker until someone taps the screen.

The beep is sounds/alarm.wav beside this file (a WAV of 16-bit samples), built in by smart_display.sound() and played by
the core's sound player (plugin_sound.h). register_plugin() takes the id, version and the screen's texts from the
manifest and translations/ beside this folder.
"""
from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number, smart_display, speaker, switch
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display", "speaker"]

CONF_SPEAKER = "speaker"
CONF_ALARM_ON = "alarm_on"
CONF_HOUR = "hour"
CONF_MINUTE = "minute"
ALARM = Path(__file__).parent / "sounds" / "alarm.wav"

alarm_clock_ns = cg.esphome_ns.namespace("alarm_clock")
AlarmClock = alarm_clock_ns.class_("AlarmClock", cg.Component)


def check_alarm(config):
    smart_display.read_sound(ALARM)
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AlarmClock),
            cv.Required(CONF_SPEAKER): cv.use_id(speaker.Speaker),
            cv.Required(CONF_ALARM_ON): cv.use_id(switch.Switch),
            cv.Required(CONF_HOUR): cv.use_id(number.Number),
            cv.Required(CONF_MINUTE): cv.use_id(number.Number),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    check_alarm,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await smart_display.register_plugin(var, __file__)
    cg.add(var.set_speaker(await cg.get_variable(config[CONF_SPEAKER])))
    cg.add(var.set_alarm_on(await cg.get_variable(config[CONF_ALARM_ON])))
    cg.add(var.set_hour(await cg.get_variable(config[CONF_HOUR])))
    cg.add(var.set_minute(await cg.get_variable(config[CONF_MINUTE])))
    cg.add(var.set_alarm(smart_display.sound(config[CONF_ID], "alarm", ALARM)))
