# Alarm clock

A clock tile with one daily alarm. At the time you choose, the screen beeps on its speaker until you tap the screen
(or after two minutes). The tile shows the time and when the alarm goes off.

## What you need

- A screen with a speaker, such as the Waveshare ESP32-P4-86 panel.

## Set up

1. Add the plugin to a screen in Tessera. The screen builds once.
2. In Layout, place the **Alarm clock** tile from the library's Plugins group.
3. Set the alarm: in the plugin's details on the screen's Plugins tab, under **Settings**, choose the hour and the
   minute and turn **Alarm** on. The same three settings are entities of the screen in Home Assistant, so a
   dashboard or an automation can change them too. On the screen itself: hold the top bar, then **Settings**,
   **Plugins**, **Alarm clock**.

## Good to know

- The alarm goes off every day at the time you set, on the screen's own clock and in its time zone. It uses a 24-hour
  time (7 and 30 is half past seven in the morning).
- It rings even when the tile is not on the page the screen shows. The tile flashes only while it is on the glass.
- Any tap on the screen switches the ringing off. An alarm nobody answers stops after two minutes.
- The volume is the screen's own volume.
