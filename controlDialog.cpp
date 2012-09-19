#include <unistd.h>
#include <stdlib.h>
#include "RoboMancer.h"
#include "controlDialog.h"
#include "thread_macros.h"
#include <mobot.h>

#ifdef _MSYS
#define sleep(x) _sleep(x)
#endif

/* The button state: For Buttons (B_), a value of 1 indicates that the button
 * has been pressed. For the sliders, 1 indicates that the slider is currently
 * depressed, and 0 indicates that it is not. */
int g_buttonState[NUM_BUTTONS];
int g_initSpeeds = 0;

/* Global variables that store slider and text entry values */
double g_speedSliderValues[4];
double g_speedEntryValues[4];
bool   g_speedEntryValuesValid[4];
double g_positionSliderValues[4];
double g_positionEntryValues[4];
bool   g_positionEntryValuesValid[4];
double g_positionValues[4];
int g_playIndex;
recordMobot_t* g_activeMobot = NULL;
MUTEX_T g_activeMobotLock;

typedef int(*handlerFunc)(void* arg);
handlerFunc g_handlerFuncs[NUM_BUTTONS];

enum gaits_e{
#define GAIT(sym, str, func) GAIT_##sym,
#include "gaits.x.h"
#undef GAIT
  NUM_GAITS
};

double normalizeDeg(double deg) {
  while(deg >= 180) {
    deg -= 360;
  }
  while (deg < -180) {
    deg += 360;
  }
  return deg;
}

void initControlDialog(void)
{
#define BUTTON(x) \
  g_handlerFuncs[B_##x] = handler##x;
#define SLIDER(x) \
  g_handlerFuncs[S_##x] = handler##x;
#include "buttons.x.h"
#undef BUTTON
#undef SLIDER
  /* Initalize vscales */
  GtkWidget* w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos1"));
  gtk_range_set_range(GTK_RANGE(w), -180, 180);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos2"));
  gtk_range_set_range(GTK_RANGE(w), -90, 90);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos3"));
  gtk_range_set_range(GTK_RANGE(w), -90, 90);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos4"));
  gtk_range_set_range(GTK_RANGE(w), -180, 180);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed1"));
  gtk_range_set_range(GTK_RANGE(w), 0, 120);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed2"));
  gtk_range_set_range(GTK_RANGE(w), 0, 120);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed3"));
  gtk_range_set_range(GTK_RANGE(w), 0, 120);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed4"));
  gtk_range_set_range(GTK_RANGE(w), 0, 120);

  /* Initialize gaits liststore */
  GtkTreeIter iter;
  GtkListStore* gaits_liststore = 
      GTK_LIST_STORE(gtk_builder_get_object(g_builder, "liststore_gaits"));
#define GAIT(sym, str, func) \
  gtk_list_store_append(gaits_liststore, &iter); \
  gtk_list_store_set(gaits_liststore, &iter, \
      0, str, -1);
#include "gaits.x.h"
#undef GAIT

  /* Initialize thread stuff */
  MUTEX_INIT(&g_activeMobotLock);

  /* Start handler thread */
  //g_idle_add(controllerHandlerTimeout, NULL);
  g_timeout_add(100, controllerHandlerTimeout, NULL);
  THREAD_T thread;
  THREAD_CREATE(&thread, controllerHandlerThread, NULL);
}

gboolean controllerHandlerTimeout(gpointer data)
{
  /* This function will repeatedly check to see if there have been any button
   * presses and process them, as well as update the UI */
  int i;
  int index;
  GtkWidget* w;
  recordMobot_t* mobot;
  double angles[4];
  /* First, check to see if a robot is even selected. If none selected, just return. */
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_connectedRobots"));
  index = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  if(index < 0) {
    MUTEX_LOCK(&g_activeMobotLock);
    g_activeMobot = NULL;
    MUTEX_UNLOCK(&g_activeMobotLock);
    return true;
  }
  /* Get the controlled mobot */
  mobot = g_robotManager->getMobot(index);
  if(mobot == NULL) {
    g_robotManager->disconnect(index);
    g_activeMobot = NULL;
    return true;
  }
  if(mobot != g_activeMobot) {
    MUTEX_LOCK(&g_activeMobotLock);
    g_activeMobot = mobot;
    MUTEX_UNLOCK(&g_activeMobotLock);
  }
  char buf[80];
#define VSCALEHANDLER(n) \
  if(g_buttonState[S_POS##n] == 0) { \
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos" #n)); \
    gtk_range_set_value(GTK_RANGE(w), normalizeDeg(g_positionValues[n-1])); \
  } \
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "label_motorPos" #n)); \
  sprintf(buf, "%.2lf", g_positionValues[n-1]); \
  gtk_label_set_text(GTK_LABEL(w), buf); 
  VSCALEHANDLER(1)
    VSCALEHANDLER(2)
    VSCALEHANDLER(3)
    VSCALEHANDLER(4)
#undef VSCALEHANDLER
    if(g_initSpeeds) {
      if(Mobot_getJointSpeeds((mobot_t*)mobot, 
          &angles[0], 
          &angles[1], 
          &angles[2], 
          &angles[3])) {
        if(!Mobot_isConnected((mobot_t*)mobot)) {
          g_robotManager->disconnect(index);
        }
        return true;
      }
#define VSCALEHANDLER(n) \
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed" #n)); \
      gtk_range_set_value(GTK_RANGE(w), RAD2DEG(angles[n-1]));  \
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "label_motorPos" #n)); \
      sprintf(buf, "%.2lf", RAD2DEG(angles[n-1])); \
      gtk_label_set_text(GTK_LABEL(w), buf);
      VSCALEHANDLER(1)
        VSCALEHANDLER(2)
        VSCALEHANDLER(3)
        VSCALEHANDLER(4)
#undef VSCALEHANDLER
        g_initSpeeds = 0;
    }
  /* Get slider, entry values */
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos1"));
  g_positionSliderValues[0] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos2"));
  g_positionSliderValues[1] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos3"));
  g_positionSliderValues[2] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos4"));
  g_positionSliderValues[3] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed1"));
  g_speedSliderValues[0] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed2"));
  g_speedSliderValues[1] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed3"));
  g_speedSliderValues[2] = gtk_range_get_value(GTK_RANGE(w));
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed4"));
  g_speedSliderValues[3] = gtk_range_get_value(GTK_RANGE(w));
  return true;
}

void* controllerHandlerThread(void* arg)
{
  /* This thread is responsible for communicating with the Mobot. All mobot
   * communications should be done in this thread to prevent communication
   * errors from hanging the app. */
  while(1) {
    /* First, check to see if a mobot is even connected. */
    MUTEX_LOCK(&g_activeMobotLock);
    if(g_activeMobot == NULL) {
      MUTEX_UNLOCK(&g_activeMobotLock);
      sleep(1);
      continue;
    }
    MUTEX_UNLOCK(&g_activeMobotLock);

#ifdef __MACH__
#undef THREAD_YIELD
#define THREAD_YIELD()
#endif

#define TESTLOCK \
    THREAD_YIELD(); \
    MUTEX_LOCK(&g_activeMobotLock); \
    if(g_activeMobot == NULL) { \
      MUTEX_UNLOCK(&g_activeMobotLock); \
      continue; \
    }

    TESTLOCK
    int rc = Mobot_getJointAngles(
        (mobot_t*)g_activeMobot,
        &g_positionValues[0],
        &g_positionValues[1],
        &g_positionValues[2],
        &g_positionValues[3]);
    /* First, get motor position values */
    /* Convert angles to degrees */
    int i;
    for(i = 0; i < 4; i++) {
      //g_positionValues[i] = normalizeAngleRad(g_positionValues[i]);
      if(!rc) {
        g_positionValues[i] = RAD2DEG(g_positionValues[i]);
      }
      else {
        g_positionValues[i] = 0;
      }
    }
    MUTEX_UNLOCK(&g_activeMobotLock);
    if(rc) {
      MUTEX_LOCK(&g_activeMobotLock);
      g_activeMobot = NULL;
      MUTEX_UNLOCK(&g_activeMobotLock);
      continue;
    }
    /* Cycle through button handlers */
    TESTLOCK
    for(i = 0; i < NUM_BUTTONS; i++) {
      if(g_buttonState[i]) {
        if( (i >= S_SPEED1) && (i <= S_POS4) ) {
          g_handlerFuncs[i](g_activeMobot);
        } else {
          g_buttonState[i] = g_handlerFuncs[i](g_activeMobot);
        }
      }
    }
    MUTEX_UNLOCK(&g_activeMobotLock);
  }
}

int handlerZERO(void* arg)
{
  Mobot_resetToZeroNB((mobot_t*)arg);
}

#define HANDLER_FORWARD(n) \
int handlerJ##n##FORWARD(void* arg) \
{ \
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT##n, MOBOT_FORWARD); \
  return 0; \
}
HANDLER_FORWARD(1)
HANDLER_FORWARD(2)
HANDLER_FORWARD(3)
HANDLER_FORWARD(4)
#undef HANDLER_FORWARD

#define HANDLER_BACKWARD(n) \
int handlerJ##n##BACK(void* arg) \
{ \
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT##n, MOBOT_BACKWARD); \
  return 0; \
}
HANDLER_BACKWARD(1)
HANDLER_BACKWARD(2)
HANDLER_BACKWARD(3)
HANDLER_BACKWARD(4)
#undef HANDLER_BACKWARD

#define HANDLER_STOP(n) \
int handlerJ##n##STOP(void* arg) \
{ \
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT##n, MOBOT_NEUTRAL); \
  return 0; \
}
HANDLER_STOP(1)
HANDLER_STOP(2)
HANDLER_STOP(3)
HANDLER_STOP(4)
#undef HANDLER_STOP

int handlerROLLFORWARD(void* arg)
{
  //Mobot_motionRollForwardNB((mobot_t*)arg, DEG2RAD(90));
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_FORWARD);
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_FORWARD);
  return 0;
}

int handlerTURNLEFT(void* arg)
{
  //Mobot_motionTurnLeftNB((mobot_t*)arg, DEG2RAD(90));
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_BACKWARD);
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_FORWARD);
  return 0;
}

int handlerTURNRIGHT(void* arg)
{
  //Mobot_motionTurnRightNB((mobot_t*)arg, DEG2RAD(90));
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_FORWARD);
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_BACKWARD);
  return 0;
}

int handlerROLLBACK(void* arg)
{
  //Mobot_motionRollBackwardNB((mobot_t*)arg, DEG2RAD(90));
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_BACKWARD);
  Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_BACKWARD);
  return 0;
}

int handlerSTOP(void* arg)
{
  Mobot_stop((mobot_t*)arg);
  return 0;
}

int handlerSETSPEEDS(void* arg)
{
  /* Get the entry values */
  double value;
  const gchar* text;
  GtkWidget *w;
#define GETVALUESETSPEED(n) \
  if(g_speedEntryValuesValid[n-1]) { \
    Mobot_setJointSpeed(\
      (mobot_t*)arg, \
      MOBOT_JOINT##n, \
      DEG2RAD(g_speedEntryValues[n-1])); \
  }

  GETVALUESETSPEED(1)
  GETVALUESETSPEED(2)
  GETVALUESETSPEED(3)
  GETVALUESETSPEED(4)
#undef GETVALUESETSPEED
  return 0;
}

int handlerMOVE(void* arg)
{
  /* Get the entry values */
  double value;
  const gchar* text;
  GtkWidget *w;
#define GETVALUEMOVEJOINT(n) \
  if(g_positionEntryValuesValid[n-1]) { \
    Mobot_moveJointNB(\
      (mobot_t*)arg, \
      MOBOT_JOINT##n, \
      DEG2RAD(g_positionEntryValues[n-1])); \
  }

  GETVALUEMOVEJOINT(1)
  GETVALUEMOVEJOINT(2)
  GETVALUEMOVEJOINT(3)
  GETVALUEMOVEJOINT(4)
#undef GETVALUEMOVEJOINT
  return 0;
}

int handlerMOVETO(void* arg)
{
  /* Get the entry values */
  double value;
  const gchar* text;
  GtkWidget *w;
#define GETVALUEMOVEJOINT(n) \
  if(g_positionEntryValuesValid[n-1]) { \
    Mobot_moveJointToNB(\
      (mobot_t*)arg, \
      MOBOT_JOINT##n, \
      DEG2RAD(g_positionEntryValues[n-1])); \
  }

  GETVALUEMOVEJOINT(1)
  GETVALUEMOVEJOINT(2)
  GETVALUEMOVEJOINT(3)
  GETVALUEMOVEJOINT(4)
#undef GETVALUEMOVEJOINT
  return 0;
}

int handlerPLAY(void* arg)
{
  switch(g_playIndex) {
#define GAIT(sym, str, func) \
    case GAIT_##sym: \
      func; \
      break;
#include "gaits.x.h"
#undef GAIT
    default:
      return 0;
  }
  return 0;
}

#define HANDLER_SPEED(n) \
int handlerSPEED##n(void* arg) \
{ \
  /* Get the slider position */ \
  double value; \
  value = g_speedSliderValues[n-1]; \
  Mobot_setJointSpeed((mobot_t*)arg, MOBOT_JOINT##n, DEG2RAD(value)); \
  return 1; \
}
HANDLER_SPEED(1)
HANDLER_SPEED(2)
HANDLER_SPEED(3)
HANDLER_SPEED(4)
#undef HANDLER_SPEED

#define HANDLER_POS(n) \
int handlerPOS##n(void*arg) \
{ \
  /* Get the slider position */ \
  double value; \
  value = g_positionSliderValues[n-1]; \
  Mobot_driveJointToDirectNB((mobot_t*)arg, MOBOT_JOINT##n, DEG2RAD(value)); \
  return 1; \
}
HANDLER_POS(1)
HANDLER_POS(2)
HANDLER_POS(3)
HANDLER_POS(4)
#undef HANDLER_POS

#define BUTTONHANDLERS(n) \
void on_button_motor##n##back_clicked(GtkWidget*w, gpointer data) \
{ \
  g_buttonState[B_J##n##BACK] = 1; \
} \
void on_button_motor##n##stop_clicked(GtkWidget*w, gpointer data) \
{ \
  g_buttonState[B_J##n##STOP] = 1; \
} \
void on_button_motor##n##forward_clicked(GtkWidget*w, gpointer data) \
{ \
  g_buttonState[B_J##n##FORWARD] = 1; \
} 
BUTTONHANDLERS(1)
BUTTONHANDLERS(2)
BUTTONHANDLERS(3)
BUTTONHANDLERS(4)
#undef BUTTONHANDLERS

#define SLIDERHANDLERS(n) \
gboolean on_vscale_motorPos##n##_button_press_event(GtkWidget*w, GdkEvent* event, gpointer data) \
{ \
  g_buttonState[S_POS##n] = 1; \
  return FALSE; \
} \
gboolean on_vscale_motorPos##n##_button_release_event(GtkWidget*w, GdkEvent* event, gpointer data) \
{ \
  g_buttonState[S_POS##n] = 0; \
  return FALSE; \
} \
gboolean on_vscale_motorspeed##n##_button_press_event(GtkWidget*w, GdkEvent* event, gpointer data) \
{ \
  g_buttonState[S_SPEED##n] = 1; \
  return FALSE; \
} \
gboolean on_vscale_motorspeed##n##_button_release_event(GtkWidget*w, GdkEvent* event, gpointer data) \
{ \
  g_buttonState[S_SPEED##n] = 0; \
  return FALSE; \
} 
SLIDERHANDLERS(1)
SLIDERHANDLERS(2)
SLIDERHANDLERS(3)
SLIDERHANDLERS(4)
#undef SLIDERHANDLERS

void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data)
{
  g_initSpeeds = 1;
}

void on_button_forward_clicked(GtkWidget* w, gpointer data)
{
  g_buttonState[B_ROLLFORWARD] = 1;
}

void on_button_rotateLeft_clicked(GtkWidget* w, gpointer data)
{
  g_buttonState[B_TURNLEFT] = 1;
}

void on_button_stop_clicked(GtkWidget* w, gpointer data)
{
  g_buttonState[B_STOP] = 1;
}

void on_button_rotateRight_clicked(GtkWidget* w, gpointer data)
{
  g_buttonState[B_TURNRIGHT] = 1;
}

void on_button_backward_clicked(GtkWidget* w, gpointer data)
{
  g_buttonState[B_ROLLBACK] = 1;
}

void on_button_setSpeeds_clicked(GtkWidget* wid, gpointer data)
{
  double value;
  const gchar* text;
  GtkWidget *w;
  /* Populate the data variables */
#define GETVALUESETSPEED(n) \
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorSpeed" #n)); \
  text = gtk_entry_get_text(GTK_ENTRY(w)); \
  if(text != NULL) { \
    sscanf(text, "%lf", &value); \
    g_speedEntryValues[n-1] = value; \
    g_speedEntryValuesValid[n-1] = true; \
  } else { \
    g_speedEntryValuesValid[n-1] = false; \
  }
  GETVALUESETSPEED(1)
  GETVALUESETSPEED(2)
  GETVALUESETSPEED(3)
  GETVALUESETSPEED(4)
#undef GETVALUESETSPEED
  g_buttonState[B_SETSPEEDS] = 1;
}

void on_button_moveToZero_clicked(GtkWidget* w, gpointer data)
{
  g_buttonState[B_ZERO] = 1;
}

void on_button_move_clicked(GtkWidget* wid, gpointer data)
{
  /* Get the entry values */
  double value;
  const gchar* text;
  GtkWidget *w;
#define GETVALUEMOVEJOINT(n) \
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorPos" #n)); \
  text = gtk_entry_get_text(GTK_ENTRY(w)); \
  if(text != NULL) { \
    sscanf(text, "%lf", &value); \
    g_positionEntryValues[n-1] = value; \
    g_positionEntryValuesValid[n-1] = true; \
  } else { \
    g_positionEntryValuesValid[n-1] = false; \
  }
  GETVALUEMOVEJOINT(1)
  GETVALUEMOVEJOINT(2)
  GETVALUEMOVEJOINT(3)
  GETVALUEMOVEJOINT(4)
#undef GETVALUEMOVEJOINT
  g_buttonState[B_MOVE] = 1;
}

void on_button_moveTo_clicked(GtkWidget* wid, gpointer data)
{
  /* Get the entry values */
  double value;
  const gchar* text;
  GtkWidget *w;
#define GETVALUEMOVEJOINT(n) \
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorPos" #n)); \
  text = gtk_entry_get_text(GTK_ENTRY(w)); \
  if(text != NULL && strlen(text) > 0) { \
    sscanf(text, "%lf", &value); \
    g_positionEntryValues[n-1] = value; \
    g_positionEntryValuesValid[n-1] = true; \
  } else { \
    g_positionEntryValuesValid[n-1] = false; \
  }
  GETVALUEMOVEJOINT(1)
  GETVALUEMOVEJOINT(2)
  GETVALUEMOVEJOINT(3)
  GETVALUEMOVEJOINT(4)
#undef GETVALUEMOVEJOINT
  g_buttonState[B_MOVETO] = 1;
}

void on_button_playGait_clicked(GtkWidget* w, gpointer data)
{
  /* Figure out which item is selected */
  GtkWidget* view =  GTK_WIDGET(gtk_builder_get_object(g_builder, "treeview_gaits"));
  GtkTreeModel* model = GTK_TREE_MODEL(gtk_builder_get_object(g_builder, "liststore_gaits"));
  GtkTreeSelection* selection = gtk_tree_view_get_selection((GTK_TREE_VIEW(view)));
  GList* list = gtk_tree_selection_get_selected_rows(selection, &model);
  if(list == NULL) return;
  gint* paths = gtk_tree_path_get_indices((GtkTreePath*)list->data);
  int i = paths[0];
  g_list_foreach(list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free(list);
  g_playIndex = i;

  g_buttonState[B_PLAY] = 1;
}
