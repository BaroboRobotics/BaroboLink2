#include <gtk/gtk.h>
#include <string.h>
#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include "RoboMancer.h"
#include "RobotManager.h"
#ifdef __MACH__
#include <sys/types.h>
#include <unistd.h>
#include <gtk-mac-integration.h>
#endif
#include <sys/stat.h>

GtkBuilder *g_builder;
GtkWidget *g_window;
GtkWidget *g_scieditor;
ScintillaObject *g_sci;

CRobotManager *g_robotManager;

char *g_interfaceFiles[512] = {
  "interface/interface.glade",
  "interface.glade",
  "../share/RoboMancer/interface.glade",
  NULL,
  NULL
};

int main(int argc, char* argv[])
{
  GError *error = NULL;
 
  gtk_init(&argc, &argv);

  /* Create the GTK Builder */
  g_builder = gtk_builder_new();

#ifdef __MACH__
  char *datadir = getenv("XDG_DATA_DIRS");
  if(datadir != NULL) {
    g_interfaceFiles[3] = (char*)malloc(sizeof(char)*512);
    sprintf(g_interfaceFiles[3], "%s/RoboMancer/interface.glade", datadir);
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
  /* Initialize the subsystem */
  initialize();

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

void initialize()
{
  g_robotManager = new CRobotManager();
  /* Read the configuration file */
  g_robotManager->read( Mobot_getConfigFilePath() );

  refreshConnectDialog();
  //g_timeout_add(1000, connectDialogPulse, NULL);

  g_notebookRoot = GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook_root"));
  initControlDialog();
  initProgramDialog();
  initializeComms();
}

int getIterModelFromTreeSelection(GtkTreeView *treeView, GtkTreeModel **model, GtkTreeIter *iter)
{
  GtkTreeSelection *treeSelection;

  treeSelection = gtk_tree_view_get_selection( treeView );
  gtk_tree_selection_set_mode(treeSelection, GTK_SELECTION_BROWSE);

  bool success =
    gtk_tree_selection_get_selected( treeSelection, model, iter );
  if(!success) {
    return -1;
  }
  return 0;
}

void on_imagemenuitem_about_activate(GtkWidget *widget, gpointer data)
{
  /* Find the about dialog and show it */
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "aboutdialog"));
  gtk_dialog_run(GTK_DIALOG(w));
}

void on_aboutdialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
  gtk_widget_hide(GTK_WIDGET(dialog));
}

void on_aboutdialog_close(GtkDialog *dialog, gpointer user_data)
{
  gtk_widget_hide(GTK_WIDGET(dialog));
}

double normalizeAngleRad(double radians)
{
  while(radians > M_PI) {
    radians -= 2*M_PI;
  }
  while(radians < -M_PI) {
    radians += 2*M_PI;
  }
  return radians;
}
