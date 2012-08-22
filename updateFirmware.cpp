#include <string.h>
#include <gtk/gtk.h>
#include <mobot.h>
#include "RoboMancer.h"
#include "RecordMobot.h"

recordMobot_t* g_reflashMobot;
char g_reflashAddress[80];
int g_reflashHWRev;
GtkNotebook *g_notebookRoot;

gboolean listenButtonHWRev(gpointer data);

void on_button_updateFirmware_clicked(GtkWidget* widget, gpointer data)
{
  /* Try and grab the HW rev number from the robot */
  int index = getConnectSelectedIndex();
  recordMobot_t* mobot = g_robotManager->getMobotIndex(index);
  if(mobot == NULL) {
    return;
  }
  strcpy(g_reflashAddress, RecordMobot_getAddress(mobot));
  g_notebookRoot = GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook_root"));
  int hwRev;
  int rc = Mobot_getHWRev((mobot_t*)mobot, &hwRev);
  if(rc) {
    /* Need to determine revision number by button press */
    g_reflashMobot = mobot;
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
    gtk_notebook_set_current_page(g_notebookRoot, 2);
  }
  return rc;
}
