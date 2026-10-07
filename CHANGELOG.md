# Changelog

## [Unreleased]

### Changed
- The orbit camera is now a free-flight camera. `p`/`P` pitch, `y`/`Y` yaw and
  `r`/`R` roll the view (the capital letter, SHIFT + the key, is the other
  direction), UP/DOWN fly forward and backward along the line of sight,
  LEFT/RIGHT fly sideways level with the ground (a roll does not change
  where they go), F1/F2 fly up and down (they used to zoom), and `z` or `Z`
  puts the camera back at its starting place. The camera is kept inside a
  boundary sphere (`CAMERA_BOUNDARY_RADIUS`) and outside every solid
  object (`CAMERA_SHAPE_CLEARANCE`). The camera keeps its axes as vectors
  rather than angles, so there is no pole or gimbal lock. The old orbit
  camera is on the `main` branch and the `before-free-flight` tag.
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
