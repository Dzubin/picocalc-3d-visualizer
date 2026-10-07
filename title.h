//
//  title.h - the title screen that comes before the 3D viewer: the program's
//  name, its version and its author, and "PRESS ANY KEY TO CONTINUE". No
//  platform dependencies. The words and positions are in title_text.h.
//

#ifndef TITLE_H
#define TITLE_H

// Clears the screen, draws the title screen and puts it on the display. The
// caller then waits for a key (see input_wait_for_key() in input.h).
void title_draw(void);

// Clears the screen and asks "Leave the program?" (Y leaves, any other key
// stays). The caller then waits for a key.
void title_leave_draw(void);

#endif
