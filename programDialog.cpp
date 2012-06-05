#include <gtk/gtk.h>
#include <string.h>
#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include "RoboMancer.h"

void on_imagemenuitem_cut_activate(GtkWidget* widget, gpointer data)
{
  scintilla_send_message(g_sci, SCI_CUT, 0, 0);
}

void on_imagemenuitem_copy_activate(GtkWidget* widget, gpointer data)
{
  scintilla_send_message(g_sci, SCI_COPY, 0, 0);
}

void on_imagemenuitem_paste_activate(GtkWidget* widget, gpointer data)
{
  scintilla_send_message(g_sci, SCI_PASTE, 0, 0);
}

void on_imagemenuitem_open_activate(GtkWidget* widget, gpointer data)
{
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("Open File",
      GTK_WINDOW(g_window),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    //open_file (filename);
    g_free (filename);
  }
  gtk_widget_destroy (dialog);
}
