/* Copyright (C)
* 2016 - John Melton, G0ORX/N6LYT
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wdsp.h>

#include "new_menu.h"
#include "noise_menu.h"
#include "band.h"
#include "bandstack.h"
#include "filter.h"
#include "mode.h"
#include "radio.h"
#include "vfo.h"
#include "button_text.h"
#include "ext.h"

static GtkWidget *dialog=NULL;
//
// NR controls
//
static GtkWidget *gain_title;
static GtkWidget *gain_combo;
static GtkWidget *npe_title;
static GtkWidget *npe_combo;
static GtkWidget *pos_title;
static GtkWidget *pos_combo;
static GtkWidget *b_ae;

//
// NB controls
//
static GtkWidget *mode_title;
static GtkWidget *mode_combo;
static GtkWidget *slew_title;
static GtkWidget *slew_b;
static GtkWidget *lead_title;
static GtkWidget *lead_b;
static GtkWidget *lag_title;
static GtkWidget *lag_b;
static GtkWidget *thresh_title;
static GtkWidget *thresh_b;

#ifdef EXTNR
//
// NR4 controls
//
static GtkWidget *nr4_reduction_title;
static GtkWidget *nr4_reduction_b;
static GtkWidget *nr4_smoothing_title;
static GtkWidget *nr4_smoothing_b;
static GtkWidget *nr4_whitening_title;
static GtkWidget *nr4_whitening_b;
static GtkWidget *nr4_rescale_title;
static GtkWidget *nr4_rescale_b;
static GtkWidget *nr4_threshold_title;
static GtkWidget *nr4_threshold_b;
#endif

static void cleanup() {
  if(dialog!=NULL) {
    gtk_widget_destroy(dialog);
    dialog=NULL;
    sub_menu=NULL;
    active_menu=NO_MENU;
  }
}

static gboolean close_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  cleanup();
  return TRUE;
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
  cleanup();
  return FALSE;
}

void set_noise() {
  //
  // Set/Update all parameters associated with the "QRM fighters"
  //
  // a) NB
  //
  SetEXTANBTau(active_receiver->id, active_receiver->nb_tau);
  SetEXTANBHangtime(active_receiver->id, active_receiver->nb_hang);
  SetEXTANBAdvtime(active_receiver->id, active_receiver->nb_advtime);
  SetEXTANBThreshold(active_receiver->id, active_receiver->nb_thresh);
  SetEXTANBRun(active_receiver->id, (active_receiver->nb == 1));
  //
  // b) NB2
  //
  SetEXTNOBMode(active_receiver->id, active_receiver->nb2_mode);
  SetEXTNOBTau(active_receiver->id, active_receiver->nb_tau);
  SetEXTNOBHangtime(active_receiver->id, active_receiver->nb_hang);
  SetEXTNOBAdvtime(active_receiver->id, active_receiver->nb_advtime);
  SetEXTNOBThreshold(active_receiver->id, active_receiver->nb_thresh);
  SetEXTNOBRun(active_receiver->id, (active_receiver->nb == 2));
  //
  // c) NR
  //
  SetRXAANRVals(active_receiver->id, 64, 16, 16e-4, 10e-7);
  SetRXAANRPosition(active_receiver->id, active_receiver->nr_agc);
  SetRXAANRRun(active_receiver->id, (active_receiver->nr == 1));
  //
  // d) NB2
  //
  SetRXAEMNRPosition(active_receiver->id, active_receiver->nr_agc);
  SetRXAEMNRgainMethod(active_receiver->id, active_receiver->nr2_gain_method);
  SetRXAEMNRnpeMethod(active_receiver->id, active_receiver->nr2_npe_method);
  SetRXAEMNRaeRun(active_receiver->id, active_receiver->nr2_ae);
  SetRXAEMNRRun(active_receiver->id, (active_receiver->nr == 2));
  //
  // e) ANF
  //
  SetRXAANFRun(active_receiver->id, active_receiver->anf);
  SetRXAANFPosition(active_receiver->id, active_receiver->nr_agc);
  //
  // f) SNB
  //
  SetRXASNBARun(active_receiver->id, active_receiver->snb);

#ifdef EXTNR
  //
  // g) NR3
  //
  SetRXARNNRRun(active_receiver->id, (rx->nr==3));

  //
  // NR4
  //
  SetRXASBNRreductionAmount(active_receiver->id,     active_receiver->nr4_reduction_amount);
  SetRXASBNRsmoothingFactor(active_receiver->id,     active_receiver->nr4_smoothing_factor);
  SetRXASBNRwhiteningFactor(active_receiver->id,     active_receiver->nr4_whitening_factor);
  SetRXASBNRnoiseRescale(active_receiver->id,        active_receiver->nr4_noise_rescale);
  SetRXASBNRpostFilterThreshold(active_receiver->id, active_receiver->nr4_post_filter_threshold);
  SetRXASBNRRun(active_receiver->id, (rx->nr == 4));
#endif

  g_idle_add(ext_vfo_update,NULL);
}

void update_noise() {
#ifdef CLIENT_SERVER
  //
  // TODO: include parameters
  //
  if(radio_is_remote) {
    send_noise(client_socket,active_receiver->id,active_receiver->nb,active_receiver->nb2,active_receiver->nr,active_receiver->nr2,active_receiver->anf,active_receiver->snb);
  } else {
#endif
    set_noise();
#ifdef CLIENT_SERVER
  }
#endif
}

static void nb_cb(GtkToggleButton *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
  active_receiver->nb=val;
  mode_settings[vfo[active_receiver->id].mode].nb=val;
  update_noise();
}

static void nr_cb(GtkToggleButton *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
  active_receiver->nr=val;
  mode_settings[vfo[active_receiver->id].mode].nr=val;
  update_noise();
}

static void anf_cb(GtkWidget *widget, gpointer data) {
  active_receiver->anf=gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  mode_settings[vfo[active_receiver->id].mode].anf=active_receiver->anf;
  update_noise();
}

static void snb_cb(GtkWidget *widget, gpointer data) {
  active_receiver->snb=gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  mode_settings[vfo[active_receiver->id].mode].snb=active_receiver->snb;
  update_noise();
}

static void ae_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr2_ae=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  update_noise();
}

static void pos_cb(GtkWidget *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
  active_receiver->nr_agc=val;
  update_noise();
}

static void mode_cb(GtkWidget *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
  active_receiver->nb2_mode=val;
  update_noise();
}

static void gain_cb(GtkWidget *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
  active_receiver->nr2_gain_method=val;
  update_noise();
}

static void npe_cb(GtkWidget *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
  active_receiver->nr2_npe_method=val;
  update_noise();
}

static void slew_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nb_tau=0.001*gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void lead_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nb_advtime=0.001*gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void lag_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nb_hang=0.001*gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void thresh_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nb_thresh=0.165*gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void nr_sel_changed(GtkWidget *widget, gpointer data) {
  // show or hide all controls for NR settings
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    gtk_widget_show(gain_title);
    gtk_widget_show(gain_combo);
    gtk_widget_show(npe_title);
    gtk_widget_show(npe_combo);
    gtk_widget_show(pos_title);
    gtk_widget_show(pos_combo);
    gtk_widget_show(b_ae);
  } else {
    gtk_widget_hide(gain_title);
    gtk_widget_hide(gain_combo);
    gtk_widget_hide(npe_title);
    gtk_widget_hide(npe_combo);
    gtk_widget_hide(pos_title);
    gtk_widget_hide(pos_combo);
    gtk_widget_hide(b_ae);
  }
}

static void nb_sel_changed(GtkWidget *widget, gpointer data) {
  // show or hide all controls for NB settings
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    gtk_widget_show(mode_title);
    gtk_widget_show(mode_combo);
    gtk_widget_show(slew_title);
    gtk_widget_show(slew_b);
    gtk_widget_show(lead_title);
    gtk_widget_show(lead_b);
    gtk_widget_show(lag_title);
    gtk_widget_show(lag_b);
    gtk_widget_show(thresh_title);
    gtk_widget_show(thresh_b);
  } else {
    gtk_widget_hide(mode_title);
    gtk_widget_hide(mode_combo);
    gtk_widget_hide(slew_title);
    gtk_widget_hide(slew_b);
    gtk_widget_hide(lead_title);
    gtk_widget_hide(lead_b);
    gtk_widget_hide(lag_title);
    gtk_widget_hide(lag_b);
    gtk_widget_hide(thresh_title);
    gtk_widget_hide(thresh_b);
  }
}

#ifdef EXTNR
static void nr4_sel_changed(GtkWidget *widget, gpointer data) {
  // show or hide all controls for NB settings
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    gtk_widget_show(nr4_reduction_title);
    gtk_widget_show(nr4_reduction_b);
    gtk_widget_show(nr4_smoothing_title);
    gtk_widget_show(nr4_smoothing_b);
    gtk_widget_show(nr4_whitening_title);
    gtk_widget_show(nr4_whitening_b);
    gtk_widget_show(nr4_rescale_title);
    gtk_widget_show(nr4_rescale_b);
    gtk_widget_show(nr4_threshold_title);
    gtk_widget_show(nr4_threshold_b);
  } else {
    gtk_widget_hide(nr4_reduction_title);
    gtk_widget_hide(nr4_reduction_b);
    gtk_widget_hide(nr4_smoothing_title);
    gtk_widget_hide(nr4_smoothing_b);
    gtk_widget_hide(nr4_whitening_title);
    gtk_widget_hide(nr4_whitening_b);
    gtk_widget_hide(nr4_rescale_title);
    gtk_widget_hide(nr4_rescale_b);
    gtk_widget_hide(nr4_threshold_title);
    gtk_widget_hide(nr4_threshold_b);
  }
}

static void nr4_reduction_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_reduction_amount=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void nr4_smoothing_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_smoothing_factor=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void nr4_whitening_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_whitening_factor = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void nr4_rescale_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_noise_rescale=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}

static void nr4_threshold_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_post_filter_threshold=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  update_noise();
}
#endif

void noise_menu(GtkWidget *parent) {

  fprintf(stderr,"HANG=%f\n", active_receiver->nb_hang);
  fprintf(stderr,"THRE=%f\n", active_receiver->nb_thresh);

  dialog=gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  char title[64];
  sprintf(title,"piHPSDR - Noise (RX %d VFO %s)",active_receiver->id,active_receiver->id==0?"A":"B");
  gtk_window_set_title(GTK_WINDOW(dialog),title);
  g_signal_connect (dialog, "delete_event", G_CALLBACK (delete_event), NULL);
  set_backgnd(dialog);

  GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  GtkWidget *grid=gtk_grid_new();

  gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid),TRUE);
  gtk_grid_set_column_spacing (GTK_GRID(grid),5);
  gtk_grid_set_row_spacing (GTK_GRID(grid),5);

  GtkWidget *close_b=gtk_button_new_with_label("Close");
  g_signal_connect (close_b, "pressed", G_CALLBACK(close_cb), NULL);
  gtk_grid_attach(GTK_GRID(grid),close_b,0,0,1,1);

  //
  // First row: select NB and NR method
  //
  GtkWidget *nb_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nb_title), "<b>Noise Blanker</b>");
  gtk_widget_show(nb_title);
  gtk_grid_attach(GTK_GRID(grid),nb_title,0,1,1,1);

  GtkWidget *nb_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nb_combo),NULL,"NONE");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nb_combo),NULL,"NB");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nb_combo),NULL,"NB2");
  gtk_combo_box_set_active(GTK_COMBO_BOX(nb_combo),active_receiver->nb);
  my_combo_attach(GTK_GRID(grid), nb_combo, 1, 1, 1, 1);
  g_signal_connect(nb_combo,"changed",G_CALLBACK(nb_cb),NULL);

  GtkWidget *nr_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr_title), "<b>Noise Reduction</b>");
  gtk_widget_show(nr_title);
  gtk_grid_attach(GTK_GRID(grid),nr_title,2,1,1,1);

  GtkWidget *nr_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nr_combo),NULL,"NONE");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nr_combo),NULL,"NR");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nr_combo),NULL,"NR2");
#ifdef EXTNR
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nr_combo),NULL,"NR3");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(nr_combo),NULL,"NR4");
#endif
  gtk_combo_box_set_active(GTK_COMBO_BOX(nr_combo),active_receiver->nr);
  my_combo_attach(GTK_GRID(grid), nr_combo, 3, 1, 1, 1);
  g_signal_connect(nr_combo,"changed",G_CALLBACK(nr_cb),NULL);

  //
  // Second row: select SNB/ANF
  //
  GtkWidget *b_snb=gtk_check_button_new_with_label("SNB");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_snb), active_receiver->snb);
  gtk_widget_show(b_snb);
  gtk_grid_attach(GTK_GRID(grid),b_snb,0,2,1,1);
  g_signal_connect(b_snb,"toggled",G_CALLBACK(snb_cb),NULL);

  GtkWidget *b_anf=gtk_check_button_new_with_label("ANF");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_anf), active_receiver->anf);
  gtk_widget_show(b_anf);
  gtk_grid_attach(GTK_GRID(grid),b_anf,1,2,1,1);
  g_signal_connect(b_anf,"toggled",G_CALLBACK(anf_cb),NULL);

  //
  // Third row: select settings: NR, NB, NR4 settings
  //
  GtkWidget *nr_sel = gtk_radio_button_new_with_label_from_widget(NULL,"NR Settings");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nr_sel), 1);
  gtk_widget_show(nr_sel);
  gtk_grid_attach(GTK_GRID(grid), nr_sel, 0,3,1,1);
  g_signal_connect(nr_sel,"toggled",G_CALLBACK(nr_sel_changed), NULL);

  GtkWidget *nb_sel = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(nr_sel),"NB Settings");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nb_sel), 0);
  gtk_widget_show(nb_sel);
  gtk_grid_attach(GTK_GRID(grid), nb_sel, 1,3,1,1);
  g_signal_connect(nb_sel,"toggled",G_CALLBACK(nb_sel_changed), NULL);

#ifdef EXTNR
  GtkWidget *nr4_sel = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(nr_sel),"NR4 Settings");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nr4_sel), 0);
  gtk_widget_show(nr4_sel);
  gtk_grid_attach(GTK_GRID(grid), nr4_sel, 2,3,1,1);
  g_signal_connect(nr4_sel,"toggled",G_CALLBACK(nr4_sel_changed), NULL);
#endif

  //
  // NR controls starting on row 4 SHOWN
  //
  gain_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(gain_title), "<b>NR2 Gain Method</b>");
  gtk_widget_show(gain_title);
  gtk_grid_attach(GTK_GRID(grid),gain_title,0,4,1,1);

  gain_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gain_combo),NULL,"Linear");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gain_combo),NULL,"Log");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gain_combo),NULL,"Gamma");
  gtk_combo_box_set_active(GTK_COMBO_BOX(gain_combo),active_receiver->nr2_gain_method);
  my_combo_attach(GTK_GRID(grid), gain_combo, 1, 4, 1, 1);
  g_signal_connect(gain_combo,"changed",G_CALLBACK(gain_cb),NULL);

  npe_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(npe_title), "<b>NR2 NPE Method</b>");
  gtk_widget_show(npe_title);
  gtk_grid_attach(GTK_GRID(grid),npe_title,2,4,1,1);

  npe_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(npe_combo),NULL,"OSMS");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(npe_combo),NULL,"MMSE");
  gtk_combo_box_set_active(GTK_COMBO_BOX(npe_combo),active_receiver->nr2_npe_method);
  my_combo_attach(GTK_GRID(grid), npe_combo, 3, 4, 1, 1);
  g_signal_connect(npe_combo,"changed",G_CALLBACK(npe_cb),NULL);

  pos_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(pos_title), "<b>NR/NR2/ANF Position</b>");
  gtk_widget_show(pos_title);
  gtk_grid_attach(GTK_GRID(grid),pos_title,0,5,1,1);

  pos_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(pos_combo),NULL,"Pre AGC");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(pos_combo),NULL,"Post AGC");
  gtk_combo_box_set_active(GTK_COMBO_BOX(pos_combo),active_receiver->nr_agc);
  my_combo_attach(GTK_GRID(grid), pos_combo, 1, 5, 1, 1);
  g_signal_connect(pos_combo,"changed",G_CALLBACK(pos_cb),NULL);

  b_ae=gtk_check_button_new_with_label("NR2 Artifact Elimination");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_ae), active_receiver->nr2_ae);
  gtk_widget_show(b_ae);
  gtk_grid_attach(GTK_GRID(grid),b_ae,2,5,2,1);
  g_signal_connect(b_ae,"toggled",G_CALLBACK(ae_cb),NULL);


  //
  // NB controls starting on row 4
  //
  mode_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(mode_title), "<b>NB2 mode</b>");
  gtk_grid_attach(GTK_GRID(grid),mode_title,0,4,1,1);

  mode_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo),NULL,"Zero");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo),NULL,"Sample&Hold");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo),NULL,"Mean Hold");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo),NULL,"Hold Sample");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo),NULL,"Interpolate");
  gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo),active_receiver->nb2_mode);
  my_combo_attach(GTK_GRID(grid), mode_combo, 1, 4, 1, 1);
  g_signal_connect(mode_combo,"changed",G_CALLBACK(mode_cb),NULL);

  slew_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(slew_title), "<b>NB Slew time (ms)</b>");
  gtk_grid_attach(GTK_GRID(grid),slew_title,0,5,1,1);

  slew_b=gtk_spin_button_new_with_range(0.0, 0.1, 0.0001);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(slew_b),active_receiver->nb_tau*1000.0);
  gtk_grid_attach(GTK_GRID(grid),slew_b,1,5,1,1);
  g_signal_connect(slew_b,"changed",G_CALLBACK(slew_cb),NULL);

  lead_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(lead_title), "<b>NB Lead time (ms)</b>");
  gtk_grid_attach(GTK_GRID(grid),lead_title,2,5,1,1);

  lead_b=gtk_spin_button_new_with_range(0.0, 0.1, 0.0001);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(lead_b),active_receiver->nb_advtime*1000.0);
  gtk_grid_attach(GTK_GRID(grid),lead_b,3,5,1,1);
  g_signal_connect(lead_b,"changed",G_CALLBACK(lead_cb),NULL);

  lag_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(lag_title), "<b>NB Lag time (ms)</b>");
  gtk_grid_attach(GTK_GRID(grid),lag_title,0,6,1,1);

  lag_b=gtk_spin_button_new_with_range(0.0, 0.1, 0.0001);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(lag_b),active_receiver->nb_hang*1000.0);
  gtk_grid_attach(GTK_GRID(grid),lag_b,1,6,1,1);
  g_signal_connect(lag_b,"changed",G_CALLBACK(lag_cb),NULL);

  thresh_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(thresh_title), "<b>NB Threshold</b>");
  gtk_grid_attach(GTK_GRID(grid),thresh_title,2,6,1,1);

  thresh_b=gtk_spin_button_new_with_range(15.0, 500.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(thresh_b),active_receiver->nb_thresh*6.0606060606);  // 1.0/0.165
  gtk_grid_attach(GTK_GRID(grid),thresh_b,3,6,1,1);
  g_signal_connect(thresh_b,"changed",G_CALLBACK(thresh_cb),NULL);

#ifdef EXTNR
  //
  // NR4 controls starting at row 4
  //
  nr4_reduction_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr4_reduction_title), "<b>NR4 Reduction Amount (dB)</b>");
  gtk_grid_attach(GTK_GRID(grid),nr4_reduction_title,0,4,1,1);

  nr4_reduction_b=gtk_spin_button_new_with_range(0.0, 20.0, 1.0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(nr4_reduction_b),active_receiver->nr4_reduction_amount);
  gtk_grid_attach(GTK_GRID(grid),nr4_reduction_b,1,4,1,1);
  g_signal_connect(G_OBJECT(nr4_reduction_b),"changed",G_CALLBACK(nr4_reduction_cb),NULL);

  nr4_smoothing_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr4_smoothing_title), "<b>NR4 Smoothing factor (%)</b>");
  gtk_grid_attach(GTK_GRID(grid),nr4_smoothing_title,2,4,1,1);

  nr4_smoothing_b=gtk_spin_button_new_with_range(0.0, 100.0, 1.0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(nr4_smoothing_b),active_receiver->nr4_smoothing_factor);
  gtk_grid_attach(GTK_GRID(grid),nr4_smoothing_b,3,4,1,1);
  g_signal_connect(G_OBJECT(nr4_smoothing_b),"changed",G_CALLBACK(nr4_smoothing_cb),NULL);

  nr4_whitening_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr4_whitening_title), "<b>NR4 Whitening factor (%)</b>");
  gtk_grid_attach(GTK_GRID(grid),nr4_whitening_title,0,5,1,1);

  nr4_whitening_b=gtk_spin_button_new_with_range(0.0, 100.0, 1.0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(nr4_whitening_b),active_receiver->nr4_whitening_factor);
  gtk_grid_attach(GTK_GRID(grid),nr4_whitening_b,1,5,1,1);
  g_signal_connect(G_OBJECT(nr4_whitening_b),"changed",G_CALLBACK(nr4_whitening_cb),NULL);

  nr4_rescale_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr4_rescale_title), "<b>NR4 noise rescale (dB)</b>");
  gtk_grid_attach(GTK_GRID(grid),nr4_rescale_title,2,5,1,1);

  nr4_rescale_b=gtk_spin_button_new_with_range(0.0, 12.0, 0.1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(nr4_rescale_b),active_receiver->nr4_noise_rescale);
  gtk_grid_attach(GTK_GRID(grid),nr4_rescale_b,3,5,1,1);
  g_signal_connect(G_OBJECT(nr4_rescale_b),"changed",G_CALLBACK(nr4_rescale_cb),NULL);

  nr4_threshold_title=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr4_threshold_title), "<b>NR4 post filter threshold (dB)</b>");
  gtk_grid_attach(GTK_GRID(grid),nr4_threshold_title,0,6,1,1);

  nr4_threshold_b=gtk_spin_button_new_with_range(-10.0, 10.0, 0.1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(nr4_threshold_b),active_receiver->nr4_post_filter_threshold);
  gtk_grid_attach(GTK_GRID(grid),nr4_threshold_b,1,6,1,1);
  g_signal_connect(G_OBJECT(nr4_threshold_b),"changed",G_CALLBACK(nr4_threshold_cb),NULL);
#endif

  //
  // the dummy takes care that the grid of the menu window does
  // not change when switching between the NR/NB/NR4 settings
  //
  GtkWidget *dummy=gtk_label_new(NULL);
  gtk_grid_attach(GTK_GRID(grid),dummy,0,4,1,1);
  dummy=gtk_label_new(NULL);
  gtk_grid_attach(GTK_GRID(grid),dummy,0,5,1,1);
  dummy=gtk_label_new(NULL);
  gtk_grid_attach(GTK_GRID(grid),dummy,0,6,1,1);

  gtk_container_add(GTK_CONTAINER(content),grid);

  sub_menu=dialog;

  gtk_widget_show_all(dialog);
  gtk_widget_hide(mode_title);
  gtk_widget_hide(mode_combo);
  gtk_widget_hide(slew_title);
  gtk_widget_hide(slew_b);
  gtk_widget_hide(lead_title);
  gtk_widget_hide(lead_b);
  gtk_widget_hide(lag_title);
  gtk_widget_hide(lag_b);
  gtk_widget_hide(thresh_title);
  gtk_widget_hide(thresh_b);

#ifdef EXTNR
  gtk_widget_hide(nr4_reduction_title);
  gtk_widget_hide(nr4_reduction_b);
  gtk_widget_hide(nr4_smoothing_title);
  gtk_widget_hide(nr4_smoothing_b);
  gtk_widget_hide(nr4_whitening_title);
  gtk_widget_hide(nr4_whitening_b);
  gtk_widget_hide(nr4_rescale_title);
  gtk_widget_hide(nr4_rescale_b);
  gtk_widget_hide(nr4_threshold_title);
  gtk_widget_hide(nr4_threshold_b);
#endif

}
