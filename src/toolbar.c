/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*/

#include <gtk/gtk.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "actions.h"
#include "agc.h"
#include "band.h"
#include "bandstack.h"
#include "channel.h"
#include "discovered.h"
#include "ext.h"
#include "filter.h"
#include "gpio.h"
#include "message.h"
#include "mode.h"
#include "new_menu.h"
#include "new_protocol.h"
#include "old_protocol.h"
#include "property.h"
#include "radio.h"
#include "receiver.h"
#include "toolbar.h"
#include "transmitter.h"
#include "vfo.h"

int function = 0;

static int width;
static int height;

static GtkWidget *toolbar;

static GtkWidget *sim_s0 = NULL;
static GtkWidget *sim_s1;
static GtkWidget *sim_s2;
static GtkWidget *sim_s3;
static GtkWidget *sim_s4;
static GtkWidget *sim_s5;
static GtkWidget *sim_s6;
static GtkWidget *sim_s7;

SWITCH *toolbar_switches = switches_controller1[0];

void update_toolbar_labels() {
  if (sim_s0) {
    //
    // If the toolbar has not yet been on display,
    // sim_s0 and friends are NULL
    //
    gtk_button_set_label(GTK_BUTTON(sim_s0), ActionTable[toolbar_switches[0].switch_function].button_str);
    gtk_button_set_label(GTK_BUTTON(sim_s1), ActionTable[toolbar_switches[1].switch_function].button_str);
    gtk_button_set_label(GTK_BUTTON(sim_s2), ActionTable[toolbar_switches[2].switch_function].button_str);
    gtk_button_set_label(GTK_BUTTON(sim_s3), ActionTable[toolbar_switches[3].switch_function].button_str);
    gtk_button_set_label(GTK_BUTTON(sim_s4), ActionTable[toolbar_switches[4].switch_function].button_str);
    gtk_button_set_label(GTK_BUTTON(sim_s5), ActionTable[toolbar_switches[5].switch_function].button_str);
    gtk_button_set_label(GTK_BUTTON(sim_s6), ActionTable[toolbar_switches[6].switch_function].button_str);

    if (toolbar_switches[7].switch_function == FUNCTION) {
      char lbl[16];
      snprintf(lbl, sizeof(lbl), "FNC(%d)", function);
      gtk_button_set_label(GTK_BUTTON(sim_s7), lbl);
    } else {
      gtk_button_set_label(GTK_BUTTON(sim_s7), ActionTable[toolbar_switches[7].switch_function].button_str);
    }
  }
}

// cppcheck-suppress constParameterCallback
static void toolbar_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data) {
  int i = GPOINTER_TO_INT(data);

  //
  // Filter out any special events generated by double or triple clicks
  //
  if (event->type != GDK_BUTTON_PRESS) { return; }

  //
  // This is our "easter egg" for the FUNC toolbar button. A secondary (right) click
  // cycles backwards
  //
  if (i == 7  && event->button == GDK_BUTTON_SECONDARY) {
    schedule_action(FUNCTIONREV, PRESSED, 0);
  } else {
    schedule_action(toolbar_switches[i].switch_function, PRESSED, 0);
  }
}

// cppcheck-suppress constParameterCallback
static void toolbar_button_released_cb(GtkWidget *widget, GdkEventButton *event, gpointer data) {
  int i = GPOINTER_TO_INT(data);
  //t_print("%s: %d action=%d\n",__FUNCTION__,i,toolbar_switches[i].switch_function);
  schedule_action(toolbar_switches[i].switch_function, RELEASED, 0);
}

GtkWidget *toolbar_init(int my_width, int my_height) {
  width = my_width;
  height = my_height;
  int button_width = width / 8;
  const char *button_css;
  t_print("toolbar_init: width=%d height=%d button_width=%d\n", width, height, button_width);

  if (height < 40) {
    button_css = "small_button";
  } else if (height < 50) {
    button_css = "medium_button";
  } else {
    button_css = "large_button";
  }

  toolbar_switches = switches_controller1[function];
  toolbar = gtk_grid_new();
  gtk_widget_set_size_request (toolbar, width, height);
  gtk_grid_set_column_homogeneous(GTK_GRID(toolbar), TRUE);
  sim_s0 = gtk_button_new_with_label(ActionTable[toolbar_switches[0].switch_function].button_str);
  gtk_widget_set_name(sim_s0, button_css);
  gtk_widget_set_size_request (sim_s0, button_width, height);
  g_signal_connect(G_OBJECT(sim_s0), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(0));
  g_signal_connect(G_OBJECT(sim_s0), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(0));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s0, 0, 0, 4, 1);
  sim_s1 = gtk_button_new_with_label(ActionTable[toolbar_switches[1].switch_function].button_str);
  gtk_widget_set_name(sim_s1, button_css);
  gtk_widget_set_size_request (sim_s1, button_width, height);
  g_signal_connect(G_OBJECT(sim_s1), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(1));
  g_signal_connect(G_OBJECT(sim_s1), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(1));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s1, 4, 0, 4, 1);
  sim_s2 = gtk_button_new_with_label(ActionTable[toolbar_switches[2].switch_function].button_str);
  gtk_widget_set_name(sim_s2, button_css);
  gtk_widget_set_size_request (sim_s2, button_width, height);
  g_signal_connect(G_OBJECT(sim_s2), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(2));
  g_signal_connect(G_OBJECT(sim_s2), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(2));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s2, 8, 0, 4, 1);
  sim_s3 = gtk_button_new_with_label(ActionTable[toolbar_switches[3].switch_function].button_str);
  gtk_widget_set_name(sim_s3, button_css);
  gtk_widget_set_size_request (sim_s3, button_width, height);
  g_signal_connect(G_OBJECT(sim_s3), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(3));
  g_signal_connect(G_OBJECT(sim_s3), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(3));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s3, 12, 0, 4, 1);
  sim_s4 = gtk_button_new_with_label(ActionTable[toolbar_switches[4].switch_function].button_str);
  gtk_widget_set_name(sim_s4, button_css);
  gtk_widget_set_size_request (sim_s4, button_width, height);
  g_signal_connect(G_OBJECT(sim_s4), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(4));
  g_signal_connect(G_OBJECT(sim_s4), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(4));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s4, 16, 0, 4, 1);
  sim_s5 = gtk_button_new_with_label(ActionTable[toolbar_switches[5].switch_function].button_str);
  gtk_widget_set_name(sim_s5, button_css);
  gtk_widget_set_size_request (sim_s5, button_width, height);
  g_signal_connect(G_OBJECT(sim_s5), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(5));
  g_signal_connect(G_OBJECT(sim_s5), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(5));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s5, 20, 0, 4, 1);
  sim_s6 = gtk_button_new_with_label(ActionTable[toolbar_switches[6].switch_function].button_str);
  gtk_widget_set_name(sim_s6, button_css);
  gtk_widget_set_size_request (sim_s6, button_width, height);
  g_signal_connect(G_OBJECT(sim_s6), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(6));
  g_signal_connect(G_OBJECT(sim_s6), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(6));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s6, 24, 0, 4, 1);

  //
  // For the FUNC button, include the layer in the description
  //
  if (toolbar_switches[7].switch_function == FUNCTION) {
    char lbl[16];
    snprintf(lbl, sizeof(lbl), "FNC(%d)", function);
    sim_s7 = gtk_button_new_with_label(lbl);
  } else {
    sim_s7 = gtk_button_new_with_label(ActionTable[toolbar_switches[7].switch_function].button_str);
  }

  gtk_widget_set_name(sim_s7, button_css);
  gtk_widget_set_size_request (sim_s7, button_width, height);
  g_signal_connect(G_OBJECT(sim_s7), "button-press-event", G_CALLBACK(toolbar_button_press_cb), GINT_TO_POINTER(7));
  g_signal_connect(G_OBJECT(sim_s7), "button-release-event", G_CALLBACK(toolbar_button_released_cb), GINT_TO_POINTER(7));
  gtk_grid_attach(GTK_GRID(toolbar), sim_s7, 28, 0, 4, 1);
  gtk_widget_show_all(toolbar);
  return toolbar;
}
