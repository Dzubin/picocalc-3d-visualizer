//
//  help_text.h - what the help screen says and where it puts it (see
//  help.h). It is drawn in the PicoCalc's 8x10 font, so lower case works.
//  Keep each line within HELP_MAX_CHARS characters or it runs off the right
//  edge of the screen.
//

#ifndef HELP_TEXT_H
#define HELP_TEXT_H

#include "constants.h"

// --- Layout -----------------------------------------------------------------
#define HELP_TEXT_SCALE 1          // the 8x10 font at its own size: 8 x 10 pixels a character
#define HELP_MARGIN_X 8
#define HELP_TOP_Y 12
#define HELP_LINE_PITCH 16         // px from one line to the next
#define HELP_GAP_PITCH 8           // extra px before a new group of lines
#define HELP_KEY_COLUMNS 12        // characters the key column takes, the action starts after it
#define HELP_MAX_CHARS 38          // (SCREEN_WIDTH - 2 * HELP_MARGIN_X) / 8

// --- Colours (full colours, no dim greys) -------------------------------------
#define HELP_COLOR_TITLE  COLOR_STATUS
#define HELP_COLOR_KEY    COLOR_AXIS_Y
#define HELP_COLOR_TEXT   COLOR_STATUS
#define HELP_COLOR_PROMPT COLOR_CYAN

// --- Words ------------------------------------------------------------------
#define HELP_TITLE "3D Visualizer keys"
#define HELP_PROMPT "Press any key to continue"

typedef struct {
    const char *keys;      // the key column
    const char *action;    // what it does
} help_row_t;

static const help_row_t HELP_ROWS[] = {
    {"Left/Right", "Orbit around the centre"},
    {"Up/Down",    "Orbit over the top"},
    {"F1/F2",      "Zoom in / out"},
    {"p/P",        "Pitch"},
    {"y/Y",        "Yaw"},
    {"r/R",        "Roll"},
    {"z",          "Back to the start view"},
    {"l",          "Hidden lines on / off"},
    {"h",          "This help"},
    {"Esc or q",   "Back to the title screen"},
    {"~",          "BOOTSEL (PicoCalc only)"},
};
#define HELP_ROW_COUNT ((int)(sizeof(HELP_ROWS) / sizeof(HELP_ROWS[0])))

// Notes under the table, one line each.
static const char *const HELP_NOTES[] = {
    "The centre is the point right ahead.",
    "Pitch and yaw move it, roll does not.",
    "Arrows follow the world, not the roll.",
    "On the title screen, q or Esc leaves.",
};
#define HELP_NOTE_COUNT ((int)(sizeof(HELP_NOTES) / sizeof(HELP_NOTES[0])))

#endif
