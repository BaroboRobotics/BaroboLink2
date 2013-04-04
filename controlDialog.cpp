#include <stdio.h>
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
int g_controlMode = 0; // 0 means update main control page, 1 means update "sensors" page

/* Global variables that store slider and text entry values */
double g_speedSliderValues[4];
double g_speedEntryValues[4];
bool   g_speedEntryValuesValid[4];
double g_positionSliderValues[4];
double g_positionEntryValues[4];
bool   g_positionEntryValuesValid[4];
double g_positionValues[4];
double g_accelerationValues[4];
GdkColor g_LEDColor;
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

void setMotorWidgetsSensitive(int motor, bool sensitive)
{
  char buf[256];
  sprintf(buf, "vscale_motorspeed%d", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
  sprintf(buf, "entry_motorSpeed%d", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
  sprintf(buf, "button_motor%dforward", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
  sprintf(buf, "button_motor%dstop", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
  sprintf(buf, "button_motor%dback", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
  sprintf(buf, "vscale_motorPos%d", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
  sprintf(buf, "entry_motorPos%d", motor);
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, buf)),
      sensitive );
}

void setColorWidgetSensitive(bool sensitive)
{
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "colorselection")),
      sensitive );
}

void setAccelWidgetSensitive(bool sensitive)
{
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelx")),
      sensitive );
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accely")),
      sensitive );
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelz")),
      sensitive );
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelmag")),
      sensitive );
}

void setRollingControlSensitive(bool sensitive)
{
  if(sensitive) {
    gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_forward")));
    gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_rotateLeft")));
    gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_rotateRight")));
    gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_backward")));
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label21")),
        "Rolling Control"
        );
  } else {
    gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_forward")));
    gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_rotateLeft")));
    gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_rotateRight")));
    gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_backward")));
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label21")),
        "Stop"
        );
  }
}

void setMotionsSensitive(bool sensitive)
{
  GtkWidget* w;
  /*
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "treeview_gaits"));
  gtk_widget_set_sensitive(w, sensitive);
  */
  /* Hide the whole frame for now */
  if(!sensitive) {
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "frame9"));
    gtk_widget_hide(w);
  } else {
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "frame9"));
    gtk_widget_show(w);
  }
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

  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelx"));
  gtk_range_set_range(GTK_RANGE(w), -5, 5);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accely"));
  gtk_range_set_range(GTK_RANGE(w), -5, 5);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelz"));
  gtk_range_set_range(GTK_RANGE(w), -5, 5);
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelmag"));
  gtk_range_set_range(GTK_RANGE(w), 0, 5);

  /* Initialize the color selection widget */
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "colorselection"));
  gtk_color_selection_set_update_policy(GTK_COLOR_SELECTION(w), GTK_UPDATE_CONTINUOUS);

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

void hideJoint4Widgets()
{
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "label26")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos4")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorPos4")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "label_motorPos4")));

  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "label20")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed4")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorSpeed4")));

  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "label15")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_motor4forward")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_motor4stop")));
  gtk_widget_hide( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_motor4back")));
}

void showJoint4Widgets()
{
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "label26")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos4")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorPos4")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "label_motorPos4")));

  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "label20")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed4")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "entry_motorSpeed4")));

  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "label15")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_motor4forward")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_motor4stop")));
  gtk_widget_show( GTK_WIDGET(gtk_builder_get_object(g_builder, "button_motor4back")));
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
  static uint8_t motorMask = 0x0F;
  int rc;
  static int form;
  static int formFactorInitialized;
  char buf[80];

  static GtkWidget 
    *vscale_motorPos[4],
    *label_motorPos[4],
    *vscale_motorspeed[4],
    *vscale_accel[4];
  static int init = 1;

  if(init) {
    for(i = 0; i < 4; i++) {
      sprintf(buf, "vscale_motorPos%d", i+1);
      vscale_motorPos[i] = GTK_WIDGET(gtk_builder_get_object(g_builder, buf)); 
      sprintf(buf, "label_motorPos%d", i+1);
      label_motorPos[i] = GTK_WIDGET(gtk_builder_get_object(g_builder, buf)); 
      sprintf(buf, "vscale_motorspeed%d", i+1);
      vscale_motorspeed[i] = GTK_WIDGET(gtk_builder_get_object(g_builder, buf)); 
    }
    vscale_accel[0] = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelx"));
    vscale_accel[1] = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accely"));
    vscale_accel[2] = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelz"));
    vscale_accel[3] = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_accelmag"));
    init = 0;
  }

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
    MUTEX_LOCK(&g_activeMobotLock);
    g_activeMobot = NULL;
    MUTEX_UNLOCK(&g_activeMobotLock);
    return true;
  }
  //MUTEX_LOCK(&g_activeMobotLock);
  if(mobot != g_activeMobot) {
    g_activeMobot = mobot;
    /* Get the form factor and disable certain widgets if necessary */
    rc = Mobot_getFormFactor((mobot_t*)g_activeMobot, &form);
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "image_jointDiagram"));
    if(rc) {
      /* Normal Mobot. Enable all widgets*/
      setMotorWidgetsSensitive(2, true);
      setMotorWidgetsSensitive(3, true);
      setMotorWidgetsSensitive(4, true);
      setColorWidgetSensitive(false);
      setAccelWidgetSensitive(false);
      setRollingControlSensitive(true);
      setMotionsSensitive(true);
      showJoint4Widgets();
      gtk_image_set_from_file(GTK_IMAGE(w), "interface/imobot_diagram.png");
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos2"));
      gtk_range_set_range(GTK_RANGE(w), -90, 90);
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos3"));
      gtk_range_set_range(GTK_RANGE(w), -90, 90);
      motorMask = 0x0F;
    } else if (form == MOBOTFORM_L) {
      /* Disable widgets for motors 3 and 4 */
      setMotorWidgetsSensitive(2, true);
      setMotorWidgetsSensitive(3, false);
      setMotorWidgetsSensitive(4, false);
      setColorWidgetSensitive(true);
      setAccelWidgetSensitive(true);
      setRollingControlSensitive(false);
      setMotionsSensitive(false);
      hideJoint4Widgets();
      gtk_image_set_from_file(GTK_IMAGE(w), "interface/DOF_joint_diagram.png");
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos2"));
      gtk_range_set_range(GTK_RANGE(w), -180, 180);
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos3"));
      gtk_range_set_range(GTK_RANGE(w), -180, 180);
      motorMask = 0x03;
    } else if (form == MOBOTFORM_I) {
      /* Disable widgets for motors 2 and 4 */
      setMotorWidgetsSensitive(2, false);
      setMotorWidgetsSensitive(3, true);
      setMotorWidgetsSensitive(4, false);
      gtk_image_set_from_file(GTK_IMAGE(w), "interface/DOF_joint_diagram.png");
      setColorWidgetSensitive(true);
      setAccelWidgetSensitive(true);
      setRollingControlSensitive(true);
      setMotionsSensitive(false);
      hideJoint4Widgets();
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos2"));
      gtk_range_set_range(GTK_RANGE(w), -180, 180);
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos3"));
      gtk_range_set_range(GTK_RANGE(w), -180, 180);
      motorMask = 0x05;
    } else {
      /* Enable all widgets */
      setMotorWidgetsSensitive(2, true);
      setMotorWidgetsSensitive(3, true);
      setMotorWidgetsSensitive(4, true);
      setColorWidgetSensitive(false);
      setAccelWidgetSensitive(false);
      setRollingControlSensitive(true);
      setMotionsSensitive(true);
      showJoint4Widgets();
      gtk_image_set_from_file(GTK_IMAGE(w), "interface/imobot_diagram.png");
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos2"));
      gtk_range_set_range(GTK_RANGE(w), -90, 90);
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos3"));
      gtk_range_set_range(GTK_RANGE(w), -90, 90);
      motorMask = 0x0F;
    }
    /* Set the color widget */
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "colorselection"));
    int r, g, b;
    Mobot_getColorRGB((mobot_t*)g_activeMobot, &r, &g, &b);
    GdkColor color;
    color.red = r*(65535/255);
    color.green = g*(65535/255);
    color.blue = b*(65535/255);
    gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(w), &color);
  }
  //MUTEX_UNLOCK(&g_activeMobotLock);

  for(i = 0; i < 4; i++) {
    if(motorMask & (1<<i)) {
      if(g_buttonState[S_POS1+i] == 0) { 
        gtk_range_set_value(GTK_RANGE(vscale_motorPos[i]), normalizeDeg(g_positionValues[i])); 
      } 
      sprintf(buf, "%3.2lf", g_positionValues[i]); 
      gtk_label_set_text(GTK_LABEL(label_motorPos[i]), buf); 
    } else {
      gtk_range_set_value(GTK_RANGE(vscale_motorPos[i]), 0); 
      sprintf(buf, "N/A"); 
      gtk_label_set_text(GTK_LABEL(label_motorPos[i]), buf); 
    }
  }
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

    for(i = 0; i < 4; i++) {
      gtk_range_set_value(GTK_RANGE(vscale_motorspeed[i]), RAD2DEG(angles[i])); 
    }
    g_initSpeeds = 0;
  }
  if(
      (form == MOBOTFORM_I) || (form == MOBOTFORM_L)
    )
  {
    /* Set acceleration sliders */
    for(i = 0; i < 4; i++) {
      gtk_range_set_value(GTK_RANGE(vscale_accel[i]), g_accelerationValues[i]);
    }
  }

  /* Get slider, entry values */
  for(i = 0; i < 4; i++ ) {
    g_positionSliderValues[i] = gtk_range_get_value(GTK_RANGE(vscale_motorPos[i]));
    g_speedSliderValues[i] = gtk_range_get_value(GTK_RANGE(vscale_motorspeed[i]));
  }
  return true;
}

void* controllerHandlerThread(void* arg)
{
  int i;
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
    if(g_controlMode == 0) {
      /* First, get motor position values */
      int rc = Mobot_getJointAngles(
          (mobot_t*)g_activeMobot,
          &g_positionValues[0],
          &g_positionValues[1],
          &g_positionValues[2],
          &g_positionValues[3]);
      /* Convert angles to degrees */
      for(i = 0; i < 4; i++) {
        //g_positionValues[i] = normalizeAngleRad(g_positionValues[i]);
        if(!rc) {
          g_positionValues[i] = RAD2DEG(g_positionValues[i]);
        }
        /*
           else {
           g_positionValues[i] = 0;
           }
         */
      }
    } else if (g_controlMode == 1) {
      /* Now get Acceleration Values */
      Mobot_getAccelerometerData(
          (mobot_t*)g_activeMobot,
          &g_accelerationValues[0],
          &g_accelerationValues[1],
          &g_accelerationValues[2]);
      g_accelerationValues[3] = 
        sqrt (
            (g_accelerationValues[0] * g_accelerationValues[0]) + 
            (g_accelerationValues[1] * g_accelerationValues[1]) + 
            (g_accelerationValues[2] * g_accelerationValues[2]) 
            );
    }

    MUTEX_UNLOCK(&g_activeMobotLock);
    /*
    if(rc) {
      MUTEX_LOCK(&g_activeMobotLock);
      g_activeMobot = NULL;
      MUTEX_UNLOCK(&g_activeMobotLock);
      continue;
    }
    */
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
  if(
      (((mobot_t*)arg)->formFactor == MOBOTFORM_I) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_L) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_T) 
    )
  {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_FORWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT3, MOBOT_FORWARD);
  } else {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_FORWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_FORWARD);
  }
  return 0;
}

int handlerTURNLEFT(void* arg)
{
  //Mobot_motionTurnLeftNB((mobot_t*)arg, DEG2RAD(90));
  if(
      (((mobot_t*)arg)->formFactor == MOBOTFORM_I) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_L) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_T) 
    )
  {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_BACKWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT3, MOBOT_FORWARD);
  } else {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_BACKWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_FORWARD);
  }
  return 0;
}

int handlerTURNRIGHT(void* arg)
{
  //Mobot_motionTurnRightNB((mobot_t*)arg, DEG2RAD(90));
  if(
      (((mobot_t*)arg)->formFactor == MOBOTFORM_I) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_L) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_T) 
    )
  {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_FORWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT3, MOBOT_BACKWARD);
  } else {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_FORWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_BACKWARD);
  }
  return 0;
}

int handlerROLLBACK(void* arg)
{
  //Mobot_motionRollBackwardNB((mobot_t*)arg, DEG2RAD(90));
  if(
      (((mobot_t*)arg)->formFactor == MOBOTFORM_I) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_L) ||
      (((mobot_t*)arg)->formFactor == MOBOTFORM_T) 
    )
  {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_BACKWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT3, MOBOT_BACKWARD);
  } else {
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT1, MOBOT_BACKWARD);
    Mobot_moveJointContinuousNB((mobot_t*)arg, MOBOT_JOINT4, MOBOT_BACKWARD);
  }
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

int handlerCOLORSELECTION(void* arg)
{
  /* Get the color from the color selection and set the Mobot RGB to that color
   * */
  Mobot_setColorRGB((mobot_t*)g_activeMobot, 
      g_LEDColor.red / (256),
      g_LEDColor.green / (256),
      g_LEDColor.blue / (256));
}

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
  int index;
  index = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  if(index != -1) {
    g_selectedRobot = index;
  }
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
  if((text != NULL) && (strlen(text) != 0)) { \
    sscanf(text, "%lf", &value); \
    g_speedEntryValues[n-1] = value; \
    g_speedEntryValuesValid[n-1] = true; \
    /* Set the slider value */ \
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorspeed" #n)); \
    if(value < 0) { value = 0; } \
    if(value > 120) {value = 120;} \
    gtk_range_set_value(GTK_RANGE(w), value);  \
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
  if((text != NULL) && (strlen(text) > 0)) { \
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

void on_colorselection_color_changed(GtkColorSelection *w, gpointer user_data)
{
  g_buttonState[S_COLORSELECTION] = 1;
  gtk_color_selection_get_current_color(w, &g_LEDColor);
}
