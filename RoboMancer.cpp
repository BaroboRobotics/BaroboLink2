#include <gtk/gtk.h>
#include <string.h>
#include "RoboMancer.h"
#include "RobotManager.h"

/* These store the embedded glade xml file */
extern const char _binary_interface_interface_glade_start[];
extern size_t _binary_interface_interface_glade_size;
extern const char _binary_interface_interface_glade_end[];

GtkBuilder *g_builder;
GtkWidget *g_window;

CRobotManager *g_robotManager;

int main(int argc, char* argv[])
{
  GError *error = NULL;

  gtk_init(&argc, &argv);

  /* Create the GTK Builder */
  g_builder = gtk_builder_new();

  /* Load the UI */
  if( ! gtk_builder_add_from_string(g_builder, _binary_interface_interface_glade_start, strlen(_binary_interface_interface_glade_start), &error) )
  {
    g_warning("%s", error->message);
    //g_free(error);
    return -1;
  }

  /* Initialize the subsystem */
  initialize();

  /* Get the main window */
  g_window = GTK_WIDGET( gtk_builder_get_object(g_builder, "window1"));
  /* Connect signals */
  gtk_builder_connect_signals(g_builder, NULL);
  /* Show the window */
  gtk_widget_show(g_window);
  gtk_main();
  return 0;
}

void initialize()
{
  g_robotManager = new CRobotManager();
  /* Read the configuration file */
  g_robotManager->read( CMobot::getConfigFilePath() );

  refreshConnectDialog();
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
