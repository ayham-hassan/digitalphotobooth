#ifndef PHOTOBOOTH_H_
#define PHOTOBOOTH_H_

#include <gtk/gtk.h>
#include "proj-nprosser/frame.h"
#include "proj-nprosser/drv-v4l2.h"
#include "ImageManipulations.h"

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE "photobooth.xml"

#define TIMER_PHOTO_SECONDS 3
#define NUM_PHOTOS 3
#define NUM_PHOTO_TYPES 12
#define MAX_STRING_LENGTH 256

enum PHOTO_TYPE
{
    FULL,
    SMALL,
    LARGE,
    OILBLOB_FULL,
    OILBLOB_SMALL,
    OILBLOB_LARGE,
    CHARCOAL_FULL,
    CHARCOAL_SMALL,
    CHARCOAL_LARGE,
    TEXTURE_FULL,
    TEXTURE_SMALL,
    TEXTURE_LARGE
};

typedef struct
{
    /* main window */
    GtkWidget *window;
    
    /* wizard panel */
    GtkWidget *wizard_panel;
    
    /* first panel - welcome and coin acceptance */
    GtkWidget *money_message_label;
    GtkWidget *money_forward_button;
    gint money_inserted;
    gint money_total;
    gchar money_str[MAX_STRING_LENGTH];
    
    /* second panel - streaming video */
    V4L2Capture *capture;
    guint video_source_id;
    GtkWidget *videobox;
    GtkWidget *take_photo_button;
    GtkWidget *take_photo_progress;
    GtkWidget *take_photo_forward_button;
    guint num_photos_taken;
    gint timer_left;
    gint timer_total;
    guint timer_source_id;
    gchar progress_bar_label[MAX_STRING_LENGTH];
    
    /* third panel - photo selection */
    GtkWidget *image1_preview_image;
    GtkWidget *image2_preview_image;
    GtkWidget *image3_preview_image;
    GtkWidget *image_preview_area;
    guint selected_image_index;
    
    /* filename variables */
    gchar photos_filenames[NUM_PHOTOS*NUM_PHOTO_TYPES][MAX_STRING_LENGTH];
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

/* camera update methods */
gboolean camera_process (DigitalPhotoBooth *booth);

/* money handling functions */
void money_insert (DigitalPhotoBooth *booth);
void money_update (DigitalPhotoBooth *booth);

/* misc. function prototypes */
void error_message (const gchar *message);
gboolean init_app (DigitalPhotoBooth *booth);

#endif
