#include <string.h>
#include <gtk/gtk.h>
#include <mobot.h>
#include "RoboMancer.h"
#include "RecordMobot.h"
#include "libstkcomms/libstkcomms.hpp"

recordMobot_t* g_reflashMobot;
int g_reflashMobotIndex;
char g_reflashAddress[80];
int g_reflashHWRev;
CStkComms *g_stkComms;
THREAD_T g_connectThread;
int g_cancelListenButtonHWRev = 0;

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
  int index = (int)data;
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
  //if(rc) {
    /* Need to determine revision number by button press */
    gtk_notebook_set_current_page(g_notebookRoot, 1);
    /* Start a timeout function that listens for a button A press */
    g_idle_add(listenButtonHWRev, NULL);
    /*
  } else {
    g_reflashHWRev = hwRev;
    gtk_notebook_set_current_page(g_notebookRoot, 2);
  }
  */
}

void on_button_cancelFlash_clicked(GtkWidget* widget, gpointer data)
{
  g_cancelListenButtonHWRev = 1;
  gtk_notebook_set_current_page(g_notebookRoot, 0);
}

gboolean listenButtonHWRev(gpointer data)
{
  /* Get the button voltage */
  double voltage;
  gboolean rc;
  if(g_cancelListenButtonHWRev) {
    g_cancelListenButtonHWRev = 0;
    return FALSE;
  }
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
  char *filename;
  char *datadir;
  filename = (char*)malloc(512);
  GtkWidget *continueButton = GTK_WIDGET(data);
  if(g_reflashConnectStatus == 3) {
    /* Switch the root notebook to the next page */
    gtk_notebook_set_current_page(g_notebookRoot, 3);
    if(g_reflashHWRev == 3) {
#ifdef __MACH__
      datadir = getenv("XDG_DATA_DIRS");
      sprintf(filename, "%s/RoboMancer/rev3.hex", datadir);
#else
      sprintf(filename, "interface/rev3.hex");
#endif
      g_stkComms->programAllAsync(filename, 3);
    } else if (g_reflashHWRev == 4) {
#ifdef __MACH__
      datadir = getenv("XDG_DATA_DIRS");
      printf("%s\n", datadir);
      sprintf(filename, "%s/RoboMancer/rev4.hex", datadir);
      printf("%s\n", filename);
#else
      sprintf(filename, "interface/rev4.hex");
#endif
      g_stkComms->programAllAsync(filename, 4);
    } else {
      fprintf(stderr, "Error: Invalid HW Rev detected.\n");
      gtk_widget_set_sensitive(continueButton, TRUE);
      return FALSE;
    }
    g_timeout_add(500, updateProgrammingProgressTimeout, NULL);
    gtk_widget_set_sensitive(continueButton, TRUE);
    return FALSE;
  } else if (g_reflashConnectStatus == 1) {
    return TRUE;
  } else {
    /* Connection failed? */
    GtkWidget *cancelButton = (GTK_WIDGET(gtk_builder_get_object(g_builder, "button_cancelFlash2")));
    gtk_widget_set_sensitive(cancelButton, TRUE);
    gtk_button_set_label(GTK_BUTTON(cancelButton), "Connect failed. Retry?");
    cancelButton = (GTK_WIDGET(gtk_builder_get_object(g_builder, "button_reflashContinue")));
    gtk_widget_set_sensitive(cancelButton, TRUE);
    gtk_spinner_stop(g_reflashConnectSpinner);
    gtk_widget_set_sensitive(continueButton, TRUE);
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
  /* Set the button insensitive */
  gtk_widget_set_sensitive(widget, FALSE);
  /* Set the cancel button to not-sensitive too */
  GtkWidget *cancelButton = (GTK_WIDGET(gtk_builder_get_object(g_builder, "button_cancelFlash2")));
  gtk_widget_set_sensitive(cancelButton, FALSE);
  gtk_widget_show(GTK_WIDGET(g_reflashConnectSpinner));
  gtk_spinner_start(g_reflashConnectSpinner);
  THREAD_CREATE(&g_connectThread, reflashConnectThread, NULL);
  g_reflashConnectStatus = 1;
  g_timeout_add(500, reflashConnectTimeout, widget);
  g_reflashProgressBar = GTK_PROGRESS_BAR(gtk_builder_get_object(g_builder, "progressbar_reflash"));
}

void on_button_cancelFlash2_clicked(GtkWidget* widget, gpointer data)
{
  /* If this button was clicked, just go back to the default page */
  refreshConnectDialog();
  gtk_notebook_set_current_page(g_notebookRoot, 0);
}

gboolean updateProgrammingProgressTimeout(gpointer data)
{
  double progress = g_stkComms->getProgress();
  if(!g_stkComms->isProgramComplete()) {
    gtk_progress_bar_set_fraction(g_reflashProgressBar, progress);
    return TRUE;
  } else {
    g_stkComms->disconnect();
    gtk_notebook_set_current_page(g_notebookRoot, 4);
    return FALSE;
  }
}

void on_button_reflashOK_clicked(GtkWidget* widget, gpointer data)
{
  /* Refresh the connect dialog and send the user back to the main page */
  refreshConnectDialog();
  gtk_notebook_set_current_page(g_notebookRoot, 0);
}
