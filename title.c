#include "title.h"
#include "title_text.h"
#include "constants.h"
#include "gfx.h"
#include "text.h"

static void draw_centred(int y, const char *s, unsigned short color, int scale);

// Author: Thomas Dzubin
void title_draw(void)
{
    gfx_clear(COLOR_BLACK);

    draw_centred(TITLE_NAME_LINE_1_Y, TITLE_NAME_LINE_1, TITLE_COLOR_NAME, TITLE_NAME_SCALE);
    draw_centred(TITLE_NAME_LINE_2_Y, TITLE_NAME_LINE_2, TITLE_COLOR_NAME, TITLE_NAME_SCALE);
    draw_centred(TITLE_VERSION_Y, TITLE_VERSION_TEXT, TITLE_COLOR_VERSION, TITLE_VERSION_SCALE);
    draw_centred(TITLE_AUTHOR_Y, TITLE_AUTHOR_TEXT, TITLE_COLOR_AUTHOR, TITLE_AUTHOR_SCALE);
    draw_centred(TITLE_PROMPT_Y, TITLE_PROMPT_TEXT, TITLE_COLOR_PROMPT, TITLE_PROMPT_SCALE);

    gfx_present();
}

// Author: Thomas Dzubin
void title_leave_draw(void)
{
    gfx_clear(COLOR_BLACK);

    draw_centred(TITLE_LEAVE_QUESTION_Y, TITLE_LEAVE_QUESTION, TITLE_COLOR_QUESTION, TITLE_LEAVE_QUESTION_SCALE);
    draw_centred(TITLE_LEAVE_KEYS_Y, TITLE_LEAVE_KEYS, TITLE_COLOR_PROMPT, TITLE_LEAVE_KEYS_SCALE);

    gfx_present();
}

/* Draws a line of text with its middle at the middle of the screen. */
static void draw_centred(int y, const char *s, unsigned short color, int scale)
{
    text_draw_8x10((SCREEN_WIDTH - text_width_8x10(s, scale)) / 2, y, s, color, scale);
}
