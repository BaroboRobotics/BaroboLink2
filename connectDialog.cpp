#include <stdlib.h>
#include <gtk/gtk.h>
#include "RoboMancer.h"
#include "thread_macros.h"
#include "mobot.h"

char g_tmpBuf[80];
bool g_dndConnect = true;
char g_connectedPort[64];

recordMobot_t* g_mobotParent;

void on_button_connect_addRobot_clicked(GtkWidget* widget, gpointer data)
{
  GtkEntry* entry = GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_connect_newAddress"));
  const gchar* addr = gtk_entry_get_text(entry);
  g_robotManager->addEntry(addr);
  g_robotManager->write();
  refreshConnectDialog();
}

void on_button_scanMobots_clicked(GtkWidget* widget, gpointer data)
{
  if(g_mobotParent == NULL) 
  {
    g_mobotParent = (recordMobot_t*)malloc(sizeof(recordMobot_t));
    RecordMobot_init(g_mobotParent, "Dongle");
  }

  if(((mobot_t*)g_mobotParent)->connected == 0) {
    askConnectDongle();
    return;
  }

  /* Start the query thread and show the progress bar */
  showScanMobotsDialog();
  /*
  gtk_widget_show(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "window_scanningProgress"))
      );
  THREAD_T thread;
  THREAD_CREATE(&thread, scanThread, &completed);
  g_timeout_add(500, progressBarScanningUpdate, &completed);
  */
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
  static int counter[30];
  static GtkListStore* liststore_available = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  GtkTreeIter iter;
  //GtkWidget* progressBarConnect = GTK_WIDGET(gtk_builder_get_object(g_builder, "progressbar_connect"));
  //GtkWidget* progressBarWindow = GTK_WIDGET(gtk_builder_get_object(g_builder, "window_connectProgress"));
  struct connectThreadArg_s* a;
  a = (struct connectThreadArg_s*)data;
  counter[a->connectIndex]++;
  if(a->connectionCompleted) {
    char *buf = (char*)malloc(1024);
    //gtk_widget_hide(progressBarWindow);
    /* Check the connection status return value */
    GtkLabel* label = GTK_LABEL(gtk_builder_get_object(g_builder, "label_connectFailed"));
    switch(a->connectReturnVal) {
      case -1: 
        sprintf(buf, 
            "Connection to %s failed: Remote device could not be found. Please check \n"
            "that the Mobot is turned on and the address is correct.",
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label, buf);
        break;
      case -2:
        sprintf(buf,
            "Connection to %s failed: Another device is already connected to the Mobot.",
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label, buf);
        break;
      case -3:
        sprintf(buf, 
            "Connection to %s failed: The address format is incorrect. Please check that \n"
            "the address has been typed correctly.",
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label,buf);
        break;
      case -4:
        sprintf(buf,
            "Connection to %s failed.",
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label, buf);
        break;
      case -5:
        sprintf(buf,
            "Connection to %s failed: Bluetooth device not found. Please plug in a Mobot \n"
            "compatible bluetooth dongle.",
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label,buf);
        break;
      case -6:
        sprintf(buf,
            "Connection to %s failed: The Mobot firmware version does not match the \n"
            "RoboMancer version. Please make sure that both your Mobot firmware and \n"
            "your RoboMancer software are up to date.",
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label,buf);
      default:
        sprintf(buf, 
            "Connection to %s failed.", 
            g_robotManager->getEntry(a->connectIndex));
        gtk_label_set_text(label, buf);
        break;
    } 
    if(a->connectReturnVal) {
      gtk_widget_show(
        GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectFailed")));
    }
    refreshConnectDialog();
    free(a);
    free(buf);
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

    if(counter[a->connectIndex] % 2) {
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
  return FALSE;
}

gboolean connectDialogPulse(gpointer data)
{
  refreshConnectDialog();
  return TRUE;
}

void on_button_Connect_clicked(GtkWidget* w, gpointer data)
{
  int index = (long)data;
  struct connectThreadArg_s* arg;
  /* First, check to see if the requested mobot is a DOF. If it is, we must
   * ensure that the dongle has been connected. */
  if(
      (
       (strlen(g_robotManager->getEntry(index)) == 4) &&
      g_mobotParent == NULL
      ) ||
      (
       ((mobot_t*)g_mobotParent)->connected == 0
      )
    )
    {
      /* Trying to connect to a DOF, but there is no dongle connected. Open the
       * connect-dongle dialog */
      g_mobotParent = (recordMobot_t*)malloc(sizeof(recordMobot_t));
      RecordMobot_init(g_mobotParent, "Dongle");
      askConnectDongle();
      return;
    }
  arg = (struct connectThreadArg_s*)malloc(sizeof(struct connectThreadArg_s));
  arg->connectIndex = index;
  arg->connectionCompleted = 0;
  gtk_widget_set_sensitive(w, FALSE);
  THREAD_T thread;
  THREAD_CREATE(&thread, connectThread, arg);
  g_timeout_add(500, progressBarConnectUpdate, arg);
}

void on_button_Disconnect_clicked(GtkWidget* w, gpointer data)
{
  int index = (long) data;
  /* We have to lock the controlDialog locks first to make sure we don't screw
   * up their data. */
  MUTEX_LOCK(&g_activeMobotLock);
  g_robotManager->disconnect( index );
  g_activeMobot = NULL;
  MUTEX_UNLOCK(&g_activeMobotLock);
  refreshConnectDialog();
}

void on_button_Remove_clicked(GtkWidget* w, gpointer data)
{
  int index = (long) data;
  /* First, make sure the robot is disconnected */
  g_robotManager->disconnect(index);
  g_robotManager->remove(index);
  g_robotManager->write();
  refreshConnectDialog();
}

void on_button_MoveUp_clicked(GtkWidget* w, gpointer data)
{
  int index = (long)data;
  g_robotManager->moveEntryUp(index);
  g_robotManager->write();
  refreshConnectDialog();
}

void on_button_MoveDown_clicked(GtkWidget* w, gpointer data)
{
  int index = (long)data;
  g_robotManager->moveEntryDown(index);
  g_robotManager->write();
  refreshConnectDialog();
}

void on_button_connect_connect_clicked(GtkWidget* widget, gpointer data)
{
#if 0
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

  struct connectThreadArg_s* arg;
  arg = (struct connectThreadArg_s*)malloc(sizeof(struct connectThreadArg_s));
  arg->connectIndex = i;
  arg->connectionCompleted = 0;
  THREAD_T thread;
  THREAD_CREATE(&thread, connectThread, arg);
  //gtk_widget_show(progressBarWindow);
  g_timeout_add(500, progressBarConnectUpdate, arg);
#endif
}

void on_button_connect_disconnect_clicked(GtkWidget* widget, gpointer data)
{
#if 0
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
#endif
}

void on_button_connectFailedOk_clicked(GtkWidget* widget, gpointer data)
{
  GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectFailed"));
  gtk_widget_hide(w);
}

void refreshConnectDialog()
{
  /* Create the GtkTable */
  static GtkWidget *rootTable = NULL;
  if(rootTable != NULL) {
    gtk_widget_destroy(rootTable);
  }
  rootTable = gtk_table_new(
      g_robotManager->numEntries()*3,
      8,
      FALSE);
  /* For each Mobot entry, we need to compose a set of child widgets and attach
   * them to the right places on the grid */
  int i;
  GtkWidget *w;
  for(i = 0; i < g_robotManager->numEntries(); i++) {
    /* Make a new label for the entry */
    w = gtk_label_new(g_robotManager->getEntry(i));
    gtk_widget_show(w);
    gtk_table_attach( GTK_TABLE(rootTable),
        w,
        0, 1, //columns
        i*3, (i*3)+2, //rows
        GTK_FILL, GTK_FILL,
        2, 2);
    /* Add connect/connecting/disconnect button */
    recordMobot_t* mobot;
    if(mobot = g_robotManager->getMobotIndex(i)) {
      switch(mobot->connectStatus) {
        case RMOBOT_NOT_CONNECTED:
          w = gtk_button_new_with_label("Connect");
          gtk_widget_show(w);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              2, 3,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          /* Attach the connect/disconnect button signal handler */
          g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_Connect_clicked), (void*)i);
          /* Add an image denoting connection status for each one */
          w = gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
          gtk_widget_show(w);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              1, 2,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          break;
        case RMOBOT_CONNECTING:
          w = gtk_button_new_with_label("Connecting...");
          gtk_widget_show(w);
          gtk_widget_set_sensitive(w, FALSE);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              2, 3,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          /* Add an image denoting connection status for each one */
          w = gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
          gtk_widget_show(w);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              1, 2,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          break;
        case RMOBOT_CONNECTED:
          w = gtk_button_new_with_label("Disconnect");
          gtk_widget_show(w);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              2, 3,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          /* Attach the connect/disconnect button signal handler */
          g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_Disconnect_clicked), (void*)i);
          /* Add an image denoting connection status for each one */
          w = gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
          gtk_widget_show(w);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              1, 2,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          break;
        default:
          w = gtk_button_new_with_label("Meh?");
          gtk_widget_show(w);
          gtk_table_attach( GTK_TABLE(rootTable),
              w,
              2, 3,
              i*3, (i*3)+2,
              GTK_FILL, GTK_FILL,
              2, 2);
          break;
      }
    } else {
      w = gtk_button_new_with_label("Connect");
      gtk_widget_show(w);
      gtk_table_attach( GTK_TABLE(rootTable),
          w,
          2, 3,
          i*3, (i*3)+2,
          GTK_FILL, GTK_FILL,
          2, 2);
      /* Attach the connect/disconnect button signal handler */
      g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_Connect_clicked), (void*)i);
      /* Add an image denoting connection status for each one */
      w = gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
      gtk_widget_show(w);
      gtk_table_attach( GTK_TABLE(rootTable),
          w,
          1, 2,
          i*3, (i*3)+2,
          GTK_FILL, GTK_FILL,
          2, 2);
    }
    /* Add remove button */
    w = gtk_button_new_with_label("Remove");
    gtk_widget_show(w);
    gtk_table_attach( GTK_TABLE(rootTable),
        w,
        3, 4,
        i*3, (i*3)+2,
        GTK_FILL, GTK_FILL,
        2, 2);
    g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_Remove_clicked), (void*)i);
    /* Add move-up button */
    w = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
    gtk_widget_show(w);
    gtk_table_attach( GTK_TABLE(rootTable),
        w,
        4, 5,
        i*3, (i*3)+1,
        GTK_FILL, GTK_FILL,
        2, 2);
    g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_MoveUp_clicked), (void*)i);
    /* Add move-down button */
    w = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
    gtk_widget_show(w);
    gtk_table_attach( GTK_TABLE(rootTable),
        w,
        4, 5,
        (i*3)+1, (i*3)+2,
        GTK_FILL, GTK_FILL,
        2, 2);
    g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_MoveDown_clicked), (void*)i);
    /* Maybe add a color button */
    int form;
    if( g_robotManager->getMobotIndex(i) != NULL ) {
      if(!Mobot_getFormFactor((mobot_t*)g_robotManager->getMobotIndex(i), &form)) {
        if( 
            (form == MOBOTFORM_I) ||
            (form == MOBOTFORM_L) ||
            (form == MOBOTFORM_T)
          )
        {
          double r, g, b;
          int _r, _g, _b;
          char buf[16];
          if(!Mobot_getRGB((mobot_t*)g_robotManager->getMobotIndex(i), &r, &g, &b)) {
            _r = r * 255.0;
            _g = g * 255.0;
            _b = b * 255.0;
            sprintf(buf, "#%2X%2X%2X", _r, _g, _b);
            GdkColor color;
            gdk_color_parse(buf, &color);
            w = gtk_color_button_new_with_color(&color);
            gtk_widget_show(w);
            gtk_table_attach( GTK_TABLE(rootTable),
                w,
                5, 6,
                i*3, (i*3)+2,
                GTK_FILL, GTK_FILL,
                2, 2);
            g_signal_connect(
                G_OBJECT(w), 
                "color-set", 
                G_CALLBACK(on_colorDialog_color_set),
                (void*)g_robotManager->getMobotIndex(i)
                );
          }
        }
      }
    }
    /* Maybe add an "Upgrade Firmware" button */
    if( (g_robotManager->getMobotIndex(i) != NULL) &&
        (g_robotManager->getMobotIndex(i)->firmwareVersion < Mobot_protocolVersion()) ) {
      GdkColor color;
      gdk_color_parse("yellow", &color);
      w = gtk_button_new_with_label("Upgrade\nFirmware");
      gtk_widget_modify_bg(w, GTK_STATE_NORMAL, &color);
      gdk_color_parse("#FFFF22", &color);
      gtk_widget_modify_bg(w, GTK_STATE_PRELIGHT, &color);
      gtk_widget_show(w);
      gtk_table_attach( GTK_TABLE(rootTable),
          w,
          6, 7,
          i*3, (i*3)+2,
          GTK_FILL, GTK_FILL,
          2, 2);
      g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(on_button_updateFirmware_clicked), (void*)i);
    }
    /* Add a horizontal separator */
    w = gtk_hseparator_new();
    gtk_widget_show(w);
    gtk_table_attach( GTK_TABLE(rootTable),
        w,
        0, 6,
        i*3+2, (i*3)+3,
        GTK_FILL, GTK_FILL,
        2, 2);
  }
  GtkRequisition sizeRequest;
  gtk_widget_size_request(rootTable, &sizeRequest);
  GtkWidget *layout = GTK_WIDGET(gtk_builder_get_object(g_builder, "layout_connectDialog"));
  gtk_layout_set_size(GTK_LAYOUT(layout), sizeRequest.width, sizeRequest.height);
  gtk_layout_put(GTK_LAYOUT(layout), rootTable, 0, 0);
  gtk_widget_show(rootTable);

  /* Refresh the list stores, etc. */
  static GtkListStore* liststore_available = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_availableRobots"));
  static GtkListStore* liststore_connected = GTK_LIST_STORE(
      gtk_builder_get_object(g_builder, "liststore_connectedRobots"));
  g_dndConnect = false;

  /* Clear the widgets */
  gtk_list_store_clear(liststore_available);
  gtk_list_store_clear(liststore_connected);

  /* Populate the widgets */
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
      /* Set the update progress bar data */
      //printf("%d:%d\n", g_robotManager->getMobotIndex(i)->firmwareVersion, Mobot_protocolVersion());
      if(g_robotManager->getMobotIndex(i)->firmwareVersion < Mobot_protocolVersion()) {
        gtk_list_store_set(liststore_available, &iter,
            2, TRUE, 3, 0, -1);
      } else {
        gtk_list_store_set(liststore_available, &iter,
            2, FALSE, 3, 0, -1);
      }
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

void on_colorDialog_color_set(GtkColorButton* w, gpointer data)
{
  mobot_t* mobot = (mobot_t*)data;
  GdkColor color;
  gtk_color_button_get_color(w, &color);
  double rgb[3];
  rgb[0] = color.red/65535.0;
  rgb[1] = color.green/65535.0;
  rgb[2] = color.blue/65535.0;
  printf("%lf %lf %lf\n", rgb[0], rgb[1], rgb[2]);
  Mobot_setRGB(mobot, rgb[0], rgb[1], rgb[2]);
}
