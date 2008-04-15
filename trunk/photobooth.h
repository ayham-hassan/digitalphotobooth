#ifndef PHOTOBOOTH_H_
#define PHOTOBOOTH_H_

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE "photobooth.xml"

#include <gtk/gtk.h>
#include "proj-nprosser/frame.h"
#include "proj-nprosser/drv-v4l2.h"

typedef struct
{
    GtkWidget *window;
    GtkWidget *wizard_panel;
    GtkWidget *take_photo_progress;
    GtkWidget *money_message_label;
    GtkWidget *money_forward_button;
    GtkWidget *videobox;
    GtkWidget *videobox1;
    GtkWidget *videobox2;
    GtkWidget *image;
    VidFrame *frame;
    GdkPixmap *pixmap;
    V4L2Capture *capture;
    gint timer_left;
    gint timer_total;
    gint timer_id;
    gint stream_time_id;
    gint money_inserted;
    gint money_total;
    gchar money_str[255];
} DigitalPhotoBooth;

/* window callback prototypes */
void on_window_destroy (GtkObject *object, DigitalPhotoBooth *booth);

/* button callback prototypes */
void on_money_forward_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth);
void on_take_photo_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth);

/* keyboard event callback prototypes */
gboolean on_window_key_press_event (GtkWidget *window, GdkEventKey *event, DigitalPhotoBooth *booth);

/* countdown timer prototypes */
void timer_start (DigitalPhotoBooth *booth);
gboolean timer_process (DigitalPhotoBooth *booth);

gboolean camera_process (DigitalPhotoBooth *booth);

/* money handling functions */
void money_insert (DigitalPhotoBooth *booth);
void money_update (DigitalPhotoBooth *booth);

/* misc. function prototypes */
void error_message (const gchar *message);
gboolean init_app (DigitalPhotoBooth *booth);

#endif
