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

