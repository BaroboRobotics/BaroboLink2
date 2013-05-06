#ifndef _ROBOMANCER_H_
#define _ROBOMANCER_H_

#include <gtk/gtk.h>
#include "RobotManager.h"

#ifdef _MSYS
#include <windows.h>
#endif

#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include <math.h>
#define RAD2DEG(x) ((x)*180.0/M_PI)
#define DEG2RAD(x) ((x)*M_PI/180.0)

#ifdef __cplusplus
extern "C" {
#endif

/* Global Funcs */
void initialize();
int getIterModelFromTreeSelection(GtkTreeView *treeView, GtkTreeModel **model, GtkTreeIter *iter);

/* Global Vars */
extern int g_controlMode; // 0 means update main control page, 1 means update "sensors" page
extern int g_selectedRobot;
extern GtkBuilder *g_builder;
extern CRobotManager *g_robotManager;
extern GtkWidget *g_window;
extern GtkWidget *g_scieditor;
extern ScintillaObject *g_sci;
extern GtkWidget *g_scieditor_ext;
extern ScintillaObject *g_sci_ext;
extern recordMobot_t* g_activeMobot;
extern recordMobot_t *g_mobotParent;
#ifdef _MSYS
extern HANDLE g_activeMobotLock;
#else
extern pthread_mutex_t g_activeMobotLock;
#endif
extern GtkNotebook *g_notebookRoot;
extern GtkSpinner* g_reflashConnectSpinner;
G_MODULE_EXPORT void on_imagemenuitem_about_activate(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_aboutdialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
G_MODULE_EXPORT void on_aboutdialog_close(GtkDialog *dialog, gpointer user_data);
G_MODULE_EXPORT void on_aboutdialog_activate_link(GtkAboutDialog *label, gchar* uri, gpointer data);
G_MODULE_EXPORT gboolean on_window1_delete_event(GtkWidget *w);
G_MODULE_EXPORT void on_menuitem_help_activate(GtkWidget *widget, gpointer data);
G_MODULE_EXPORT void on_menuitem_demos_activate(GtkWidget *widget, gpointer data);

/* Connect Dialog */
G_MODULE_EXPORT void on_button_connect_addRobot_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_remove_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_moveUpAvailable_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_moveDownAvailable_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_connect_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_disconnect_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connectFailedOk_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_Connect_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_Disconnect_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_Remove_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_MoveDown_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_MoveUp_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_scanMobots_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_colorDialog_color_set(GtkColorButton* w, gpointer data);
G_MODULE_EXPORT void on_beep_button_pressed(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_beep_button_released(GtkWidget *w, gpointer data);
void refreshConnectDialog();
gboolean connectDialogPulse(gpointer data);

/* Connect Dongle Dialog */
G_MODULE_EXPORT void on_button_connectDongleConnect_clicked(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_button_connectDongleClose_clicked(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_menuitem_DongleDialog_activate(GtkWidget *w, gpointer data);
void askConnectDongle(void);
void showConnectDongleDialog(void);
void hideConnectDongleDialog(void);

/* Scan Mobots Dialog */
void initScanMobotsDialog();
void showScanMobotsDialog();
G_MODULE_EXPORT void on_button_scanMobotsAdd_clicked(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_button_scanMobotsOK_clicked(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_button_scanMobotsCancel_clicked(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_button_scanMobotsRefresh_clicked(GtkWidget *w, gpointer data);

/* Reflashing Process */
extern char g_reflashAddress[80];
extern int g_reflashHWRev;
G_MODULE_EXPORT void on_button_updateFirmware_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_cancelFlash_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_reflashContinue_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_cancelFlash2_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_reflashOK_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_forceUpgradeBegin_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_forceUpgradeCancel_clicked(GtkWidget *w, gpointer data);
G_MODULE_EXPORT void on_menuitem_forceUpgrade_activate(GtkWidget *w, gpointer data);

/* Program Dialog */
void initProgramDialog(void);
gboolean check_io_timeout(gpointer data);
G_MODULE_EXPORT gboolean on_textview_programMessages_key_press_event(GtkWidget*w, GdkEventKey* event, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_new_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_open_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_save_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_saveAs_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_undo_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_redo_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_cut_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_copy_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_paste_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_open_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_exportExe_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_runExe_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_scintilla_notify(GObject *gobject, GParamSpec *pspec, struct SCNotification* scn);
G_MODULE_EXPORT void on_checkbutton_showExternalEditor_toggled(GtkToggleButton *tb, gpointer data);
void refreshExternalEditor();

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
G_MODULE_EXPORT gboolean on_vscale_motorPos1_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos2_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos3_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos4_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos1_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos2_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos3_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos4_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data);

G_MODULE_EXPORT gboolean on_vscale_motorspeed1_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed2_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed3_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed4_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed1_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed2_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed3_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed4_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_forward_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_rotateLeft_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_stop_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_rotateRight_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_backward_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_setSpeeds_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_moveToZero_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_move_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_moveTo_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_playGait_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_colorselection_color_changed(GtkColorSelection *w, gpointer user_data);
void initControlDialog(void);
void* controllerHandlerThread(void* arg);

/* Teaching Dialog */
G_MODULE_EXPORT void on_button_setJointsNeutral_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_holdJoints_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_clearRecordedPositions_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_recordPos_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_addDelay_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_deleteRecordedPos_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_saveToProgram_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_radiobutton_neutralOnExit_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_radiobutton_holdOnExit_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_playRecorded_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_stopRecorded_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_checkbutton_playLooped_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_notebook1_switch_page(GtkNotebook* notebook, gpointer page, guint page_num, gpointer userdata);
G_MODULE_EXPORT void on_liststore_recordedMotions_rows_reordered(
    GtkTreeModel* model, 
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer new_order,
    gpointer user_data);
G_MODULE_EXPORT void on_cellrenderertext_recordedMotionName_edited(
    GtkCellRendererText* renderer,
    gchar* path,
    gchar* new_text,
    gpointer user_data);
G_MODULE_EXPORT void on_liststore_recordedMotions_row_deleted(
    GtkTreeModel* model,
    GtkTreePath* path,
    gpointer user_data);
G_MODULE_EXPORT void on_liststore_recordedMotions_row_inserted(
    GtkTreeModel* model,
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer user_data);
void* playThread(void* arg);
gboolean poseGuiTimeout(gpointer userdata);
extern bool g_holdOnExit;

/* Comms Engine */
int initializeComms(void);
double normalizeAngleRad(double radians);

#ifdef __cplusplus
}
#endif



#endif
