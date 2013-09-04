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

#ifndef _MOBOT_FIRMWARE_UPDATE_H_
#define _MOBOT_FIRMWARE_UPDATE_H_

#ifdef _MSYS
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

G_MODULE_EXPORT void on_button_p1_next_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_p2_yes_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_flashAnother_clicked(GtkWidget* widget, gpointer data);

#ifdef __cplusplus
}
#endif
#endif
