# Changelog

## [Unreleased]

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
