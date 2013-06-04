/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   Foobar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Foobar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <mobot.h>
#include "BaroboLink.h"
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
  int index = (long)data;
  recordMobot_t* mobot = g_robotManager->getMobotIndex(index);
  if(mobot == NULL) {
    return;
  }
  if(mobot->mobot.formFactor == MOBOTFORM_ORIGINAL) {
    g_reflashMobotIndex = index;
    g_reflashMobot = mobot;
    strcpy(g_reflashAddress, RecordMobot_getAddress(mobot));
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
  } else {
    /* This is a Mobot-I or Mobot-L. Must use the firmware update utility to
     * upgrade the firmware. Pop up a dialog box... */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "To upgrade the firmware of this robot, please close BaroboLink and start the Mobot Firmware Update utility located in your Start menu at Start->All Programs->Barobo Mobot 2.0->Mobot Firmware Update.");
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_hide(GTK_WIDGET(d));
  }
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
    gtk_spinner_stop(g_reflashConnectSpinner);
    gtk_notebook_set_current_page(g_notebookRoot, 2);
  }
  return rc;
}

void* reflashConnectThread(void* arg)
{
  g_reflashConnectStatus = 1;
  if(g_stkComms->connect(g_reflashAddress) == 0) {
    g_reflashConnectStatus = 3;
  } else {
    g_reflashConnectStatus = -1;
  }
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
      sprintf(filename, "%s/BaroboLink/hexfiles/rev3.hex", datadir);
#else
      sprintf(filename, "hexfiles/rev3.hex");
#endif
      g_stkComms->programAllAsync(filename, 3);
    } else if (g_reflashHWRev == 4) {
#ifdef __MACH__
      datadir = getenv("XDG_DATA_DIRS");
      printf("%s\n", datadir);
      sprintf(filename, "%s/BaroboLink/hexfiles/rev4.hex", datadir);
      printf("%s\n", filename);
#else
      sprintf(filename, "hexfiles/rev4.hex");
#endif
      g_stkComms->programAllAsync(filename, 4);
    } else if (g_reflashHWRev == -1) {
#ifdef __MACH__
      datadir = getenv("XDG_DATA_DIRS");
      printf("%s\n", datadir);
      sprintf(filename, "%s/BaroboLink/hexfiles/rev3_safe.hex", datadir);
      printf("%s\n", filename);
#else
      sprintf(filename, "hexfiles/rev3_safe.hex");
#endif
      g_stkComms->programAllAsync(filename, 3);
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
    GtkWidget *cancelButton;
    cancelButton = (GTK_WIDGET(gtk_builder_get_object(g_builder, "button_reflashContinue")));
    gtk_widget_set_sensitive(cancelButton, TRUE);
    gtk_button_set_label(GTK_BUTTON(cancelButton), "Connect failed. Retry?");
    cancelButton = (GTK_WIDGET(gtk_builder_get_object(g_builder, "button_cancelFlash2")));
    gtk_widget_set_sensitive(cancelButton, TRUE);
    gtk_spinner_stop(g_reflashConnectSpinner);
    gtk_widget_set_sensitive(continueButton, TRUE);
    return FALSE;
  }
}

void on_button_reflashContinue_clicked(GtkWidget* widget, gpointer data)
{
  g_stkComms = new CStkComms();  
  /* The robot should now be in "Programming" mode. We will need to reconnect
   * no matter what because the Mobot library is currently hogging the
   * listening socket */
  /* Set the button insensitive */
  gtk_widget_set_sensitive(widget, FALSE);
  gtk_button_set_label(GTK_BUTTON(widget), "Continue");
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

void on_button_forceUpgradeBegin_clicked(GtkWidget* widget, gpointer data)
{
  /* Make sure the robot is not currently connected. If it is, disconnect it */
  const char* addr;
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_forceUpgradeAddr"));
  addr = gtk_entry_get_text(GTK_ENTRY(w));
  int i;
  for(i = 0; i < g_robotManager->numEntries(); i++) {
    if(!strcasecmp(addr, g_robotManager->getEntry(i))) {
      g_robotManager->disconnect(i);
    }
  }
  /* Set up global vars */
  strcpy(g_reflashAddress, addr);
  g_reflashHWRev = -1;
  /* Go to the appropriate reflash page */
  gtk_spinner_stop(g_reflashConnectSpinner);
  gtk_notebook_set_current_page(g_notebookRoot, 2);
}

void on_button_forceUpgradeCancel_clicked(GtkWidget *w, gpointer data)
{
  gtk_notebook_set_current_page(g_notebookRoot, 0);
}

void on_menuitem_forceUpgrade_activate(GtkWidget *w, gpointer data)
{
  gtk_notebook_set_current_page(g_notebookRoot, 5);
}
