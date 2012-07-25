#include <stdlib.h>
#include <gtk/gtk.h>
#include "RoboMancer.h"
#include "thread_macros.h"

char g_tmpBuf[80];
bool g_dndConnect = true;

void on_button_connect_addRobot_clicked(GtkWidget* widget, gpointer data)
{
  GtkEntry* entry = GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_connect_newAddress"));
  const gchar* addr = gtk_entry_get_text(entry);
  g_robotManager->addEntry(addr);
  g_robotManager->write();
  refreshConnectDialog();
}

void on_button_connect_remove_clicked(GtkWidget* widget, gpointer data)
{
  int i = getConnectSelectedIndex();
  if(i < 0) return;
  g_robotManager->remove(i);
  g_robotManager->write();
  refreshConnectDialog();
  /* Select the next mobot automatically */
  GtkTreePath* treePath;
  GtkTreeView* availableBots = 
      GTK_TREE_VIEW(gtk_builder_get_object(g_builder, "treeview_availableRobots"));
  treePath = gtk_tree_path_new_from_indices(i, -1);
  //gtk_tree_view_row_activated(availableBots, treePath, column);
  GtkTreeSelection* selection = gtk_tree_view_get_selection(availableBots);
  gtk_tree_selection_select_path(selection, treePath);
}

void on_button_connect_moveUpAvailable_clicked(GtkWidget* widget, gpointer data)
{
  int i = getConnectSelectedIndex();
  if(i < 1) {
    return;
  }
  g_robotManager->moveEntryUp(i);
  g_robotManager->write();
  refreshConnectDialog();
  /* Select the moved index */
  GtkTreePath* treePath;
  GtkTreeView* availableBots = 
      GTK_TREE_VIEW(gtk_builder_get_object(g_builder, "treeview_availableRobots"));
  treePath = gtk_tree_path_new_from_indices(i-1, -1);
  //gtk_tree_view_row_activated(availableBots, treePath, column);
  GtkTreeSelection* selection = gtk_tree_view_get_selection(availableBots);
  gtk_tree_selection_select_path(selection, treePath);
}

void on_button_connect_moveDownAvailable_clicked(GtkWidget* widget, gpointer data)
{
  int i = getConnectSelectedIndex();
  if(i < 0) {
    return;
  }
  g_robotManager->moveEntryDown(i);
  g_robotManager->write();
  refreshConnectDialog();
  /* Select the moved index */
  GtkTreePath* treePath;
  GtkTreeView* availableBots = 
      GTK_TREE_VIEW(gtk_builder_get_object(g_builder, "treeview_availableRobots"));
  treePath = gtk_tree_path_new_from_indices(i+1, -1);
  //gtk_tree_view_row_activated(availableBots, treePath, column);
  GtkTreeSelection* selection = gtk_tree_view_get_selection(availableBots);
  gtk_tree_selection_select_path(selection, treePath);
}

struct connectThreadArg_s {
  int connectIndex;
  int connectionCompleted;
  int connectReturnVal;
};

void* connectThread(void* arg)
{
  struct connectThreadArg_s* a;
  a = (struct connectThreadArg_s*)arg;
  a->connectReturnVal = g_robotManager->connectIndex(a->connectIndex);
  a->connectionCompleted = 1;
}

gboolean progressBarConnectUpdate(gpointer data)
{
  static int counter = 0;
  static GtkListStore* liststore_available = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  GtkTreeIter iter;
  //GtkWidget* progressBarConnect = GTK_WIDGET(gtk_builder_get_object(g_builder, "progressbar_connect"));
  //GtkWidget* progressBarWindow = GTK_WIDGET(gtk_builder_get_object(g_builder, "window_connectProgress"));
  struct connectThreadArg_s* a;
  counter++;
  a = (struct connectThreadArg_s*)data;
  if(a->connectionCompleted) {
    //gtk_widget_hide(progressBarWindow);
    /* Check the connection status return value */
    GtkLabel* label = GTK_LABEL(gtk_builder_get_object(g_builder, "label_connectFailed"));
    switch(a->connectReturnVal) {
      case -1: 
        gtk_label_set_text(label, 
            "Connection failed: Remote device could not be found. Please check \n"
            "that the Mobot is turned on and the address is correct.");
        break;
      case -2:
        gtk_label_set_text(label,
            "Connection failed: Another device is already connected to the Mobot.");
        break;
      case -3:
        gtk_label_set_text(label,
            "Connection failed: The address format is incorrect. Please check that \n"
            "the address has been typed correctly.");
        break;
      case -4:
        gtk_label_set_text(label, 
            "Connection failed.");
        break;
      case -5:
        gtk_label_set_text(label,
            "Connection failed: Bluetooth device not found. Please plug in a Mobot \n"
            "compatible bluetooth dongle.");
        break;
      case -6:
        gtk_label_set_text(label,
            "Connection failed: The Mobot firmware version does not match the RoboMancer version. Please make sure that both your Mobot firmware and your RoboMancer software are up to date.");
      default:
        gtk_label_set_text(label,
            "Connection error.");
        break;
    } 
    if(a->connectReturnVal) {
      gtk_widget_show(
        GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectFailed")));
    }
    refreshConnectDialog();
    return FALSE;
  } else {
    //gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progressBarConnect));
    char buf[20];
    sprintf(buf, "%d", a->connectIndex);
    int rc = gtk_tree_model_get_iter_from_string(
        GTK_TREE_MODEL(liststore_available),
        &iter,
        buf);
    if(!rc) {
      /* Could not set iter for some reason... */
      return FALSE;
    }

    if(counter % 2) {
      gtk_list_store_set(liststore_available, &iter,
          0, 
          g_robotManager->getEntry(a->connectIndex),
          1, GTK_STOCK_DISCONNECT,
          -1 );
    } else {
      gtk_list_store_set(liststore_available, &iter,
          0, 
          g_robotManager->getEntry(a->connectIndex),
          1, GTK_STOCK_CONNECT,
          -1 );
    }
    return TRUE;
  }
}

int getConnectSelectedIndex()
{
  /* Get the index and/or text */
  GtkWidget* view =  GTK_WIDGET(gtk_builder_get_object(g_builder, "treeview_availableRobots"));
  GtkTreeModel* model = GTK_TREE_MODEL(gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  GtkTreeSelection* selection = gtk_tree_view_get_selection((GTK_TREE_VIEW(view)));
  GList* list = gtk_tree_selection_get_selected_rows(selection, &model);
  if(list == NULL) return -1;
  gint* paths = gtk_tree_path_get_indices((GtkTreePath*)list->data);
  int i = paths[0];
  g_list_foreach(list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free(list);
  return i;
}

void on_button_connect_connect_clicked(GtkWidget* widget, gpointer data)
{
  GtkWidget* progressBarWindow = GTK_WIDGET(gtk_builder_get_object(g_builder, "window_connectProgress"));
  GtkWidget* progressBarConnect = GTK_WIDGET(gtk_builder_get_object(g_builder, "progressbar_connect"));
  /* Get the index and/or text */
  int i = getConnectSelectedIndex();
  if(i < 0) {
    return;
  }

  /* Set the icon */
  static GtkListStore* liststore_available = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  GtkTreeIter iter;
  char buf[20];
  sprintf(buf, "%d", i);
  int rc = gtk_tree_model_get_iter_from_string(
      GTK_TREE_MODEL(liststore_available),
      &iter,
      buf);
  gtk_list_store_set(liststore_available, &iter,
      0, 
      g_robotManager->getEntry(i),
      1, GTK_STOCK_CONNECT,
      -1 );

  static struct connectThreadArg_s arg;
  arg.connectIndex = i;
  arg.connectionCompleted = 0;
  THREAD_T thread;
  THREAD_CREATE(&thread, connectThread, &arg);
  //gtk_widget_show(progressBarWindow);
  g_timeout_add(500, progressBarConnectUpdate, &arg);
}

void on_button_connect_disconnect_clicked(GtkWidget* widget, gpointer data)
{
  int i = getConnectSelectedIndex();
  if(i < 0) {
    return;
  }
  /* We have to lock the controlDialog locks first to make sure we don't screw
   * up their data. */
  MUTEX_LOCK(&g_activeMobotLock);
  g_robotManager->disconnect( i );
  g_activeMobot = NULL;
  MUTEX_UNLOCK(&g_activeMobotLock);
  refreshConnectDialog();
}

void on_button_connectFailedOk_clicked(GtkWidget* widget, gpointer data)
{
  GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectFailed"));
  gtk_widget_hide(w);
}

void on_liststore_availableRobots_row_deleted(
    GtkTreeModel* model,
    GtkTreePath* path,
    gpointer user_data)
{
  
  gint* indices;
  int depth, index;
  if(g_dndConnect) {
    indices = gtk_tree_path_get_indices_with_depth(path, &depth);
    index = indices[0];
    g_robotManager->remove(index);
    g_robotManager->write();
  }
}

void on_liststore_availableRobots_row_inserted(
    GtkTreeModel* model,
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer user_data)
{
  gint* indices;
  int depth, index;
  if(g_dndConnect) {
    /* Get the text entry from the liststore */
    gchar* addr;
    indices = gtk_tree_path_get_indices_with_depth(path, &depth);
    index = indices[0];
    gtk_tree_model_get(model, iter, 0, &addr, -1);
    if(addr == NULL) {
      addr = strdup("00:00:00:00:00:00");
    }
    g_robotManager->insertEntry(addr, index);
    free(addr);
    g_robotManager->write();
  }
}

void on_liststore_availableRobots_row_changed(
    GtkTreeModel* model,
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer user_data)
{
  gint* indices;
  int depth, index;
  /* Get the new value */
  gchar* addr;
  gtk_tree_model_get(model, iter, 0, &addr, -1);
  if(addr == NULL) {
    return;
  }
  indices = gtk_tree_path_get_indices_with_depth(path, &depth);
  index = indices[0];
  g_robotManager->rename(addr, index);
}

void refreshConnectDialog()
{
  static GtkListStore* liststore_available = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  static GtkListStore* liststore_connected = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_connectedRobots"));
  g_dndConnect = false;

  /* Clear the widgets */
  gtk_list_store_clear(liststore_available);
  gtk_list_store_clear(liststore_connected);

  /* Populate the widgets */
  int i;
  GtkTreeIter iter;
  GtkTreeIter connectedIter;
  for(i = 0; i < g_robotManager->numEntries(); i++) {
    gtk_list_store_append(liststore_available, &iter);
    if(g_robotManager->isConnected(i)) {
      /* Add it to the liststore of connected bots */
      gtk_list_store_append(liststore_connected, &connectedIter);
      gtk_list_store_set(liststore_connected, &connectedIter, 
          0, 
          g_robotManager->getEntry(i),
          -1);
      /* Set the blinky light icon to green */
      gtk_list_store_set(liststore_available, &iter,
          0, 
          g_robotManager->getEntry(i),
          1, GTK_STOCK_YES,
          -1 );
    } else {
      gtk_list_store_set(liststore_available, &iter,
          0, 
          g_robotManager->getEntry(i),
          1, GTK_STOCK_DISCONNECT,
          -1 );
    }
  }
  /* If there is only one entry, set that entry as active in the "Control
   * Robot" dialog. */
  /*
  if(g_robotManager->numConnected() == 1) {
    GtkWidget *w;
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_connectedRobots"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
  }
  */
  g_dndConnect = true;
}
