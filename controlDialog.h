#ifndef _CONTROLDIALOG_H_
#define _CONTROLDIALOG_H_

#define BUTTON(x) B_##x,
#define SLIDER(x) S_##x,
enum controlButtons_e
{
#include "buttons.x.h"
  NUM_BUTTONS
};
#undef BUTTON
#undef SLIDER

#define BUTTON(x) \
int handler##x(void* arg);
#define SLIDER(x) \
int handler##x(void* arg);
#include "buttons.x.h"
#undef BUTTON
#undef SLIDER

#endif
