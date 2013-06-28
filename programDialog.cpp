/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   BaroboLink is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BaroboLink is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BaroboLink.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include "BaroboLink.h"
#ifdef _MSYS
#include <windows.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
//#include <Python.h>
#include <fcntl.h>
#include "thread_macros.h"

#ifndef _MSYS
#define MAX_PATH 512
#endif

char *g_curFileName = NULL;
char *g_exportFileName = NULL;
bool g_dirty = false;
bool g_reexportFlag = false;
bool g_programRunning = false;

bool g_enableBackspace = true;
char g_userChars[256];
int g_numUserChars = 0;
int g_numUserCharsValid = 0;

GtkWidget *g_textview_programMessages;
GtkTextBuffer *g_textbuffer_programMessages;
int g_pipefd_stdout[2]; // Pipe for reading stdio from python/client program
int g_pipefd_stderr[2]; 
int g_pipefd_stdin[2]; // Pipe for pushing data to client program's stdin

void initProgramDialog(void)
{
  /* First, make sure there actually is a programming dialog */
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "notebook1"));
  int numPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(w));
  if(numPages == 3) {
    return;
  }
  /* Add Scintilla */
  g_scieditor = scintilla_new();
  g_sci = SCINTILLA(g_scieditor);
  GtkWidget *container = GTK_WIDGET(gtk_builder_get_object(g_builder, "alignment2"));
  gtk_container_add(GTK_CONTAINER(container), g_scieditor);
  scintilla_set_id(g_sci, 0);
  //gtk_widget_set_usize(g_scieditor, 100, 300);
  gtk_widget_show(g_scieditor);

  /* Attach signal handlers */
  g_signal_connect(G_OBJECT(g_scieditor), SCINTILLA_NOTIFY, G_CALLBACK(on_scintilla_notify), NULL);
  /* Set a monospace font */
  scintilla_send_message(
      g_sci,
      SCI_STYLESETFONT,
      STYLE_DEFAULT,
      (sptr_t)"!Courier");
  scintilla_send_message(
      g_sci,
      SCI_STYLESETSIZE,
      STYLE_DEFAULT,
      (sptr_t)10);
#define SSM(m, w, l) scintilla_send_message(g_sci, m, w, l)
   SSM(SCI_SETLEXER, SCLEX_PYTHON, 0);
   SSM(SCI_SETKEYWORDS, 0, (sptr_t)"int char float double Linkbot barobo if else for while");
   SSM(SCI_STYLESETFORE, SCE_C_COMMENT, 0x008000);
   SSM(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x008000);
   SSM(SCI_STYLESETFORE, SCE_C_NUMBER, 0x808000);
   SSM(SCI_STYLESETFORE, SCE_C_WORD, 0x800000);
   SSM(SCI_STYLESETFORE, SCE_C_STRING, 0x800080);
   SSM(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1);
   SSM(SCI_SETSTYLING, SCE_C_OPERATOR, 1);
   SSM(SCI_SETMARGINWIDTHN, 0, 40);
#undef SSM

  /* Initialize "external" scintilla editor */
  /* Add Scintilla */
  g_scieditor_ext = scintilla_new();
  g_sci_ext = SCINTILLA(g_scieditor_ext);
  container = GTK_WIDGET(gtk_builder_get_object(g_builder, "alignment12"));
  gtk_container_add(GTK_CONTAINER(container), g_scieditor_ext);
  scintilla_set_id(g_sci_ext, 0);
  //gtk_widget_set_usize(g_scieditor_ext, 100, 300);
  gtk_widget_show(g_scieditor_ext);

  /* Attach signal handlers */
  g_signal_connect(G_OBJECT(g_scieditor_ext), SCINTILLA_NOTIFY, G_CALLBACK(on_scintilla_notify), NULL);
  /* Set a monospace font */
  scintilla_send_message(
      g_sci_ext,
      SCI_STYLESETFONT,
      STYLE_DEFAULT,
      (sptr_t)"!Courier");
  scintilla_send_message(
      g_sci_ext,
      SCI_STYLESETSIZE,
      STYLE_DEFAULT,
      (sptr_t)10);
#define SSM(m, w, l) scintilla_send_message(g_sci_ext, m, w, l)
  SSM(SCI_SETLEXER, SCLEX_PYTHON, 0);
  SSM(SCI_SETKEYWORDS, 0, (sptr_t)"int char float double Linkbot barobo if else for while");
  SSM(SCI_STYLESETFORE, SCE_C_COMMENT, 0x008000);
  SSM(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x008000);
  SSM(SCI_STYLESETFORE, SCE_C_NUMBER, 0x808000);
  SSM(SCI_STYLESETFORE, SCE_C_WORD, 0x800000);
  SSM(SCI_STYLESETFORE, SCE_C_STRING, 0x800080);
  SSM(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1);
  SSM(SCI_SETSTYLING, SCE_C_OPERATOR, 1);
  SSM(SCI_SETMARGINWIDTHN, 0, 40);
#undef SSM
  /* Set the size of the external editor window */
  gtk_widget_set_size_request(GTK_WIDGET(g_scieditor_ext), 400, 400);

  /* Create text tag for error text in messages window */
  GtkWidget *textview = GTK_WIDGET(gtk_builder_get_object(g_builder, "textview_programMessages"));
  GtkTextBuffer *tb = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview)));
  gtk_text_buffer_create_tag(tb, "stderr", "foreground", "#FF0000", NULL);

  on_imagemenuitem_new_activate(NULL, NULL);
  g_textview_programMessages = GTK_WIDGET(gtk_builder_get_object(g_builder, "textview_programMessages"));
  g_textbuffer_programMessages = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_textview_programMessages)));
#if 0
  if(
      (pipe2(g_pipefd_stdout, O_NONBLOCK) == -1) ||
      (pipe2(g_pipefd_stdin, 0) == -1) ||
      (pipe2(g_pipefd_stderr, O_NONBLOCK) == -1) 
      ) 
  {
    perror("pipe");
    exit(-1);
  }
  dup2(g_pipefd_stdout[1], STDOUT_FILENO);
  dup2(g_pipefd_stderr[1], STDERR_FILENO);
  dup2(g_pipefd_stdin[0], STDIN_FILENO);
  g_timeout_add(500, check_io_timeout, NULL);
#endif
}

void check_save_on_dirty()
{
  if(g_dirty) {
    GtkWidget *w;
    w = gtk_message_dialog_new(GTK_WINDOW(g_window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        "There are unsaved changes in this file. Would you like to save?");
    gint result = gtk_dialog_run(GTK_DIALOG(w));
    if(result == GTK_RESPONSE_YES) {
      on_imagemenuitem_save_activate(NULL, NULL);
    }
    gtk_widget_destroy(w);
  }
}

void on_imagemenuitem_new_activate(GtkWidget* widget, gpointer data)
{
  check_save_on_dirty();

  if(g_curFileName != NULL) {
    free(g_curFileName);
    g_curFileName = NULL;
  }
  /* Clear the document */
  scintilla_send_message(g_sci, SCI_CLEARALL, 0, 0);
  /* Set up the initial code stub */
  scintilla_send_message(g_sci, SCI_INSERTTEXT, 0, (sptr_t)
      "#!/usr/bin/env python\n"
      "from barobo.linkbot import *\n"
      "\n"
      "myLinkbot = Linkbot()\n"
      "myLinkbot.connect()\n"
      "\n"
      "myLinkbot.move(90, 90, 90)\n"
      "myLinkbot.move(-90, -90, -90)\n"
      );
  scintilla_send_message(g_sci, SCI_EMPTYUNDOBUFFER, 0, 0);
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_undo"));
  gtk_widget_set_sensitive(w, false);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_redo"));
  gtk_widget_set_sensitive(w, false);
}

void on_imagemenuitem_open_activate(GtkWidget* widget, gpointer data)
{
  check_save_on_dirty();
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
    /* Load the contents of the file into the scintilla window. First, load the
     * whole file contents into a character buffer. */
    char *contents;
    struct stat s;
    stat(filename, &s);
    FILE *fp;
    fp = fopen(filename, "rb");
    if(fp == NULL) {
      /* FIXME: Should pop up a warning dialog or something */
      return;
    }
    contents = (char*)malloc(s.st_size+1);
    fread(contents, s.st_size, 1, fp);
    contents[s.st_size-1] = '\0';
    contents[s.st_size] = '\0';
    scintilla_send_message(g_sci, SCI_SETTEXT, 0, (sptr_t)contents);
    fclose(fp);
    free(contents);
    
    g_curFileName = strdup(filename);
    g_free (filename);
  }
  gtk_widget_destroy (dialog);
}

void save_to_file(const char* filename)
{
  /* Open the file for writing */
  FILE* fp;
  fp = fopen(filename, "wb");
  if(fp == NULL) {
    /* Could not open the file. Pop up an error message */
    GtkWidget* dialog;
    dialog = gtk_message_dialog_new (GTK_WINDOW(g_window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "Error saving to file '%s': %s",
        filename, g_strerror (errno));

    /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
    g_signal_connect_swapped (dialog, "response",
        G_CALLBACK (gtk_widget_destroy),
        dialog);
    return;
  }
  int sourceCodeSize;
  char* sourceCode;
  sourceCodeSize = scintilla_send_message(g_sci, SCI_GETLENGTH, 0, 0) + 1;
  sourceCode = (char*)malloc(sourceCodeSize);
  scintilla_send_message(g_sci, SCI_GETTEXT, sourceCodeSize, (sptr_t)sourceCode);
  /* Write it to the file */
  fwrite(sourceCode, sourceCodeSize-1, sizeof(char), fp); 
  /* Close the file */
  fclose(fp);
  free(sourceCode);
  /* Set the scintilla save point */
  scintilla_send_message(g_sci, SCI_SETSAVEPOINT, 0, 0);
  return;
}

void on_imagemenuitem_save_activate(GtkWidget* widget, gpointer data)
{
  /* if there is a current file name, just save there */
  if(g_curFileName != NULL) {
    save_to_file(g_curFileName);
  } else {
    on_imagemenuitem_saveAs_activate(widget, data);
  }
}

void on_imagemenuitem_saveAs_activate(GtkWidget* widget, gpointer data)
{
  /* Open a save file dialog */
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("Save File",
      GTK_WINDOW(g_window),
      GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
      NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  if (g_curFileName == NULL)
  {
    //gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), default_folder_for_saving);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "UntitledDocument");
  }
  else
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), g_curFileName);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    save_to_file (filename);
    if(g_curFileName != NULL) {
      free(g_curFileName);
    }
    g_curFileName = strdup(filename);
    g_free (filename);
  }
  gtk_widget_destroy (dialog);
}

void on_imagemenuitem_undo_activate(GtkWidget* widget, gpointer data)
{
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_undo"));
  scintilla_send_message(g_sci, SCI_UNDO, 0, 0);
  if( scintilla_send_message(g_sci, SCI_CANUNDO, 0, 0) ) {
    gtk_widget_set_sensitive(w, true);
  } else {
    gtk_widget_set_sensitive(w, false);
  }
}

void on_imagemenuitem_redo_activate(GtkWidget* widget, gpointer data)
{
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_redo"));
  scintilla_send_message(g_sci, SCI_REDO, 0, 0);
  if( scintilla_send_message(g_sci, SCI_CANREDO, 0, 0) ) {
    gtk_widget_set_sensitive(w, true);
  } else {
    gtk_widget_set_sensitive(w, false);
  }
}

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

void on_button_exportExe_clicked(GtkWidget* widget, gpointer data)
{
#ifdef _MSYS
  static bool path_set = false;
  static char lastFilename[256] = "";
  GtkWidget *dialog;
  char *exportExecutableFileName;
  /* Make sure the file is saved */
  if(g_curFileName == NULL) {
    dialog = gtk_message_dialog_new(
        GTK_WINDOW(g_window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "File not yet saved. Save the source code to a file to continue.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }
  on_imagemenuitem_save_activate(NULL, NULL);
  if(g_curFileName == NULL) {
    return;
  }
  if(g_exportFileName != NULL && g_reexportFlag) {
    exportExecutableFileName = (char*)g_malloc(sizeof(char) * (strlen(g_exportFileName)+1));
    strcpy(exportExecutableFileName, g_exportFileName);
  } else {
    /* Pop up a "save file" dialog and get the exportExecutableFileName */
    dialog = gtk_file_chooser_dialog_new ("Export Executable",
        GTK_WINDOW(g_window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
        NULL);
    gtk_file_chooser_set_do_overwrite_confirmation( GTK_FILE_CHOOSER(dialog), TRUE);
    if(lastFilename[0] != '\0') {
      /* A file has been saved before */
      gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilename);
    } else {
      gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "program.exe");
    }
    if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT)
    {
      return;
    }
    exportExecutableFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
  }
  wchar_t *tmpdirname = new wchar_t[MAX_PATH];
  wchar_t tmpdirpfx[256] = L"RoboMancerTmp";
  wchar_t* dirname;
  GetTempPathW(256, tmpdirname);
  dirname = _wtempnam(tmpdirname, tmpdirpfx);
  /* Create the directory */
  _wmkdir(dirname);
  /* Get the current working directory */
  wchar_t *cwd = new wchar_t[MAX_PATH];
  _wgetcwd(cwd, MAX_PATH);
  /* Modify the PATH environment variables so that necessary libraries can be
   * found */
  if(!path_set) {
    wchar_t* pathenv;
    wchar_t* newpathenv;
    int pathenvSize;
    pathenvSize = GetEnvironmentVariableW(L"PATH", NULL, 0);
    pathenvSize += MAX_PATH*2;
    pathenv = (wchar_t*)malloc(sizeof(wchar_t)*(pathenvSize+1));
    newpathenv = (wchar_t*)malloc(sizeof(wchar_t)*(pathenvSize+1));
    GetEnvironmentVariableW(L"PATH", pathenv, pathenvSize);
    swprintf(newpathenv, L"%s;%s\\mingw\\bin", pathenv, cwd);
    SetEnvironmentVariableW(L"PATH", newpathenv);
    path_set = true;
    free(pathenv);
    free(newpathenv);
  }
  /* Set up the compile command */
  wchar_t *compileCommand= new wchar_t[MAX_PATH*10];
  wchar_t curFileNameW[MAX_PATH];
  wchar_t exportExecutableFileNameW[MAX_PATH];
  mbstowcs(&curFileNameW[0], g_curFileName, MAX_PATH);
  mbstowcs(&exportExecutableFileNameW[0], exportExecutableFileName, MAX_PATH);
  /* Compile the object file first */
  swprintf(
      compileCommand, 
      L"%s\\mingw\\bin\\gcc.exe -c -U_WIN32 -D_MSYS \"%s\" -o \"%s\\object.o\" > %s\\log.txt 2>&1", 
      cwd, curFileNameW, dirname, dirname);
  _wsystem(compileCommand);
  wprintf(L"%s\n", compileCommand);
  /* Make sure the object file exists */
  struct _stat s;
  swprintf(compileCommand, L"%s\\object.o", dirname);
  int filestatus = _wstat(compileCommand, &s);
  if(filestatus == 0) {
    /* Link the executable */
    swprintf(
        compileCommand, 
        L"%s\\mingw\\bin\\g++.exe -static -static-libgcc -static-libstdc++ \"%s\\object.o\" -lmobot++_wrapper -lmobot -lws2_32 -o \"%s\" >> %s\\log.txt 2>&1", 
        cwd, dirname, exportExecutableFileNameW, dirname);
    _wsystem(compileCommand);
    wprintf(L"%s\n", compileCommand);
    /* Check to see if the executable was successfully linked */
    filestatus = _wstat(exportExecutableFileNameW, &s);
    if(filestatus == 0) {
      if(g_exportFileName != NULL) {
        free(g_exportFileName);
      }
      g_exportFileName = (char*)malloc(sizeof(char)*MAX_PATH);
      wcstombs(g_exportFileName, exportExecutableFileNameW, sizeof(char)*MAX_PATH);
    }
  }
  /* Display the log contents */
  /* Open the log file */
  FILE *logfile;
  wchar_t* logfileName = new wchar_t[MAX_PATH];
  swprintf(logfileName, L"%s\\log.txt", dirname);
  logfile = _wfopen(logfileName, L"r");
  if(logfile == NULL) {
    fprintf(stderr, "Could not open compile-log file for reading!\n");
    return;
  }
  GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(g_builder, "textview_programMessages"));
  GtkTextBuffer *tb = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(w)));
  char line[512];
  /* Clear the text buffer */
  gtk_text_buffer_set_text(tb, "", -1);
  /* Get the file line by line, inserting it into the text buffer */
  bool errMsg = false;
  while(fgets(line, 512, logfile) != NULL) {
    if(
        !strstr(line, "manifestdependency") &&
        !strstr(line, "EXPORT")
      ) {
      gtk_text_buffer_insert_at_cursor(
          GTK_TEXT_BUFFER(tb),
          line,
          -1);
      errMsg = true;
    }
  }
  if(errMsg) {
    gtk_text_buffer_insert_at_cursor(
        GTK_TEXT_BUFFER(tb),
        "Export process completed.\n",
        -1);
  }
  fclose(logfile);

  /* Delete the log file */
  swprintf(compileCommand, L"%s\\log.txt", dirname);
  DeleteFileW(compileCommand);
  /* Delete the object file */
  swprintf(compileCommand, L"%s\\object.o", dirname);
  DeleteFileW(compileCommand);
  /* Delete the directory */
  RemoveDirectoryW(dirname);

  g_free (exportExecutableFileName);

  /* FRee eevveryythinggg!!! */
  delete[] logfileName;
  delete[] tmpdirname;
  delete[] cwd;
  delete[] compileCommand;
  gtk_widget_destroy (dialog);
#endif
}

void* run_program_thread(void* arg)
{
#if 0
  char* sourceCode = (char*)arg;
  /* Use the python interpreter */
  Py_Initialize();
  PyRun_SimpleString(sourceCode);
  //Py_Finalize();
  free(sourceCode);
  g_programRunning = false;
#endif
  return NULL;
}

void on_button_runExe_clicked(GtkWidget* widget, gpointer data)
{
  gtk_widget_set_sensitive(widget, false);
  /* Get the source code */
  int sourceCodeSize;
  char* sourceCode;
  sourceCodeSize = scintilla_send_message(g_sci, SCI_GETLENGTH, 0, 0) + 1;
  sourceCode = (char*)malloc(sourceCodeSize);
  scintilla_send_message(g_sci, SCI_GETTEXT, sourceCodeSize, (sptr_t)sourceCode);
  g_programRunning = true;
  THREAD_T thread;
  THREAD_CREATE(&thread, run_program_thread, sourceCode);
}

void on_checkbutton_showExternalEditor_toggled(GtkToggleButton *tb, gpointer data)
{
  if(gtk_toggle_button_get_active(tb)) {
    /* Button is checked. Show the external dialog */
    gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(g_builder, "window_externalEditor")));
  } else {
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(g_builder, "window_externalEditor")));
  }
}

void on_scintilla_notify(GObject *gobject, GParamSpec *pspec, struct SCNotification* scn)
{
  GtkWidget *w;
  switch(scn->nmhdr.code) {
    case SCN_SAVEPOINTREACHED:
      /* gray out "save" file menu item */
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_save"));
      gtk_widget_set_sensitive(w, false);
      g_dirty = false;
      break;
    case SCN_SAVEPOINTLEFT:
      /* Ungray "save" menu item */
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_save"));
      gtk_widget_set_sensitive(w, true);
      g_dirty = true;
      break;
    case SCN_MODIFIED:
      /* Update undo and redo buttons */
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_undo"));
      if(scintilla_send_message(g_sci, SCI_CANUNDO, 0, 0)) {
        gtk_widget_set_sensitive(w, true);
      } else {
        gtk_widget_set_sensitive(w, false);
      }
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem_redo"));
      if(scintilla_send_message(g_sci, SCI_CANREDO, 0, 0)) {
        gtk_widget_set_sensitive(w, true);
      } else {
        gtk_widget_set_sensitive(w, false);
      }
      break;
  }
}

void sendInputToClient()
{
  g_userChars[g_numUserChars] = '\n';
  g_numUserChars++;
  write(g_pipefd_stdin[1], g_userChars, g_numUserChars);
  g_numUserChars = 0;
  g_numUserCharsValid = 0;
}

gboolean on_textview_programMessages_key_press_event(GtkWidget*w, GdkEventKey* event, gpointer data)
{
  switch (event->keyval) {
    case GDK_KEY_BackSpace:
      if(g_numUserCharsValid > 0) {
        g_numUserCharsValid--;
        g_numUserChars--;
        return FALSE;
      } else {
        return TRUE;
      }
      break;
    case GDK_KEY_Return:
      sendInputToClient();
      break;
    default:
      g_userChars[g_numUserChars] = event->keyval;
      g_numUserChars++;
      g_numUserCharsValid++;
      break;
  }
  return FALSE; // Propogate signal
}

gboolean check_io_timeout(gpointer data)
{
  char buf[256];
  int rc;
  GtkTextIter iter;
  fflush(stdout);
  if(g_programRunning == false) {
    gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(g_builder, "button_runExe")), true);
  }
  if((rc = read(g_pipefd_stdout[0], buf, 255)) > 0) {
    buf[rc] = '\0';
    /* Append the read data to the end of the text buffer */
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(g_textbuffer_programMessages), &iter);
    gtk_text_buffer_insert(
        GTK_TEXT_BUFFER(g_textbuffer_programMessages),
        &iter,
        buf,
        -1);
    g_numUserCharsValid = 0;
  }
  if((rc = read(g_pipefd_stderr[0], buf, 255)) > 0) {
    buf[rc] = '\0';
    /* Append the read data to the end of the text buffer */
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(g_textbuffer_programMessages), &iter);
    gtk_text_buffer_insert_with_tags_by_name(
        GTK_TEXT_BUFFER(g_textbuffer_programMessages),
        &iter,
        buf,
        -1, "stderr", NULL);
    g_numUserCharsValid = 0;
  }
  return TRUE;
}

void refreshExternalEditor()
{
  /* Clear all contents and rewrite with current pose data */
  scintilla_send_message(g_sci_ext, SCI_CLEARALL, 0, 0);
  string* program;

  bool playLooped = false;
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "checkbutton_playLooped"));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
    playLooped = true;
  }  

  GtkWidget* combobox;
  combobox = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_outputLanguage"));
  int languageindex = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox));
  switch(languageindex) {
    case 0: // Ch
      program = g_robotManager->generateChProgram(playLooped, g_holdOnExit);
      break;
    case 1:
      // c++
      program = g_robotManager->generateCppProgram(playLooped, g_holdOnExit);
      break;
    case 2: // Python
      program = g_robotManager->generatePythonProgram(playLooped, g_holdOnExit);
      break;
  }
  scintilla_send_message(g_sci_ext, SCI_INSERTTEXT, 0, (sptr_t)program->c_str());
  scintilla_send_message(g_sci_ext, SCI_LINESCROLL, 0, 999);
}

void on_button_copyExternalToClipboard_clicked(GtkWidget *w, gpointer data)
{
  int size = scintilla_send_message(g_sci_ext, SCI_GETLENGTH, 0, 0) + 1;
  scintilla_send_message(g_sci_ext, SCI_COPYRANGE, 0, size);
}
