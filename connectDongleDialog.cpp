#include "mobot.h"
#include "RoboMancer.h"

void askConnectDongle(void)
{
  /* Set up a question dialog */
  GtkWidget* d = gtk_message_dialog_new(
      GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_YES_NO,
      "There is currently no Mobot dongle associated with RoboMancer. Would you like to add one now?");
  int rc = gtk_dialog_run(GTK_DIALOG(d));
  if(rc == GTK_RESPONSE_YES) {
    showConnectDongleDialog();
  }
  gtk_widget_destroy(d);
}

void showConnectDongleDialog(void)
{
  GdkColor color;
  GtkWidget *w;
  /* First, see if the dongle is connected */
  if( (g_mobotParent != NULL) && (((mobot_t*)g_mobotParent)->connected != 0)) {
    /* The dongle is connected */
    /* Make the text entry green */
    gdk_color_parse("green", &color);
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_connectDongleCurrentPort"));
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL, &color);
  } else {
    /* The dongle is not connected */
    gdk_color_parse("red", &color);
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_connectDongleCurrentPort"));
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL, &color);
    gtk_entry_set_text(GTK_ENTRY(w), "<Not Connected>");
  }
  GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectDongle"));
  gtk_widget_show(window);
}

void hideConnectDongleDialog(void)
{
  GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectDongle"));
  gtk_widget_hide(window);
}

void on_button_connectDongleConnect_clicked(GtkWidget *w, gpointer data)
{
  char buf[256];
  GtkRadioButton *connectAutomaticallyButton = 
    GTK_RADIO_BUTTON(gtk_builder_get_object(g_builder, "radiobutton_connectDongleAuto"));
  GtkRadioButton *connectManuallyButton = 
    GTK_RADIO_BUTTON(gtk_builder_get_object(g_builder, "radiobutton_connectDongleManual"));
  GtkEntry *manualComPort = GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_connectDongleManual"));
  GtkEntry *currentComPort = GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_connectDongleCurrentPort"));

  /* Check to see if the auto button is pressed */
  if(
      gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(connectAutomaticallyButton))
    )
  {
    /* If the auto-detect button is pressed */
    int i;
    bool dongleFound = false;
    for(i = 0; i < 64; i++) {
#ifndef _WIN32
      sprintf(buf, "/dev/ttyACM%d", i);
#else
      sprintf(buf, "COM%d", i);
#endif
      if(!Mobot_connectWithTTY((mobot_t*)g_mobotParent, buf)) {
        /* We found the TTY port. */
        gtk_entry_set_text(currentComPort, buf);
        g_robotManager->addDongle(buf);
        g_robotManager->write();
        dongleFound = true;
        Mobot_setDongleMobot((mobot_t*)g_mobotParent);
        break;
      }
    }
    if(!dongleFound) {
      /* Display a warning/error dialog */
      GtkWidget* d = gtk_message_dialog_new(
          GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_CLOSE,
          "No dongle detected. Please make sure that a Mobot is currently "
          "connected to the computer with a USB cable and turned on.");
      gtk_dialog_run(GTK_DIALOG(d));
      gtk_widget_destroy(d);
    }
  } else if (
      gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(connectManuallyButton))
      )
  {
    /* If the manual selection is pressed */
    /* Get the entry text */
    const char* port = gtk_entry_get_text(manualComPort);
    printf("Connecting to port %s\n", port);
    if(Mobot_connectWithTTY((mobot_t*)g_mobotParent, port)) {
      GtkLabel* errLabel = GTK_LABEL(gtk_builder_get_object(g_builder, "label_connectFailed"));
      sprintf(buf, "Error: Could not connect to dongle at %s.\n", port);
      gtk_label_set_text(errLabel, buf);
      GtkWidget* errWindow = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectFailed"));
      gtk_widget_show(errWindow);
    } else {
      gtk_entry_set_text(currentComPort, port);
      g_robotManager->addDongle(port);
      g_robotManager->write();
      Mobot_setDongleMobot((mobot_t*)g_mobotParent);
    }
  } else {
    /* Error */
    return;
  }
}

void on_button_connectDongleClose_clicked(GtkWidget *w, gpointer data)
{
  hideConnectDongleDialog();
  return;
}
