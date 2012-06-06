#include "RoboMancer.h"
#include "controlDialog.h"
#include "thread_macros.h"

/* The button state: For Buttons (B_), a value of 1 indicates that the button
 * has been pressed. For the sliders, 1 indicates that the slider is currently
 * depressed, and 0 indicates that it is not. */
int g_buttonState[NUM_BUTTONS];

typedef int(*handlerFunc)(void* arg);
handlerFunc g_handlerFuncs[NUM_BUTTONS];

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

  /* Start handler thread */
  THREAD_T thread;
  THREAD_CREATE(&thread, controllerHandlerThread, NULL);

}

void* controllerHandlerThread(void* arg)
{
  /* This function will repeatedly check to see if there have been any button
   * presses and process them, as well as update the UI */
  int i;
  int index;
  GtkWidget* w;
  recordMobot_t* mobot;
  double angles[4];
  while(1) {
    /* First, check to see if a robot is even selected. If none selected, just return. */
    gdk_threads_enter();
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "combobox_connectedRobots"));
    index = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    gdk_threads_leave();
    if(index < 0) {
      usleep(250000);
      continue;
    }
    /* Get the controlled mobot */
    mobot = g_robotManager->getMobot(index);
    /* Update the position sliders */
    Mobot_getJointAngles((mobot_t*)mobot, 
        &angles[0], 
        &angles[1], 
        &angles[2], 
        &angles[3]);
    /* Convert angles to degrees */
    for(i = 0; i < 4; i++) {
      angles[i] = RAD2DEG(angles[i]);
    }
#define VSCALEHANDLER(n) \
    if(g_buttonState[S_POS##n] == 0) { \
      gdk_threads_enter(); \
      w = GTK_WIDGET(gtk_builder_get_object(g_builder, "vscale_motorPos" #n)); \
      gtk_range_set_value(GTK_RANGE(w), angles[n-1]); \
      gdk_threads_leave(); \
    }
    VSCALEHANDLER(1)
    VSCALEHANDLER(2)
    VSCALEHANDLER(3)
    VSCALEHANDLER(4)
#undef VSCALEHANDLER
    /* Go through the buttons and handle the pressed ones */
    for(i = 0; i < NUM_BUTTONS; i++) {
      if(g_buttonState[i]) {
        g_handlerFuncs[i](NULL);
        g_buttonState[i] = 0;
      }
    }
    usleep(100000);
  }
}

int handlerZERO(void* arg)
{
}

#define HANDLER_FORWARD(n) \
int handlerJ##n##FORWARD(void* arg) \
{ \
}
HANDLER_FORWARD(1)
HANDLER_FORWARD(2)
HANDLER_FORWARD(3)
HANDLER_FORWARD(4)
#undef HANDLER_FORWARD

#define HANDLER_BACKWARD(n) \
int handlerJ##n##BACK(void* arg) \
{ \
}
HANDLER_BACKWARD(1)
HANDLER_BACKWARD(2)
HANDLER_BACKWARD(3)
HANDLER_BACKWARD(4)
#undef HANDLER_BACKWARD

#define HANDLER_STOP(n) \
int handlerJ##n##STOP(void* arg) \
{ \
}
HANDLER_STOP(1)
HANDLER_STOP(2)
HANDLER_STOP(3)
HANDLER_STOP(4)
#undef HANDLER_STOP

int handlerROLLFORWARD(void* arg)
{
}

int handlerTURNLEFT(void* arg)
{
}

int handlerTURNRIGHT(void* arg)
{
}

int handlerROLLBACK(void* arg)
{
}

int handlerSTOP(void* arg)
{
}

int handlerMOVE(void* arg)
{
}

int handlerMOVETO(void* arg)
{
}

int handlerPLAY(void* arg)
{
}

#define HANDLER_SPEED(n) \
int handlerSPEED##n(void* arg) \
{ \
}
HANDLER_SPEED(1)
HANDLER_SPEED(2)
HANDLER_SPEED(3)
HANDLER_SPEED(4)
#undef HANDLER_SPEED

#define HANDLER_POS(n) \
int handlerPOS##n(void*arg) \
{ \
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


