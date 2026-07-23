# Changelog

## Unreleased

- Restructured project into modules: Core, Camera, Input, UI, Zoom.
- Replaced ButtonInfo-based zoom control with a hand-drawn zone +
  direct touch tracking, enabling true single-finger hold+drag (an
  earlier ButtonInfo-based version only detected drag from a second,
  separate finger).
- FOV override via `CameraAPI::tryGetFOV` hook confirmed working
  on-device.
- Currently blocked on a SIGSEGV inside `pl::modmenu::registerModule`
  - see README "Open problem" section.
