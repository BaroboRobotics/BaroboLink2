#include <gtk/gtk.h>
#include "RoboMancer.h"

void on_button_connect_addRobot_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_remove_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_moveUpAvailable_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_moveDownAvailable_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_connect_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_moveUpConnected_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_moveDownConnected_clicked(GtkWidget* widget, gpointer data)
{
}

void on_button_connect_disconnect_clicked(GtkWidget* widget, gpointer data)
{
}

void refreshConnectDialog()
{
  static GtkListStore* liststore_available = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  static GtkListStore* liststore_connected = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_connectedRobots"));

  /* Clear the widgets */
  gtk_list_store_clear(liststore_available);
  gtk_list_store_clear(liststore_connected);

  /* Populate the widgets */
  int i;
  GtkTreeIter iter;
  for(i = 0; i < g_robotManager->numAvailable(); i++) {
    gtk_list_store_append(liststore_available, &iter);
    gtk_list_store_set(liststore_available, &iter,
        0, 
        g_robotManager->getEntry( g_robotManager->availableIndexToIndex(i)),
        -1 );
  }
}
