#include <gtk/gtk.h>
#include <string.h>
#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include "RoboMancer.h"
#ifdef _MSYS
#include <windows.h>
#endif

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

void on_button_exportExe_clicked(GtkWidget* widget, gpointer data)
{
#ifdef _MSYS
  static bool path_set = false;
  static char lastFilename[256] = "";
  printf("Hello there\n");
  /* Pop up a "save file" dialog and get the filename */
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("Save File",
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
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    //open_file (filename);
    wchar_t *tmpdirname = new wchar_t[MAX_PATH];
    wchar_t tmpdirpfx[256] = L"RoboMancerTmp";
    wchar_t* dirname;
    GetTempPathW(256, tmpdirname);
    dirname = _wtempnam(tmpdirname, tmpdirpfx);
    /* Create the directory */
    _wmkdir(dirname);
    /* Copy the source code over there */
    char* sourceCode;
    int sourceCodeSize;
    sourceCodeSize = scintilla_send_message(g_sci, SCI_GETLENGTH, 0, 0) + 1;
    sourceCode = (char*)malloc(sourceCodeSize);
    scintilla_send_message(g_sci, SCI_GETTEXT, sourceCodeSize, (sptr_t)sourceCode);
    wchar_t sourceFileName[256];
    swprintf(sourceFileName, L"%s/source.cpp", dirname);
    wprintf(L"%s\n", sourceFileName);
    FILE *fp;
    fp = _wfopen(sourceFileName, L"w");
    if(fp == NULL) {
      fprintf(stderr, "Error writing temporary source code file\n.");
      free(sourceCode);
      g_free(filename);
      gtk_widget_destroy(dialog);
      return;
    }
    fprintf(fp, "%s\n", sourceCode);
    fclose(fp);
    free(sourceCode);
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
      pathenv = (wchar_t*)malloc(pathenvSize+1);
      newpathenv = (wchar_t*)malloc(pathenvSize+1);
      GetEnvironmentVariableW(L"PATH", pathenv, pathenvSize);
      swprintf(newpathenv, L"%s;%s\\mingw\\bin", pathenv, cwd);
      SetEnvironmentVariableW(L"PATH", newpathenv);
      path_set = true;
      free(pathenv);
      free(newpathenv);
    }
    /* Set up the compile command */
    wchar_t *compileCommand= new wchar_t[MAX_PATH];
    swprintf(
        compileCommand, 
        L"%s\\mingw\\bin\\gcc.exe -U_WIN32 -lmobot %s -o %s\\program.exe > %s\\log.txt 2>&1", 
        cwd, sourceFileName, dirname, dirname);
    _wsystem(compileCommand);
    /* Copy the compiled file to the requested location */
    wchar_t* fileOrigin = new wchar_t[MAX_PATH];
    swprintf(fileOrigin, L"%s\\program.exe", dirname);
    char* fileOriginA = new char[MAX_PATH];
    wcstombs(fileOriginA, fileOrigin, MAX_PATH);
    CopyFileA(fileOriginA, filename, false);
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
    w = GTK_WIDGET(gtk_text_view_get_buffer(GTK_TEXT_VIEW(w)));
    char line[512];
    /* Get the file line by line, inserting it into the text buffer */
    while(fgets(line, 512, logfile) != NULL) {
      gtk_text_buffer_insert_at_cursor(
          GTK_TEXT_BUFFER(w),
          line,
          -1);
    }
      gtk_text_buffer_insert_at_cursor(
          GTK_TEXT_BUFFER(w),
          "Helloooo!!!",
          -1);
    fclose(logfile);

    g_free (filename);

    /* FRee eevveryythinggg!!! */
    delete[] logfileName;
    delete[] tmpdirname;
    delete[] cwd;
    delete[] compileCommand;
    delete[] fileOrigin;
    delete[] fileOriginA;
  } else {
    printf("Gtk response was not accept!\n");
  }
  gtk_widget_destroy (dialog);
#endif
}

void on_button_runExe_clicked(GtkWidget* widget, gpointer data)
{
}
