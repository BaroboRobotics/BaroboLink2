#include "RoboMancer.h"
#include "RobotManager.h"
#include "thread_macros.h"
#include <mobot.h>
#include <errno.h>

bool g_isPlaying = false;
bool g_haltPlayFlag = false;
int g_dndInsertIndex = 0;
bool g_dndInsertIndexValid = false;
int g_dndRemoveIndex = 0;
bool g_dndRemoveIndexValid = false;
bool g_dnd = true;
bool g_playLooped = false;
bool g_holdOnExit = false;
int g_poseIndex;
int g_selectedRobot = 0;

void teachingDialog_refreshRecordedMotions(int currentMotion)
{
  g_dnd = false;
  int i;
  GtkTreeIter iter;
  /* Clear the liststore */
  GtkListStore* ls = GTK_LIST_STORE(gtk_builder_get_object(g_builder, "liststore_recordedMotions"));
  gtk_list_store_clear(ls);
  /* Populate it with motions */
  recordMobot_t* mobot;
  mobot = g_robotManager->getMobot(0);
  if(mobot == NULL) {return;}
  for(i = 0; i < mobot->numMotions; i++) {
    gtk_list_store_append(ls, &iter);
    if(i == currentMotion) {
      gtk_list_store_set(ls, &iter, 
          0, mobot->motions[i]->name,
          1, GTK_STOCK_YES,
          -1);
    } else {
      gtk_list_store_set(ls, &iter, 
          0, mobot->motions[i]->name,
          1, GTK_STOCK_ABOUT,
          -1);
    }
  }
  g_dnd = true;
}

void on_button_setJointsNeutral_clicked(GtkWidget*w, gpointer data)
{
  /* Set all joints of all connected mobots to neutral */
  mobot_t* mobot;
  int i;
  for(i = 0; i < g_robotManager->numEntries(); i++) {
    mobot = (mobot_t*)g_robotManager->getMobotIndex(i);
    if(mobot == NULL) {
      continue;
    }
    Mobot_setMovementStateNB(mobot, 
        MOBOT_NEUTRAL,
        MOBOT_NEUTRAL,
        MOBOT_NEUTRAL,
        MOBOT_NEUTRAL);
  }
}

void on_button_holdJoints_clicked(GtkWidget*w, gpointer data)
{
  /* Set all joints of all connected mobots to hold */
  mobot_t* mobot;
  int i;
  for(i = 0; i < g_robotManager->numEntries(); i++) {
    mobot = (mobot_t*)g_robotManager->getMobotIndex(i);
    if(mobot == NULL) {
      continue;
    }
    Mobot_setMovementStateNB(mobot, 
        MOBOT_HOLD,
        MOBOT_HOLD,
        MOBOT_HOLD,
        MOBOT_HOLD);
  }
}

void on_button_recordPos_clicked(GtkWidget*w, gpointer data)
{
  g_robotManager->record();
  teachingDialog_refreshRecordedMotions(-1);
}

void on_button_addDelay_clicked(GtkWidget*w, gpointer data)
{
  /* Get the text from the entry */
  const gchar* str;
  GtkEntry *entry;
  double seconds;
  entry = GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_delaySeconds"));
  str = gtk_entry_get_text(entry);
  if(strlen(str) == 0) {
    return;
  }
  sscanf(str, "%lf", &seconds);
  g_robotManager->addDelay(seconds);
  teachingDialog_refreshRecordedMotions(-1);
}

void on_button_deleteRecordedPos_clicked(GtkWidget*w, gpointer data)
{
  recordMobot_t* m;
  int i, index;
  /* Get the selected position */
  GtkWidget* view =  GTK_WIDGET(gtk_builder_get_object(g_builder, "treeview_recordedMotions"));
  GtkTreeModel* model = GTK_TREE_MODEL(gtk_builder_get_object(g_builder, "liststore_recordedMotions"));
  GtkTreeSelection* selection = gtk_tree_view_get_selection((GTK_TREE_VIEW(view)));
  GList* list = gtk_tree_selection_get_selected_rows(selection, &model);
  if(list == NULL) return;
  gint* paths = gtk_tree_path_get_indices((GtkTreePath*)list->data);
  index = paths[0];
  g_list_foreach(list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free(list);

  m = g_robotManager->getMobot(0);
  for(i = 1; m != NULL; i++) {
    RecordMobot_removeMotion(m, index, true);
    m = g_robotManager->getMobot(i);
  }
  teachingDialog_refreshRecordedMotions(-1);
}

void on_button_clearRecordedPositions_clicked(GtkWidget*w, gpointer data)
{
  recordMobot_t* m;
  int i;
  m = g_robotManager->getMobot(0);
  for(i = 1; m != NULL; i++) {
    RecordMobot_clearAllMotions(m);
    m = g_robotManager->getMobot(i);
  }
  teachingDialog_refreshRecordedMotions(-1);
}

void on_button_saveToProgram_clicked(GtkWidget*widget, gpointer data)
{
  /* See if the looped checkmark is checked */
  bool playLooped = false;
  string* program;
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "checkbutton_playLooped"));
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
    playLooped = true;
  }  
  GtkWidget* combobox;
  combobox = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_poseLanguage"));
  int languageindex = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox));
  char defaultFilename[128];
  switch(languageindex) {
    case 0: // Python
      program = g_robotManager->generatePythonProgram(playLooped, g_holdOnExit);
      strcpy(defaultFilename, "untitled.py");
      break;
    case 1:
      strcpy(defaultFilename, "untitled.cpp");
    case 2: // C++/Ch
      program = g_robotManager->generateChProgram(playLooped, g_holdOnExit);
      strcpy(defaultFilename, "untitled.Ch");
  }
  
  /* Open a save file dialog */
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("Save File",
      GTK_WINDOW(g_window),
      GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
      NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), defaultFilename);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    FILE *fp;
    fp = fopen(filename, "w");
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
    fwrite(program->c_str(), program->size(), sizeof(char), fp);
    fclose(fp);
    g_free (filename);
  }
  gtk_widget_destroy (dialog);
  delete program;
}

void on_radiobutton_neutralOnExit_clicked(GtkWidget*w, gpointer data)
{
  g_holdOnExit = false;
}

void on_radiobutton_holdOnExit_clicked(GtkWidget*w, gpointer data)
{
  g_holdOnExit = true;
}

void* playThread(void* arg)
{
  CRobotManager* robotManager;
  robotManager = g_robotManager;
  static int j=0;
  g_isPlaying = true;
  int done = 0;

  for(g_poseIndex = 0; !done ; g_poseIndex++) {
    if(g_haltPlayFlag) {
      g_haltPlayFlag = 0;

      for(j = 0; j < robotManager->numConnected(); j++) {
        //Mobot_stop((mobot_t*)robotManager->getMobot(j));
        if(g_holdOnExit) {
          Mobot_setMovementStateNB(
              (mobot_t*)robotManager->getMobot(j),
              MOBOT_HOLD,
              MOBOT_HOLD,
              MOBOT_HOLD,
              MOBOT_HOLD);
        } else {
          Mobot_setMovementStateNB(
              (mobot_t*)robotManager->getMobot(j),
              MOBOT_NEUTRAL,
              MOBOT_NEUTRAL,
              MOBOT_NEUTRAL,
              MOBOT_NEUTRAL);
        }
      }
      g_isPlaying = false;
      done = 0; g_poseIndex = 0; j = 0;
      return NULL;
    }

    for(j = 0; j < robotManager->numConnected(); j++) {
      if(RecordMobot_getMotionType(robotManager->getMobot(j), g_poseIndex) == MOTION_SLEEP) {
        RecordMobot_play(robotManager->getMobot(j), g_poseIndex);
        break;
      }
      if(RecordMobot_play(robotManager->getMobot(j), g_poseIndex)) {
        done = 1;
      }
    }
    for(j = 0; j < g_robotManager->numConnected(); j++) {
      Mobot_moveWait((mobot_t*)g_robotManager->getMobot(j));
    }
    if(done) {
      if(g_playLooped) {
        g_poseIndex = -1;
        done = 0;
      }
    }
  }
  for(j = 0; j < robotManager->numConnected(); j++) {
    //Mobot_stop((mobot_t*)robotManager->getMobot(j));
    if(g_holdOnExit) {
      Mobot_setMovementStateNB(
          (mobot_t*)robotManager->getMobot(j),
          MOBOT_HOLD,
          MOBOT_HOLD,
          MOBOT_HOLD,
          MOBOT_HOLD);
    } else {
      Mobot_setMovementStateNB(
          (mobot_t*)robotManager->getMobot(j),
          MOBOT_NEUTRAL,
          MOBOT_NEUTRAL,
          MOBOT_NEUTRAL,
          MOBOT_NEUTRAL);
    }
  }
  g_isPlaying = false;
  done = 0; g_poseIndex = 0; j = 0;
  return NULL;
}

gboolean poseGuiTimeout(gpointer userdata)
{
  GtkWidget *w;
  int j;
  static int lastPoseIndex;
  if(!g_isPlaying) {
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "button_playRecorded"));
    gtk_widget_set_sensitive(w, TRUE);
    teachingDialog_refreshRecordedMotions(-1);
    g_poseIndex = 0; j = 0;
    return false;
  }
  if(lastPoseIndex != g_poseIndex) {
    teachingDialog_refreshRecordedMotions(g_poseIndex);
    lastPoseIndex = g_poseIndex;
  }
  return true;
}

void on_button_playRecorded_clicked(GtkWidget*button, gpointer data) 
{
  GtkWidget *w;
  /* Disable the play button */
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "button_playRecorded"));
  gtk_widget_set_sensitive(w, FALSE);
  teachingDialog_refreshRecordedMotions(0);
  /* Start the play thread */
  g_haltPlayFlag = false;
  g_isPlaying = true;
  THREAD_T thread;
  THREAD_CREATE(&thread, playThread, NULL);
  g_timeout_add(200, poseGuiTimeout, NULL);
}

void on_button_stopRecorded_clicked(GtkWidget*w, gpointer data)
{
  g_haltPlayFlag = true;
}

void on_checkbutton_playLooped_clicked(GtkWidget*w, gpointer data)
{
  g_playLooped = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
}

gboolean refreshTimeout(gpointer userdata)
{
  teachingDialog_refreshRecordedMotions(-1);
  return false;
}

gboolean initPlayTimeout(gpointer userdata)
{
  on_button_playRecorded_clicked(NULL, NULL);
  return false;
}

void on_mobotButtonPress(void* data, int button, int buttonDown)
{
  /* Button A: Record Motion 
   * Button B: Playback / Stop
   * Both Buttons: Reset motions */
  mobot_t* mobot = (mobot_t*)data;
  static unsigned int lastState = 0;
  unsigned int newState;
  static bool debounce = false;
  int i;

  /* Calculate the new state */
  if(buttonDown) {
    //Mobot_blinkLED(mobot, 0.1, 2);
    newState = lastState | (1<<button);
  } else {
    newState = lastState & ~(1<<button);
  }
  if(debounce) {
    /* Must wait until all buttons are up */
    if(newState == 0) {
      debounce = false;
      lastState = newState;
      return;
    } else {
      lastState = newState;
      return;
    }
  }

  /* If a button release event is detected, use the last state to determine the action */
  if(buttonDown == 0) {
    if(lastState == 0x01) {
      /* Button A press/release */
      for(i = 0; i < g_robotManager->numConnected(); i++) {
		    RecordMobot_record(g_robotManager->getMobot(i));
	    }
	    //teachingDialog_refreshRecordedMotions(-1);
      g_idle_add(refreshTimeout, NULL);
    }
    if(lastState == 0x02) {
      /* Button B press/release */
      /* If it is not playing play. Otherwise, stop. */
      if(g_isPlaying) {
        g_haltPlayFlag = 1;
      } else {
        g_idle_add(initPlayTimeout, NULL);
      }
    }
    if(lastState == 0x03) {
      /* Buttons A/B Pressed, one released */
      debounce = true;
      /* Clear all recorded motions */
      recordMobot_t* m;
      int i;
      m = g_robotManager->getMobot(0);
      for(i = 1; m != NULL; i++) {
        RecordMobot_clearAllMotions(m);
        m = g_robotManager->getMobot(i);
      }
      //teachingDialog_refreshRecordedMotions(-1);
      g_idle_add(refreshTimeout, NULL);
    }
  }
  lastState = newState;
}

void on_notebook1_switch_page(GtkNotebook* notebook, gpointer page, guint page_num, gpointer userdata)
{
  static int lastPage = 0;
  static bool buttonCallbackEnabled = false;
  int i;
  int connectPage, controlPage, controlPage2, programPage, posePage;
  GtkWidget *w;

  /* Figure out the page numbers by the number of total pages */
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "notebook1"));
  int numPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(w));
  if(numPages == 4) {
    connectPage = 0;
    controlPage = 1;
    controlPage2 = 2;
    posePage = 3;
    programPage = 99;
  } else if (numPages == 5) {
    connectPage = 0;
    controlPage = 1;
    controlPage2 = 2;
    programPage = 3;
    posePage = 4;
  }

  mobot_t* mobot;
  refreshConnectDialog();
  /* If the teaching dialog gets selected, we should initialize all connected
   * Mobots to use our custom button handler. */
  if(page_num == posePage) {
    /* Enable all button handlers */
    for(i = 0; i < g_robotManager->numConnected(); i++) {
      mobot = (mobot_t*)g_robotManager->getMobot(i);
      Mobot_enableButtonCallback(mobot, mobot, on_mobotButtonPress);
      buttonCallbackEnabled = true;
    }
  } else {
    if(buttonCallbackEnabled) {
    /* Disable all button callbacks */
      for(i = 0; i < g_robotManager->numConnected(); i++) {
        mobot = (mobot_t*)g_robotManager->getMobot(i);
        Mobot_disableButtonCallback(mobot);
      }
    }
  }

  /* If the control dialog is selected... */
  if(
      (page_num == controlPage) ||
      (page_num == controlPage2)
      ) 
  {
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_connectedRobots"));
    if(g_selectedRobot == -1) {
      g_selectedRobot = 0;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), g_selectedRobot);
    if(page_num == controlPage) {
      g_controlMode = 0;
    } else {
      g_controlMode = 1;
    }
  } else {
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_connectedRobots"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), -1);
  }

  /* If the connect dialog is selected, refresh the dialog */
  if(page_num == connectPage) {
    refreshConnectDialog();
  }

  lastPage = page_num;
}

void on_liststore_recordedMotions_rows_reordered(
    GtkTreeModel* model, 
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer _new_order,
    gpointer user_data)
{
  int *new_order = (int*)_new_order;
  int i;
  for(i = 0; i < g_robotManager->getMobot(0)->numMotions; i++) {
    printf("%d : %d\n", i, new_order[i]);
  }
}

void on_cellrenderertext_recordedMotionName_edited(
    GtkCellRendererText* renderer,
    gchar* path,
    gchar* new_text,
    gpointer user_data)
{
  //printf("Edited cell: %s\n", path);
  int index;
  sscanf(path, "%d", &index);
  recordMobot_t* m;
  m = g_robotManager->getMobot(0);
  for(int i = 1; m != NULL; i++) {
    RecordMobot_setMotionName(m, index, new_text);
    m = g_robotManager->getMobot(i);
  }
  teachingDialog_refreshRecordedMotions(-1);
}

void on_liststore_recordedMotions_row_deleted(
    GtkTreeModel* model,
    GtkTreePath* path,
    gpointer user_data)
{
  gint* indices;
  int depth;
  if(g_dnd) {
    indices = gtk_tree_path_get_indices_with_depth(path, &depth);
    g_dndRemoveIndex = indices[0];
    g_dndRemoveIndexValid = true;
    //printf("Row Deleted: %d\n", indices[0]);
    printf("%d %d\n", g_dndRemoveIndexValid, g_dndInsertIndexValid);

    if(g_dndRemoveIndexValid && g_dndInsertIndexValid) {
      g_dndRemoveIndexValid = false;
      g_dndInsertIndexValid = false;
      if(g_dndInsertIndex < g_dndRemoveIndex) {
        g_dndRemoveIndex--;
      } else {
        g_dndInsertIndex--;
      }
      recordMobot_t *m;
      m = g_robotManager->getMobot(0);
      for(int i = 1; m != NULL; i++) {
        RecordMobot_moveMotion(m, g_dndRemoveIndex, g_dndInsertIndex);
        m = g_robotManager->getMobot(i);
      }
      teachingDialog_refreshRecordedMotions(-1);
    }
  }
}

void on_liststore_recordedMotions_row_inserted(
    GtkTreeModel* model,
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer user_data)
{
  gint* indices;
  int depth;
  if(g_dnd) {
    indices = gtk_tree_path_get_indices_with_depth(path, &depth);
    //printf("Row Inserted: %d\n", indices[0]);
    g_dndInsertIndex = indices[0];
    g_dndInsertIndexValid = true;

    printf("%d %d\n", g_dndRemoveIndexValid, g_dndInsertIndexValid);
    if(g_dndRemoveIndexValid && g_dndInsertIndexValid) {
      g_dndRemoveIndexValid = false;
      g_dndInsertIndexValid = false;
      recordMobot_t *m;
      m = g_robotManager->getMobot(0);
      for(int i = 1; m != NULL; i++) {
        RecordMobot_moveMotion(m, g_dndRemoveIndex, g_dndInsertIndex);
        m = g_robotManager->getMobot(i);
      }
      teachingDialog_refreshRecordedMotions(-1);
    }
  }
}
    
