# Changelog

## [Unreleased]

### Changed
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
