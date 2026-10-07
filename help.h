//
//  help.h - the help screen: the keys and what they do, drawn with the same
//  built-in font as the status display. No platform dependencies. The words
//  and positions are in help_text.h.
//

#ifndef HELP_H
#define HELP_H

// Clears the screen, draws the help screen and puts it on the display. The
// caller then waits for a key (see input_wait_for_key() in input.h) and draws
// the scene again.
void help_draw(void);

#endif
