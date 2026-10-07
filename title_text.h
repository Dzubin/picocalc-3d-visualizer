//
//  title_text.h - what the title screen says and where it puts it (see
//  title.h). It is drawn in the PicoCalc's 8x10 font. Every line is centred
//  across the screen, so a line only has to fit: a character is 8 pixels times
//  its scale wide, so at scale 2 a line holds 20 characters, at scale 1 40.
//

#ifndef TITLE_TEXT_H
#define TITLE_TEXT_H

#include "constants.h"

// --- Words ------------------------------------------------------------------
#define TITLE_NAME_LINE_1 "PicoCalc"
#define TITLE_NAME_LINE_2 "3D Visualizer"
#define TITLE_VERSION_TEXT "Version " VERSION
#define TITLE_AUTHOR_TEXT "by Thomas Dzubin"
#define TITLE_PROMPT_TEXT "Press any key to continue"
#define TITLE_LEAVE_QUESTION "Leave the program?"
#define TITLE_LEAVE_KEYS "Y: leave    any other key: stay"

// --- Layout (screen pixels; a character is 8 x 10 times its scale) ------------------
#define TITLE_NAME_SCALE 3
#define TITLE_NAME_LINE_1_Y 70
#define TITLE_NAME_LINE_2_Y 108
#define TITLE_VERSION_SCALE 2
#define TITLE_VERSION_Y 176
#define TITLE_AUTHOR_SCALE 2
#define TITLE_AUTHOR_Y 206
#define TITLE_PROMPT_SCALE 1
#define TITLE_PROMPT_Y 270
#define TITLE_LEAVE_QUESTION_SCALE 2
#define TITLE_LEAVE_QUESTION_Y 120
#define TITLE_LEAVE_KEYS_SCALE 1
#define TITLE_LEAVE_KEYS_Y 170

// --- Colours (full colours, no dim greys) -------------------------------------
#define TITLE_COLOR_NAME    COLOR_STATUS
#define TITLE_COLOR_VERSION COLOR_AXIS_Y
#define TITLE_COLOR_AUTHOR  COLOR_STATUS
#define TITLE_COLOR_PROMPT  COLOR_CYAN
#define TITLE_COLOR_QUESTION COLOR_STATUS

#endif
