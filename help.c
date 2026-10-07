#include "help.h"
#include "help_text.h"
#include "constants.h"
#include "gfx.h"
#include "text.h"

// Author: Thomas Dzubin
void help_draw(void)
{
    int y = HELP_TOP_Y;
    int action_x = HELP_MARGIN_X + HELP_KEY_COLUMNS * TEXT_8X10_WIDTH * HELP_TEXT_SCALE;
    int i;

    gfx_clear(COLOR_BLACK);

    text_draw_8x10(HELP_MARGIN_X, y, HELP_TITLE, HELP_COLOR_TITLE, HELP_TEXT_SCALE);
    y += HELP_LINE_PITCH + HELP_GAP_PITCH;

    for (i = 0; i < HELP_ROW_COUNT; i++) {
        text_draw_8x10(HELP_MARGIN_X, y, HELP_ROWS[i].keys, HELP_COLOR_KEY, HELP_TEXT_SCALE);
        text_draw_8x10(action_x, y, HELP_ROWS[i].action, HELP_COLOR_TEXT, HELP_TEXT_SCALE);
        y += HELP_LINE_PITCH;
    }
    y += HELP_GAP_PITCH;

    for (i = 0; i < HELP_NOTE_COUNT; i++) {
        text_draw_8x10(HELP_MARGIN_X, y, HELP_NOTES[i], HELP_COLOR_TEXT, HELP_TEXT_SCALE);
        y += HELP_LINE_PITCH;
    }
    y += HELP_GAP_PITCH;

    text_draw_8x10(HELP_MARGIN_X, y, HELP_PROMPT, HELP_COLOR_PROMPT, HELP_TEXT_SCALE);

    gfx_present();
}
