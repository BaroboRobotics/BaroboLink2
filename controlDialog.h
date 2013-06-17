/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   BaroboLink is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BaroboLink is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BaroboLink.  If not, see <http://www.gnu.org/licenses/>.
*/

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

#ifdef __cplusplus
extern "C" {
#endif

gboolean controllerHandlerTimeout(gpointer data);

#ifdef __cplusplus
}
#endif

#endif
