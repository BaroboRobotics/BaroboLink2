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
