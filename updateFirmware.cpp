#include <string.h>
#include <gtk/gtk.h>
#include <mobot.h>
#include "RoboMancer.h"
#include "RecordMobot.h"
#include "libstkcomms.hpp"

recordMobot_t* g_reflashMobot;
int g_reflashMobotIndex;
char g_reflashAddress[80];
int g_reflashHWRev;
CStkComms *g_stkComms;

int g_reflashConnectStatus;
/* g_reflashConnectStatus Status:
 * 1: Connecting
 * 2: Connection Failed
 * 3: Connection Succeeded */
GtkNotebook *g_notebookRoot;
GtkSpinner* g_reflashConnectSpinner;
GtkProgressBar* g_reflashProgressBar;

gboolean listenButtonHWRev(gpointer data);
gboolean updateProgrammingProgressTimeout(gpointer data);

void on_button_updateFirmware_clicked(GtkWidget* widget, gpointer data)
{
  /* Try and grab the HW rev number from the robot */
  int index = getConnectSelectedIndex();
  recordMobot_t* mobot = g_robotManager->getMobotIndex(index);
  if(mobot == NULL) {
    return;
  }
  g_reflashMobotIndex = index;
  g_reflashMobot = mobot;
  strcpy(g_reflashAddress, RecordMobot_getAddress(mobot));
  g_notebookRoot = GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook_root"));
  int hwRev;
  int rc = Mobot_getHWRev((mobot_t*)mobot, &hwRev);
  if(rc) {
    /* Need to determine revision number by button press */
    gtk_notebook_set_current_page(g_notebookRoot, 1);
    /* Start a timeout function that listens for a button A press */
    g_idle_add(listenButtonHWRev, NULL);
  } else {
    g_reflashHWRev = hwRev;
    gtk_notebook_set_current_page(g_notebookRoot, 2);
  }
}

gboolean listenButtonHWRev(gpointer data)
{
  /* Get the button voltage */
  double voltage;
  gboolean rc;
  Mobot_getButtonVoltage((mobot_t*)g_reflashMobot, &voltage);
  if(ABS(voltage-5) < 0.1) {
    rc = TRUE;
  } else if (ABS(voltage - 2.5) < 0.1) {
    g_reflashHWRev = 3;
    rc = FALSE;
  } else if (ABS(voltage - 1.685) < 0.1) {
    g_reflashHWRev = 4;
    rc = FALSE;
  }
  if(rc == FALSE) {
    /* Swith the main notebook to page 2 */
    g_robotManager->disconnect(g_reflashMobotIndex);
    gtk_notebook_set_current_page(g_notebookRoot, 2);
  }
  return rc;
}

void* reflashConnectThread(void* arg)
{
  g_reflashConnectStatus = 1;
  g_stkComms->connect(g_reflashAddress);
  g_reflashConnectStatus = 3;
}

gboolean reflashConnectTimeout(gpointer data) 
{
  if(g_reflashConnectStatus == 3) {
    /* Switch the root notebook to the next page */
    gtk_notebook_set_current_page(g_notebookRoot, 3);
    if(g_reflashHWRev == 3) {
      printf("Programming rev3...\n");
      g_stkComms->programAllAsync("interface/rev3.hex");
    } else if (g_reflashHWRev == 4) {
      printf("Programming rev4...\n");
      g_stkComms->programAllAsync("interface/rev4.hex");
    } else {
      fprintf(stderr, "Error: Invalid HW Rev detected.\n");
      return FALSE;
    }
    g_timeout_add(500, updateProgrammingProgressTimeout, NULL);
    return FALSE;
  } else if (g_reflashConnectStatus == 1) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void on_button_reflashContinue_clicked(GtkWidget* widget, gpointer data)
{
  g_stkComms = new CStkComms();  
  g_reflashConnectSpinner = GTK_SPINNER(gtk_builder_get_object(g_builder, "spinner_reflashConnect"));
  /* The robot should now be in "Programming" mode. We will need to reconnect
   * no matter what because the Mobot library is currently hogging the
   * listening socket */
  THREAD_T connectThread;
  gtk_widget_show(GTK_WIDGET(g_reflashConnectSpinner));
  gtk_spinner_start(g_reflashConnectSpinner);
  THREAD_CREATE(&connectThread, reflashConnectThread, NULL);
  g_timeout_add(500, reflashConnectTimeout, NULL);
  g_reflashProgressBar = GTK_PROGRESS_BAR(gtk_builder_get_object(g_builder, "progressbar_reflash"));
}

gboolean updateProgrammingProgressTimeout(gpointer data)
{
  double progress = g_stkComms->getProgress();
  if(progress < 1) {
    gtk_progress_bar_set_fraction(g_reflashProgressBar, progress);
    return TRUE;
  } else {
    gtk_notebook_set_current_page(g_notebookRoot, 4);
    return FALSE;
  }
}
