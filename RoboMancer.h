#ifndef _ROBOMANCER_H_
#define _ROBOMANCER_H_

#include <gtk/gtk.h>
#include "RobotManager.h"

#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include <math.h>
#define RAD2DEG(x) ((x)*180.0/M_PI)

#ifdef __cplusplus
extern "C" {
#endif

/* Global Funcs */
void initialize();
int getIterModelFromTreeSelection(GtkTreeView *treeView, GtkTreeModel **model, GtkTreeIter *iter);

/* Global Vars */
extern GtkBuilder *g_builder;
extern CRobotManager *g_robotManager;
extern GtkWidget *g_scieditor;
extern GtkWidget *g_window;
extern ScintillaObject *g_sci;

/* Connect Dialog */
G_MODULE_EXPORT void on_button_connect_addRobot_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_remove_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_moveUpAvailable_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_moveDownAvailable_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_connect_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_disconnect_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connectFailedOk_clicked(GtkWidget* widget, gpointer data);
void refreshConnectDialog();
int getConnectSelectedIndex();

/* Program Dialog */
G_MODULE_EXPORT void on_imagemenuitem_cut_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_copy_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_paste_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_open_activate(GtkWidget* widget, gpointer data);

/* Control Dialog */
G_MODULE_EXPORT void on_button_motor1back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor2back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor3back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor4back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor1stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor2stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor3stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor4stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor1forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor2forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor3forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor4forward_clicked(GtkWidget*w, gpointer data); 
void initControlDialog(void);
void* controllerHandlerThread(void* arg);

#ifdef __cplusplus
}
#endif



#endif
