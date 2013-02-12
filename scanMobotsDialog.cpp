#include "RoboMancer.h"

int g_scanMobotsActive = 0;

void* scanMobotsThread(void* arg)
{
}

void showScanMobotsDialog()
{
  if(!g_scanMobotsActive) {
    /* If we're not currently scanning for mobots, start a scan */
  }
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_scanMobots")));
}

void hideScanMobotsDialog()
{
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_scanMobots")));
}

void scanmobots_foreach_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  /* Get each serial ID and check to see if it is already in the RobotManager */
  char* str;
  gtk_tree_model_get(model, iter, 0, &str, -1);
  if(!g_robotManager->entryExists(str)) {
    g_robotManager->addEntry(str);
  }
  free(str);
}

void on_button_scanMobotsAdd_clicked(GtkWidget *w, gpointer data)
{
  /* Go through each selection and add it to the robotmanager */
  GtkTreeView* treeview;
  treeview  = GTK_TREE_VIEW(gtk_builder_get_object(g_builder, "treeview_scanMobots"));
  GtkTreeSelection* selection;
  selection = gtk_tree_view_get_selection(treeview);
  gtk_tree_selection_selected_foreach(selection, scanmobots_foreach_callback, NULL);
}

void on_button_scanMobotsOK_clicked(GtkWidget *w, gpointer data)
{
}

void on_button_scanMobotsCancel_clicked(GtkWidget *w, gpointer data)
{
}

void on_button_scanMobotsRefresh_clicked(GtkWidget *w, gpointer data)
{
}
