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

#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <string.h>
#define PLAT_GTK 1
#define GTK
#ifdef __MACH__
#include <sys/types.h>
#include <unistd.h>
#include <gtk-mac-integration.h>
#endif
#include <sys/stat.h>
#include <mobot.h>
#include <libstkcomms.hpp>
#include "MobotFirmwareUpdate.h"
#include "thread_macros.h"

#define MAX_THREADS 20

typedef enum dongleSearchStatus_e
{
  DONGLE_NIL,
  DONGLE_FOUND,
  DONGLE_NOTFOUND,
  DONGLE_SEARCHING,
} dongleSearchStatus_t;

GtkBuilder *g_builder;
GtkWidget *g_window;
dongleSearchStatus_t g_dongleSearchStatus;
MUTEX_T g_giant_lock;
COND_T g_giant_cond;
THREAD_T g_mainSearchThread;

mobot_t* g_mobot = NULL;

int g_numThreads = 0;

char g_comport[80];
CStkComms *g_stkComms;

char *g_interfaceFiles[512] = {
  "interface/mobotfirmwareupdateinterface.glade",
  "mobotfirmwareupdateinterface.glade ",
  "../share/BaroboLink/mobotfirmwareupdateinterface.glade",
  NULL,
  NULL
};

const char* g_hexfilename;

void* findDongleWorkerThread(void* arg)
{
  /* The argument is a pointer to an int */
  int num = *(int*)arg;
  char buf[80];
  mobot_t* mobot;
  int rc;
#if defined (_WIN32) or defined (_MSYS)
  sprintf(buf, "\\\\.\\COM%d", num);
#else
  sprintf(buf, "/dev/ttyACM%d", num);
#endif
  /* Try to connect to it */
  mobot = (mobot_t*)malloc(sizeof(mobot_t));
  Mobot_init(mobot);
  rc = Mobot_connectWithTTY(mobot, buf);
  if(rc == 0) {
    /* We found the Mobot */
    MUTEX_LOCK(&g_giant_lock);
    /* Only update g_mobot pointer if no one else has updated it already */
    if(g_mobot == NULL) {
      g_mobot = mobot;
      g_dongleSearchStatus = DONGLE_FOUND;
      strcpy(g_comport, buf);
      COND_SIGNAL(&g_giant_cond);
      MUTEX_UNLOCK(&g_giant_lock);
    } else {
      MUTEX_UNLOCK(&g_giant_lock);
      Mobot_disconnect(mobot);
      free(mobot);
    }
    Mobot_getStatus(mobot);
  } else {
    free(mobot);
  }
  MUTEX_LOCK(&g_giant_lock);
  g_numThreads--;
  COND_SIGNAL(&g_giant_cond);
  MUTEX_UNLOCK(&g_giant_lock);
  return NULL;
}

#define MAX_COMPORT 256
void* findDongleThread(void* arg)
{
  static int args[MAX_COMPORT];
  int i;
  THREAD_T threads[MAX_COMPORT];
  for(i = 0; i < MAX_COMPORT; i++) {
    args[i] = i;
  }
  g_dongleSearchStatus = DONGLE_SEARCHING;
  /* Spawn worker threads to find the dongle on a com port */
  for(i = 0; i < MAX_COMPORT; i++) { 
    /* First, make sure there are less than MAX_THREADS running */
    MUTEX_LOCK(&g_giant_lock);
    while(
        (g_numThreads >= MAX_THREADS) &&
        (g_dongleSearchStatus == DONGLE_SEARCHING)
        ) 
    {
      COND_WAIT(&g_giant_cond, &g_giant_lock);
    }
    if(g_dongleSearchStatus != DONGLE_SEARCHING) {
      i++;
      MUTEX_UNLOCK(&g_giant_lock);
      break;
    }
    /* Spawn a thread */
    THREAD_CREATE(&threads[i], findDongleWorkerThread, &args[i]);
    g_numThreads++;
    MUTEX_UNLOCK(&g_giant_lock);
  }
  /* Join all threads */
  int j;
  for(j = 0; j < i; j++) {
    MUTEX_LOCK(&g_giant_lock);
    if(g_dongleSearchStatus != DONGLE_SEARCHING) {
      MUTEX_UNLOCK(&g_giant_lock);
      break;
    }
    MUTEX_UNLOCK(&g_giant_lock);
    THREAD_JOIN(threads[j]);
  }
  MUTEX_LOCK(&g_giant_lock);
  if(g_dongleSearchStatus == DONGLE_SEARCHING) {
    g_dongleSearchStatus = DONGLE_NOTFOUND;
  }
  MUTEX_UNLOCK(&g_giant_lock);
  return NULL;
}

gboolean findDongleTimeout(gpointer data)
{
  char buf[80];
  /* Check to see if the dongle is found. If it is, proceed to the next page.
   * If not, reset state vars and stay on the first page, pop up a warning
   * dialog. */
  gboolean rc = FALSE;
  MUTEX_LOCK(&g_giant_lock);
  if(g_dongleSearchStatus == DONGLE_SEARCHING) {
    rc = TRUE;
  } else if (g_dongleSearchStatus == DONGLE_FOUND) {
    /* Set up the labels and stuff */
    switch(g_mobot->formFactor) {
      case MOBOTFORM_I:
        sprintf(buf, "Mobot-I");
        break;
      case MOBOTFORM_L:
        sprintf(buf, "Mobot-L");
        break;
      default:
        sprintf(buf, "Unkown");
        break;
    }
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label_formFactor")),
        buf);
    sprintf(buf, "%d", Mobot_getVersion(g_mobot));
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label_firmwareVersion")),
        buf);
    sprintf(buf, "%d", Mobot_protocolVersion());
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label_upgradeVersion")),
        buf);
    gtk_entry_set_text(
        GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_serialID")),
        g_mobot->serialID);
    /* Go to next notebook page */
    gtk_notebook_next_page(
        GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")));
    rc = FALSE;
  } else if (g_dongleSearchStatus == DONGLE_NOTFOUND) {
    /* Renable the button */
    gtk_widget_set_sensitive(
        GTK_WIDGET(gtk_builder_get_object(g_builder, "button_p1_next")),
        TRUE);
    /* Pop up a dialog box */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_OK,
        "No attached Mobot was detected. Please make sure there is a Mobot connected to the computer with a Micro-USB cable and that the Mobot is powered on.");
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_hide(GTK_WIDGET(d));
    rc = FALSE;
  } else {
    /* Something very weird happened */
    fprintf(stderr, "Fatal error: %s:%d\n", __FILE__, __LINE__);
    exit(-1);
  }
  MUTEX_UNLOCK(&g_giant_lock);
  return rc;
}

int main(int argc, char* argv[])
{
  GError *error = NULL;

#ifdef _WIN32
  /* Make sure there isn't another instance of BaroboLink running by checking
   * for the existance of a named mutex. */
  HANDLE hMutex;
  hMutex = CreateMutex(NULL, TRUE, TEXT("Global\\RoboMancerMutex"));
  DWORD dwerror = GetLastError();
#endif

 
  gtk_init(&argc, &argv);

  /* Create the GTK Builder */
  g_builder = gtk_builder_new();

#ifdef _WIN32
  if(dwerror == ERROR_ALREADY_EXISTS) {
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "An instance of BaroboLink is already running. Please terminate Robomancer before running the Mobot Firmware Update Utility.");
    int rc = gtk_dialog_run(GTK_DIALOG(d));
    exit(0);
  }
#endif
#ifdef __MACH__
  char *datadir = getenv("XDG_DATA_DIRS");
  if(datadir != NULL) {
    g_interfaceFiles[3] = (char*)malloc(sizeof(char)*512);
    sprintf(g_interfaceFiles[3], "%s/BaroboLink/interface.glade", datadir);
  }
#endif

  /* Load the UI */
  /* Find ther interface file */
  struct stat s;
  int err;
  int i;
  for(i = 0; g_interfaceFiles[i] != NULL; i++) {
    err = stat(g_interfaceFiles[i], &s);
    if(err == 0) {
      if( ! gtk_builder_add_from_file(g_builder, g_interfaceFiles[i], &error) )
      {
        g_warning("%s", error->message);
        //g_free(error);
        return -1;
      } else {
        break;
      }
    }
  }

  if(g_interfaceFiles[i] == NULL) {
    /* Could not find the interface file */
    g_warning("Could not find interface file.");
    return -1;
  }

  /* Get the main window */
  g_window = GTK_WIDGET( gtk_builder_get_object(g_builder, "window1"));
  /* Connect signals */
  gtk_builder_connect_signals(g_builder, NULL);

#ifdef __MACH__
  //g_signal_connect(GtkOSXMacmenu, "NSApplicationBlockTermination",
      //G_CALLBACK(app_should_quit_cb), NULL);
  GtkWidget* quititem = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem5"));
  gtk_mac_menu_set_quit_menu_item(GTK_MENU_ITEM(quititem));
#endif

  /* Show the window */
  gtk_widget_show(g_window);
  gtk_main();
  return 0;
}

void on_button_p1_next_clicked(GtkWidget* widget, gpointer data)
{
  /* First, disable the widget so it cannot be clicked again */
  gtk_widget_set_sensitive(widget, false);

  /* Make sure thread is joined */
  THREAD_JOIN(g_mainSearchThread);

  /* Reset state vars */
  g_dongleSearchStatus = DONGLE_SEARCHING;
  
  /* Start the main search thread */
  THREAD_CREATE(&g_mainSearchThread, findDongleThread, NULL);

  /* Start the search timeout */
  g_timeout_add(200, findDongleTimeout, NULL);
}

gboolean programming_progress_timeout(gpointer data)
{
  /* Set the progress bar */
  gtk_progress_bar_set_fraction(
      GTK_PROGRESS_BAR(gtk_builder_get_object(g_builder, "progressbar1")),
      g_stkComms->getProgress());
  /* See if programming is completed */
  if(g_stkComms->isProgramComplete()) {
    g_stkComms->disconnect();
    /* Switch to next page */
    gtk_notebook_next_page(
        GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")));
  } else {
    return TRUE;
  }
}

gboolean switch_to_p3_timeout(gpointer data)
{
  /* Renable the button */
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "button_p2_yes")),
      true);
  /* Switch to next page */
  gtk_notebook_next_page(
      GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")));
  g_stkComms = new CStkComms();
  g_stkComms->connectWithTTY(g_comport);
  g_stkComms->programAllAsync(g_hexfilename);
  /* Start the programming progress timeout */
  g_timeout_add(500, programming_progress_timeout, NULL);
  return FALSE;
}

int fileExists(const char* filename)
{
  struct stat s;
  return (stat(filename, &s) == 0);
}

void on_button_p2_yes_clicked(GtkWidget* widget, gpointer data)
{
  int i;
  gtk_widget_set_sensitive(widget, false);
  /* First, reprogram the serial ID */
  const char* text;
  char buf[5];
  /* Make sure a file is selected and exists */
  g_hexfilename = gtk_file_chooser_get_filename(
      GTK_FILE_CHOOSER(gtk_builder_get_object(g_builder, "filechooserbutton_hexfile")));
  if(!fileExists(g_hexfilename)) {
    /* Pop up a warning dialog and abort */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "File does not exist. Please select a valid hex file to flash to the Linkbot.");
    int rc = gtk_dialog_run(GTK_DIALOG(d));
    return;
  }
  text = gtk_entry_get_text(
      GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_serialID")));
#if 0
  if(strlen(text)==4) {
    for(i = 0; i < 5; i++) {
      buf[i] = toupper(text[i]);
    }
    Mobot_setID(g_mobot, buf);
  } else {
    /* Pop up warning dialog */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "The Serial ID does not have a valid format. The serial ID should consist of four alpha-numeric characters.");
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_hide(d);
    gtk_widget_set_sensitive(widget, true);
    return;
  }
#endif
  /* Next, reboot the module and go on to the next page */
  Mobot_reboot(g_mobot);
  Mobot_disconnect(g_mobot);
  free(g_mobot);
  g_mobot = NULL;
  g_timeout_add(3000, switch_to_p3_timeout, NULL);
}

void on_button_flashAnother_clicked(GtkWidget* widget, gpointer data)
{
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "button_p1_next")),
      true);
  gtk_notebook_set_current_page(
      GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")),
      0);
}

