# Changelog

## [Unreleased]

## [V1.10A] - 2026-10-07

### Added
- A title screen comes first: the program's name, its version and its author,
  and "PRESS ANY KEY TO CONTINUE". Any key goes on to the viewer. The version
  is the one in `constants.h`.
- `H` shows a help screen that lists the keys and what they do; any key
  returns to the scene. A reminder ("H KEY=HELP") sits in the top-left corner.
  The help and title screens are drawn in the PicoCalc's own 8x10 font (the
  glyph table from `picocalc/drivers/font-8x10.c`, which the desktop build now
  compiles too, with a stand-in for the Pico SDK header it includes in
  `desktop/stub/`), so they have lower case; the small hand-made font is
  still used for the status numbers. On the PicoCalc the help screen ignores a
  held `H` until it stops repeating, so it does not close straight away. On the
  desktop build `Q` now quits, like on the PicoCalc.

### Changed
- Hidden-line removal is toggled with `L` (it was `H`); the indicator reads
  "L KEY=HLR:1".
- `ESC` and `Q` go back to the title screen instead of leaving the program. On
  the title screen they ask "Leave the program?" and `Y` leaves (to the UF2
  Loader menu on the PicoCalc, closing the window on the desktop); any other
  key stays. The camera is kept when you go back to the viewer. The "Press any
  key" prompts on the title and help screens are cyan, because red is hard to
  read on the PicoCalc screen.
- `p`/`P` pitch, `y`/`Y` yaw and `r`/`R` roll the camera (the capital letter,
  SHIFT + the key, is the other direction), and `z` or `Z` puts it back at its
  starting place. The camera is still on a sphere and faces its centre, and the
  arrow keys still move it over the sphere's surface (LEFT/RIGHT round the
  world's vertical axis, UP/DOWN over the top and bottom, in the world's frame
  so a roll never changes them), but the centre is now the point straight ahead
  of the camera at the sphere's radius, so pitch and yaw (which turn the camera
  where it is) move the centre; roll does not. F1/F2 still change the radius.
  The camera is kept inside a boundary sphere (`CAMERA_BOUNDARY_RADIUS`) and
  outside every solid object (`CAMERA_SHAPE_CLEARANCE`). It keeps its axes as
  vectors rather than angles, so there is no pole or gimbal lock.
- Faces that reach behind the camera are clipped to the near plane before
  they are used for hidden-line removal and colour fill. Before, a face with
  any corner behind the camera was dropped whole, which was fine while the
  camera could never be close to a shape.

## [V1.0RC2] - 2026-10-02

### Added
- Build outputs are named without the `picocalc-` prefix, since the chip or system
  at the end of the name already says what they are for: `3d-visualizer-RP2040.uf2`, `3d-visualizer-RP2350.uf2`, `3d-visualizer-Windows.exe` and `3d-visualizer-Linux`.
- Leaving: on the PicoCalc build `ESC` or `Q` now leave for the PicoCalc UF2
  Loader menu (it asks the loader for its menu through the watchdog scratch
  registers; with no loader installed the program just restarts). `~` still goes
  to BOOTSEL.

### Changed
- Build outputs now carry the platform / chip at the end of their file names
  (`3d-visualizer-Windows.exe`, `3d-visualizer-Linux`,
  `3d-visualizer-RP2040.uf2` / `-RP2350.uf2`, in the build folders as
  well as the top level). The Linux release workflow uses the new names.
- The Linux build now copies `libSDL2-2.0.so.0` next to the binary itself (build
  folder and top level), found through the existing `$ORIGIN` run-path.
