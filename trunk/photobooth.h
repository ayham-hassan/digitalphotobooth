/*
 * photobooth.h
 * 
 * 	 @authors -	Kyle J. Farnung - kjf8400@rit.edu
 */

#ifndef PHOTOBOOTH_H_
#define PHOTOBOOTH_H_

#include <gtk/gtk.h>
#include "camera/frame.h"
#include "camera/drv-v4l2.h"
#include "ImageManipulations.h"

#ifndef PREFIX
/* installer prefix value */
#define PREFIX "/usr/local/"
#endif

/* location of the data directory */
#define DATA_DIR PREFIX "share/photobooth/"

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE DATA_DIR "photobooth.xml"

/* location of the texture file */
#define TEXTURE_FILE DATA_DIR "texture_fabric.gif"

#define TIMER_PHOTO_SECONDS 3
#define NUM_PHOTOS 3
#define NUM_PHOTO_STYLES 4
#define NUM_PHOTO_SIZES 3
#define MAX_STRING_LENGTH 256

enum PHOTO_STYLE
{
    NONE,
    OILBLOB,
    CHARCOAL,
    TEXTURE
};

enum PHOTO_SIZE
{
    FULL,
    SMALL,
    LARGE
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
    gint usb_cost;
    gint print_cost;
    
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
    
    /* third panel - photo selection */
    GtkWidget *preview_thumb1_image;
    GtkWidget *preview_thumb2_image;
    GtkWidget *preview_thumb3_image;
    GtkWidget *preview_large_image;
    guint selected_image_index;
    
    /* fourth panel - photo selection */
    GtkWidget *effects_thumb1_image;
    GtkWidget *effects_thumb2_image;
    GtkWidget *effects_thumb3_image;
    GtkWidget *effects_large_image;
    GtkWidget *effects_thumb1_button;
    GtkWidget *effects_thumb2_button;
    GtkWidget *effects_thumb3_button;
    enum PHOTO_STYLE selected_effect_enum;
    
    /* fifth panel - delivery selection */
    GtkWidget *delivery_usb_toggle;
    GtkWidget *delivery_print_toggle;
    GtkWidget *delivery_total_label;
    GtkWidget *delivery_large_image;
    GtkWidget *delivery_forward_button;
    gint delivery_total_cost;
    gboolean delivery_usb;
    gboolean delivery_print;
    
    /* sixth panel - finish */
    GtkWidget *finish_usb_frame;
    GtkWidget *finish_print_frame;
    GtkWidget *finish_usb_progress;
    GtkWidget *finish_large_image;
    
    /* filename variables */
    const gchar *tempdir;
    gchar photos_filenames[NUM_PHOTOS * NUM_PHOTO_STYLES * NUM_PHOTO_SIZES][MAX_STRING_LENGTH];
} DigitalPhotoBooth;


/******************************************************************************
 *
 *  Function:       error_message
 *  Description:    This function will display an error message dialog box
 *  Inputs:         message - the message string to display
 *  Outputs:        
 *  Routines Called: g_warning, gtk_message_dialog_new, gtk_window_set_title
 *                  gtk_dialog_run, gtk_widget_destroy
 *
 *****************************************************************************/
void error_message (const gchar *message);

/******************************************************************************
 *
 *  Function:       init_app
 *  Description:    This function initializes the application variables
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE on success, FALSE if an error occurs.
 *  Routines Called: gtk_builder_new, gtk_builder_add_from_file, error_message
 *                  g_error_free, gtk_builder_get_object, money_update, memset
 *                  gtk_builder_connect_signals, g_object_unref
 *
 *****************************************************************************/
gboolean init_app (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_window_destroy
 *  Description:    Callback function for the window being destroyed (closed)
 *  Inputs:         object - a pointer to the window object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: g_source_remove, v42lCaptureStopStreaming, close_camera
 *                  gtk_main_quit
 *
 *****************************************************************************/
void on_window_destroy (GtkObject *object, DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_window_key_press_event
 *  Description:    Callback function for window key press events
 *  Inputs:         windows - a pointer to the window object
 *                  event - a pointer to the key press event object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE if event handled, FALSE if event should propagate.
 *  Routines Called: money_insert, money_update
 *
 *****************************************************************************/
gboolean on_window_key_press_event (GtkWidget *window, GdkEventKey *event,
    DigitalPhotoBooth *booth);


/* General utility functions */

/******************************************************************************
 *
 *  Function:       get_image_filename_pointer
 *  Description:    This function returns a pointer to the requested filename
 *                  string.
 *  Inputs:         index - the index of the image
 *                  pt - the image type requested
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        a pointer to the requested string
 *  Routines Called: 
 *
 *****************************************************************************/
gchar* get_image_filename_pointer (guint index, enum PHOTO_STYLE pstyle,
    enum PHOTO_SIZE psize, DigitalPhotoBooth *booth);


/* Functions for the first screen */

/******************************************************************************
 *
 *  Function:       money_insert
 *  Description:    This function increases the current money total.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: 
 *
 *****************************************************************************/
void money_insert (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       money_pay
 *  Description:    This function checks the amount of available money and
 *                  deducts the purchase.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: money_update
 *
 *****************************************************************************/
gboolean money_pay (gint payment, DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       money_update
 *  Description:    This function updates the money display.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_widget_set_sensitive, sprintf, gtk_label_set_text
 *
 *****************************************************************************/
void money_update (DigitalPhotoBooth *booth);
    
/******************************************************************************
 *
 *  Function:       money_next
 *  Description:    Function for moving to the next panel
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_next_page, open_camera, g_idle_add
 *                  v42lCaptureStartStreaming
 *
 *****************************************************************************/
void money_next (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_money_forward_button_clicked
 *  Description:    Callback function for the money_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_next_page, open_camera, g_idle_add
 *                  v42lCaptureStartStreaming
 *
 *****************************************************************************/
void on_money_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);
    

/* Functions for the second screen */

/******************************************************************************
 *
 *  Function:       take_photo_init
 *  Description:    Initialize the second screen to take the photos
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: open_camera, g_idle_add, v42lCaptureStartStreaming
 *
 *****************************************************************************/
void take_photo_init (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       take_photo_free_frame
 *  Description:    Callback function for freeing frame data.
 *                  Only used when creating a pixbuf.
 *  Inputs:         pixels - a pointer to the data array
 *                  frame - a pointer to the frame struct
 *  Outputs:        
 *  Routines Called: vidFrameRelease
 *
 *****************************************************************************/
void take_photo_free_frame (guchar *pixels, VidFrame *frame);

/******************************************************************************
 *
 *  Function:       take_photo_live_feed_idle
 *  Description:    Callback function which gets a video frame, resizes it,
 *                  and displays it.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: getFrame, gdk_pixbuf_new_from_data, vidFrameGetImageData,
 *                  gdk_pixbuf_scale_simple, gdk_draw_pixbuf, g_object_unref
 *
 *****************************************************************************/
gboolean take_photo_live_feed_idle (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       take_photo_process
 *  Description:    Callback function which captures and process a photo from
 *                  the video stream.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: get_image_filename_pointer, sprintf, capture_hr_jpg,
 *                  image_resize, g_idle_add, timer_start, gtk_widget_hide,
 *                  gtk_widget_show
 *
 *****************************************************************************/
gboolean take_photo_process (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       take_photo_timer_start
 *  Description:    This function sets up and starts the countdown timer
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_progress_bar_set_fraction, sprintf,
 *                  gtk_progress_bar_set_text, g_timeout_add_seconds
 *
 *****************************************************************************/
void take_photo_timer_start (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       take_photo_timer_process
 *  Description:    Callback function which processes each timer tick
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: gtk_progress_bar_set_fraction, sprintf, g_source_remove
 *                  gtk_progress_bar_set_text, g_idle_add
 *
 *****************************************************************************/
gboolean take_photo_timer_process (DigitalPhotoBooth *booth);
    
/******************************************************************************
 *
 *  Function:       on_take_photo_button_clicked
 *  Description:    Callback function for the take_photo_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_widget_hide, gtk_widget_show, timer_start
 *
 *****************************************************************************/
void on_take_photo_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);


/* Functions for the third screen */

/******************************************************************************
 *
 *  Function:       preview_init
 *  Description:    Initialize the third screen to preview the photos
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf, preview_update_image
 *
 *****************************************************************************/
void preview_init (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       preview_update_image
 *  Description:    This function updates the larger preview image
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf
 *
 *****************************************************************************/
void preview_update_image (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_preview_forward_button_clicked
 *  Description:    Callback function for the preview_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_next_page, effects_init
 *
 *****************************************************************************/
void on_preview_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);
    
/******************************************************************************
 *
 *  Function:       on_preview_back_button_clicked
 *  Description:    Callback function for the preview_back_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_prev_page, take_photo_init
 *
 *****************************************************************************/
void on_preview_back_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_preview_thumb1_button_clicked
 *  Description:    Callback function for the preview_thumb1_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: preview_update_image
 *
 *****************************************************************************/
void on_preview_thumb1_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_preview_thumb2_button_clicked
 *  Description:    Callback function for the preview_thumb2_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: preview_update_image
 *
 *****************************************************************************/
void on_preview_thumb2_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_preview_thumb3_button_clicked
 *  Description:    Callback function for the preview_thumb3_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: preview_update_image
 *
 *****************************************************************************/
void on_preview_thumb3_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);


/* Functions for the fourth screen */

/******************************************************************************
 *
 *  Function:       effects_init
 *  Description:    Initialize the fourth screen to preview the effects
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_image_set_from_stock, gtk_widget_set_sensitive,
 *                  get_image_filename_pointer, g_sprintf,
 *                  create_oil_blob_image, g_child_watch_add,
 *                  create_charcoal_image, create_textured_image,
 *                  effects_update_image
 *
 *****************************************************************************/
void effects_init (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       effects_init_idle
 *  Description:    Callback function which processes creates the effects
 *                  images
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: get_image_filename_pointer, g_sprintf,
 *                  create_oil_blob_image, g_child_watch_add,
 *                  create_charcoal_image, create_textured_image
 *
 *****************************************************************************/
gboolean effects_init_idle (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       effects_oilblob_complete
 *  Description:    Callback function for the oilblob process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void effects_oilblob_complete (GPid pid, gint status, 
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       effects_effects_charcoal_complete
 *  Description:    Callback function for the charcoal process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void effects_charcoal_complete (GPid pid, gint status, 
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       effects_texture_complete
 *  Description:    Callback function for the texture process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void effects_texture_complete (GPid pid, gint status, 
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       effects_update_image
 *  Description:    This function updates the larger effects image
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf
 *
 *****************************************************************************/
void effects_update_image (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_effects_forward_button_clicked
 *  Description:    Callback function for the effects_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: delivery_init, gtk_notebook_next_page
 *
 *****************************************************************************/
void on_effects_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_effects_back_button_clicked
 *  Description:    Callback function for the effects_back_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: preview_init, gtk_notebook_next_page
 *
 *****************************************************************************/
void on_effects_back_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_effects_thumb1_button_clicked
 *  Description:    Callback function for the effects_thumb1_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: effects_update_image
 *
 *****************************************************************************/
void on_effects_thumb1_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_effects_thumb2_button_clicked
 *  Description:    Callback function for the effects_thumb2_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: effects_update_image
 *
 *****************************************************************************/
void on_effects_thumb2_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_effects_thumb3_button_clicked
 *  Description:    Callback function for the effects_thumb3_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: effects_update_image
 *
 *****************************************************************************/
void on_effects_thumb3_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);
    
/******************************************************************************
 *
 *  Function:       on_effects_none_button_clicked
 *  Description:    Callback function for the effects_none_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: effects_update_image
 *
 *****************************************************************************/
void on_effects_none_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);


/* Functions for the fifth screen */

/******************************************************************************
 *
 *  Function:       delivery_init
 *  Description:    Label the buttons with the current prices
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_button_set_label, sprintf
 *
 *****************************************************************************/
void delivery_init (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       delivery_required
 *  Description:    Calculate the required money
 *  Inputs:         price - the price to check
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        returns the amount of additional money required
 *  Routines Called: 
 *
 *****************************************************************************/
gint delivery_required (gint price, DigitalPhotoBooth *booth);
    
/******************************************************************************
 *
 *  Function:       delivery_update
 *  Description:    Recalculate the costs of delivery
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_toggle_button_get_active, sprintf, strchr,
 *                  gtk_widget_set_sensitive, gtk_label_set_text
 *
 *****************************************************************************/
void delivery_update (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_delivery_forward_button_clicked
 *  Description:    Callback function for the delivery_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: 
 *
 *****************************************************************************/
void on_delivery_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_delivery_back_button_clicked
 *  Description:    Callback function for the delivery_back_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: effects_init, gtk_notebook_prev_page
 *
 *****************************************************************************/
 void on_delivery_back_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);
    
/******************************************************************************
 *
 *  Function:       on_delivery_usb_toggle_toggled
 *  Description:    Callback function for the delivery_usb_toggle
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: 
 *
 *****************************************************************************/
void on_delivery_usb_toggle_toggled (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_delivery_print_toggle_toggled
 *  Description:    Callback function for the delivery_print_toggle
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: 
 *
 *****************************************************************************/
void on_delivery_print_toggle_toggled (GtkWidget *button,
    DigitalPhotoBooth *booth);


/* Functions for the sixth screen */

/******************************************************************************
 *
 *  Function:       finish_init
 *  Description:    Initialize the finish screen
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf
 *
 *****************************************************************************/
void finish_init (DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_finish_home_button_clicked
 *  Description:    Callback function for the finish_home_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_set_current_page
 *
 *****************************************************************************/
void on_finish_home_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

/******************************************************************************
 *
 *  Function:       on_finish_back_button_clicked
 *  Description:    Callback function for the finish_back_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_set_current_page
 *
 *****************************************************************************/
void on_finish_back_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth);

#endif
