/*
 * photobooth.c
 * 
 * 	 @authors -	Kyle J. Farnung - kjf8400@rit.edu
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include "camera/frame.h"
#include "camera/cam.h"
#include "usb-drive.h"
#include "ImageManipulations.h"
#include "FileHandler.h"
#include "photobooth.h"

/******************************************************************************
 *
 *  Function:       main
 *  Description:    This function sets up the program and begins execution
 *  Inputs:         argc - the number of arguments received
 *                  argv - the arguments received
 *  Outputs:        0 on exit success, Not 0 if an error occurs.
 *  Routines Called: g_slice_new, gtk_init, init_app, gtk_widget_show,
 *                  gtk_main, g_slice_free
 *
 *****************************************************************************/
int main (int argc, char *argv[])
{
    /* create the data structure */
    DigitalPhotoBooth *booth;
    
    /* allocate the memory needed by our DigitalPhotoBooth struct */
    booth = g_slice_new (DigitalPhotoBooth);

    /* initialize GTK+ libraries */
    gtk_init (&argc, &argv);
    
    /* initialize the app and exit on error */
    if (init_app (booth) == FALSE) return 1; /* error loading UI */

    /* show the window */
    gtk_widget_show (booth->window);
    
    /* make the window fullscreen */
    gtk_window_fullscreen ((GtkWindow*)booth->window);                
    
    /* enter GTK+ main loop */
    gtk_main ();
    
    /* free memory we allocated for DigitalPhotoBooth struct */
    g_slice_free (DigitalPhotoBooth, booth);

    /* signal a clean exit */
    return 0;
}

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
void error_message (const gchar *message)
{
    GtkWidget               *dialog;
    
    /* log to terminal window */
    g_warning (message);
    
    /* create an error message dialog and display modally to the user */
    dialog = gtk_message_dialog_new (NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK, message);
    
    gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
    gtk_dialog_run (GTK_DIALOG (dialog));      
    gtk_widget_destroy (dialog);         
}

/******************************************************************************
 *
 *  Function:       init_app
 *  Description:    This function initializes the application variables
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE on success, FALSE if an error occurs.
 *  Routines Called: gtk_builder_new, gtk_builder_add_from_file, error_message
 *                  g_error_free, gtk_builder_get_object, money_update, memset
 *                  gtk_builder_connect_signals, g_object_unref,
 *                  delivery_update
 *
 *****************************************************************************/
gboolean init_app (DigitalPhotoBooth *booth)
{
    GtkBuilder *builder;
    GError *err = NULL;
    
    /* use GtkBuilder to build our interface from the XML file */
    builder = gtk_builder_new ();
    if (gtk_builder_add_from_file (builder, BUILDER_XML_FILE, &err) == 0)
    {
            error_message (err->message);
            g_error_free (err);
            return FALSE;
    }
    
    /* get the widgets which will be referenced in callbacks */
    booth->window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
    booth->wizard_panel =
        GTK_WIDGET (gtk_builder_get_object (builder, "wizard_panel"));
    booth->money_message_label =
        GTK_WIDGET (gtk_builder_get_object (builder, "money_message_label"));
    booth->money_forward_button =
        GTK_WIDGET (gtk_builder_get_object (builder, "money_forward_button"));
    booth->take_photo_button =
        GTK_WIDGET (gtk_builder_get_object (builder, "take_photo_button"));
    booth->take_photo_progress =
        GTK_WIDGET (gtk_builder_get_object (builder, "take_photo_progress"));
    booth->videobox =
        GTK_WIDGET (gtk_builder_get_object (builder, "videobox"));
    booth->preview_thumb1_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "preview_thumb1_image"));
    booth->preview_thumb2_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "preview_thumb2_image"));
    booth->preview_thumb3_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "preview_thumb3_image"));
    booth->preview_large_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "preview_large_image"));    
    booth->effects_thumb1_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_thumb1_image"));
    booth->effects_thumb2_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_thumb2_image"));
    booth->effects_thumb3_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_thumb3_image"));
    booth->effects_large_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_large_image"));
    booth->effects_thumb1_button =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_thumb1_button"));
    booth->effects_thumb2_button =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_thumb2_button"));
    booth->effects_thumb3_button =
        GTK_WIDGET (gtk_builder_get_object (builder, "effects_thumb3_button"));
    booth->delivery_large_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "delivery_large_image"));
    booth->delivery_usb_toggle =
        GTK_WIDGET (gtk_builder_get_object (builder, "delivery_usb_toggle"));
    booth->delivery_print_toggle =
        GTK_WIDGET (gtk_builder_get_object (builder, "delivery_print_toggle"));
    booth->delivery_total_label =
        GTK_WIDGET (gtk_builder_get_object (builder, "delivery_total_label"));
    booth->delivery_forward_button =
        GTK_WIDGET (gtk_builder_get_object (builder,
        "delivery_forward_button"));
    booth->finish_usb_frame =
        GTK_WIDGET (gtk_builder_get_object (builder, "finish_usb_frame"));
    booth->finish_usb_progress =
        GTK_WIDGET (gtk_builder_get_object (builder, "finish_usb_progress"));
    booth->finish_print_frame =
        GTK_WIDGET (gtk_builder_get_object (builder, "finish_print_frame"));
    booth->finish_large_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "finish_large_image"));
    
    /* setup money handler variables */
    booth->usb_cost = 1;
    booth->print_cost = 2;
    booth->money_inserted = 0;
    booth->delivery_total_cost = 0;
    
    /* update money-related display features */
    money_update (booth);
    delivery_update (booth);
    
    /* reset the number of photos taken */
    booth->num_photos_taken = 0;
    
    /* set the streaming video pointers to NULL */
    booth->capture = NULL;
    
    /* set the source id fields to zero */
    booth->take_photo_video_source = 0;
	booth->take_photo_timer_source = 0;
	
	/* initialize the user image options */
	booth->selected_image_index = 0;
	booth->selected_effect_enum = NONE;
	
	/* clear the image filename storage area */
	memset (booth->photos_filenames, 0,
	    NUM_PHOTOS * NUM_PHOTO_STYLES * NUM_PHOTO_SIZES * MAX_STRING_LENGTH);
	    
    /* get the location of the system temp directory */
    booth->tempdir = g_get_tmp_dir ();
    
    /* connect signals, passing our DigitalPhotoBooth struct as user data */
    gtk_builder_connect_signals (builder, booth);
                
    /* free memory used by GtkBuilder object */
    g_object_unref (G_OBJECT (builder));
    
    /* indicate a successful init */
    return TRUE;
}

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
void on_window_destroy (GtkObject *object, DigitalPhotoBooth *booth)
{
    /* cleanup the camera */
    take_photo_cleanup (booth);

    /* make sure the camera was open and close it */
    if (booth->capture != NULL)
    {
        close_camera (booth->capture);
    }
    
    /* cleanup the application timeout */
    app_timeout_cleanup (booth);
    
    /* quit the main gtk loop */
    gtk_main_quit();
}

/******************************************************************************
 *
 *  Function:       on_window_key_press_event
 *  Description:    Callback function for window key press events
 *  Inputs:         windows - a pointer to the window object
 *                  event - a pointer to the key press event object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE if event handled, FALSE if event should propagate.
 *  Routines Called: money_insert
 *
 *****************************************************************************/
gboolean on_window_key_press_event (GtkWidget *window, GdkEventKey *event,
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);
    
    /* check if the 'f' key has been pressed (case-insensitive) */
    if (event->keyval == 102 || event->keyval == 70)
    {
        /* make the window fullscreen */
        gtk_window_fullscreen ((GtkWindow*)booth->window);
    }
    /* check if the 'g' key has been pressed (case-insensitive) */
    else if (event->keyval == 71 || event->keyval == 103 )
    {
        /* make the window un-fullscreen */
        gtk_window_unfullscreen ((GtkWindow*)booth->window);
    }
    /* check if the 'c' key has been pressed (case-insensitive) */
    else if (event->keyval == 67 || event->keyval == 99)
    {
        /* insert money and update the money display */
        money_insert (booth);
    }

    /* the event should propagate further */
    return FALSE;
}


/* functions for the app timeout */

/******************************************************************************
 *
 *  Function:       app_timeout_init
 *  Description:    Initialize the application timeout
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: g_idle_add
 *
 *****************************************************************************/
void app_timeout_init (DigitalPhotoBooth *booth)
{
    booth->app_timeout = 0;
    
    booth->app_timeout_source =
        g_timeout_add_seconds (1, (GSourceFunc)app_timeout_idle, booth);
}

/******************************************************************************
 *
 *  Function:       app_timeout_reset
 *  Description:    Reset the application timeout to zero (a button was pushed)
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: 
 *
 *****************************************************************************/
void app_timeout_reset (DigitalPhotoBooth *booth)
{
    booth->app_timeout = 0;
}

/******************************************************************************
 *
 *  Function:       app_timeout_cleanup
 *  Description:    Cleanup the application timeout
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: g_source_remove
 *
 *****************************************************************************/
void app_timeout_cleanup (DigitalPhotoBooth *booth)
{
    if (booth->app_timeout_source != 0) {
        g_source_remove (booth->app_timeout_source);
    }
}

/******************************************************************************
 *
 *  Function:       app_timeout_idle
 *  Description:    Process the application timeout
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: take_photo_cleanup, gtk_notebook_set_current_page
 *
 *****************************************************************************/
gboolean app_timeout_idle (DigitalPhotoBooth *booth)
{
    if (++booth->app_timeout < APP_TIMEOUT_SECONDS)
    {
        return TRUE;
    }
    else
    {
        booth->app_timeout = 0;
        
        take_photo_cleanup (booth);
        
        gtk_notebook_set_current_page ((GtkNotebook*)booth->wizard_panel, 0);
        
        return FALSE;
    }
}


/* General utility functions */

/******************************************************************************
 *
 *  Function:       get_image_filename_pointer
 *  Description:    This function returns a pointer to the requested filename
 *                  string.
 *  Inputs:         index - the index of the image
 *                  pt - the image type requested
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        a pointer to the requested string or NULL on failure
 *  Routines Called: 
 *
 *****************************************************************************/
gchar* get_image_filename_pointer (guint index, enum PHOTO_STYLE pstyle,
    enum PHOTO_SIZE psize, DigitalPhotoBooth *booth)
{
    /* make sure the parameters are valid */
    if (index < NUM_PHOTOS && pstyle < NUM_PHOTO_STYLES
        && psize < NUM_PHOTO_SIZES)
    {
        /* return a pointer to the image filename requested */
        return booth->photos_filenames[
            index * NUM_PHOTO_STYLES * NUM_PHOTO_SIZES
            + pstyle * NUM_PHOTO_SIZES + psize];
    }
    else
    {
        /* on failure return NULL */
        return NULL;
    }
}


/* Functions for the first screen */

/******************************************************************************
 *
 *  Function:       money_insert
 *  Description:    This function increases the current money total and
 *                  triggers an update.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: money_update
 *
 *****************************************************************************/
void money_insert (DigitalPhotoBooth *booth)
{
    /* increase the amount of money inserted into the machine */
    ++booth->money_inserted;
    
    /* update the first screen */
    money_update (booth);
    
    /* update the delivery screen */
    delivery_update (booth);
}

/******************************************************************************
 *
 *  Function:       money_pay
 *  Description:    This function checks the amount of available money and
 *                  deducts the purchase.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE if successful, FALSE otherwise
 *  Routines Called: money_update
 *
 *****************************************************************************/
gboolean money_pay (gint payment, DigitalPhotoBooth *booth)
{
    /* make sure the payment amount is available */
    if (booth->money_inserted >= payment)
    {
        /* decrement the payment from the money inserted */
        booth->money_inserted -= payment;
        
        /* update the first screen */
        money_update (booth);
        
        /* update the delivery screen */
        delivery_update (booth);
        
        /* return TRUE to signify payment success */
        return TRUE;
    }
    
    /* return FALSE to signify payment failure */
    return FALSE;
}

/******************************************************************************
 *
 *  Function:       money_update
 *  Description:    This function updates the money display.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_widget_set_sensitive, g_sprintf, gtk_label_set_text
 *
 *****************************************************************************/
void money_update (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* calculate the money still required */
    gint remaining = booth->usb_cost - booth->money_inserted;
    
    /* if there is enough, allow the user to proceed */
    if (remaining <= 0)
    {
        /* remaining cannot be less than 0, force it to zero */
        remaining = 0;
        
        /* make the forward button available */
        gtk_widget_set_sensitive (booth->money_forward_button, TRUE);
        
        if (gtk_notebook_get_current_page ((GtkNotebook*)booth->wizard_panel)
            == 0)            
        {
            money_next (booth);
        }
    }
    else
    {
        /* make the forward button unavailable */
        gtk_widget_set_sensitive (booth->money_forward_button, FALSE);
    }
    
    /* update the display text */
    g_sprintf (label, 
        "<big><b>%d Quarters Inserted\nPlease insert %d more quarters to continue\n\n",
        booth->money_inserted, remaining);
    
    /* concatenate the display text */
    g_sprintf (strchr (label, 0),
        "Delivery Costs:\nUSB Thumbdrive: %d Quarters\nPrinter: %d Quarters</b></big>",
        booth->usb_cost, booth->print_cost);

    /* update the textual display */
    gtk_label_set_markup ((GtkLabel*)booth->money_message_label, label);
}

/******************************************************************************
 *
 *  Function:       money_next
 *  Description:    Function for moving to the next panel
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_notebook_next_page, take_photo_init
 *
 *****************************************************************************/
void money_next (DigitalPhotoBooth *booth)
{
    /* initialize the application timeout */
    app_timeout_init (booth);

    /* initialize the take photo screen */
    take_photo_init (booth);
    
    /* switch to the next wizard page */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

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
    DigitalPhotoBooth *booth)
{
    money_next (booth);
}


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
void take_photo_init (DigitalPhotoBooth *booth)
{
    /* make sure camera is not already open and open if necessary */
    if (booth->capture == NULL)
    {
        booth->capture = open_camera();
    }
    
    /* start the video stream */
    if (booth->capture != NULL)
    {
        v4l2CaptureStartStreaming (booth->capture, 0, 4);
    }
    
    /* reset the number of photos taken this session to 0 */
    booth->num_photos_taken = 0;
	
	/* start a idle source which updates the drawing area */
    booth->take_photo_video_source = g_idle_add
        ((GSourceFunc)take_photo_live_feed_idle, booth);
}

/******************************************************************************
 *
 *  Function:       take_photo_clearnup
 *  Description:    Clean up the take photo screen
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: g_source_remove, v4l2CaptureStopStreaming
 *
 *****************************************************************************/
void take_photo_cleanup (DigitalPhotoBooth *booth)
{
    /* check if the video idle source still exists and remove if necessary */
    if (booth->take_photo_video_source != 0)
    {
        g_source_remove(booth->take_photo_video_source);
        booth->take_photo_video_source = 0;
    }

    /* check if the timer source still exists and remove if necessary */
    if (booth->take_photo_timer_source != 0)
    {
        g_source_remove(booth->take_photo_timer_source);
        booth->take_photo_timer_source = 0;
    }
    
    /* make sure the camera was open and stop streaming */
    if (booth->capture != NULL)
    {
        v4l2CaptureStopStreaming (booth->capture);
    }
}

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
void take_photo_free_frame (guchar *pixels, VidFrame *frame)
{
    /* release the received video frame */
    vidFrameRelease (&frame);
}

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
gboolean take_photo_live_feed_idle (DigitalPhotoBooth *booth)
{
    /* get the current RGB frame */
	VidFrame *frame = getFrame (booth->capture);
	
	/* put the frame in a pixel buffer */
	GdkPixbuf *buf = gdk_pixbuf_new_from_data (vidFrameGetImageData(frame),
        GDK_COLORSPACE_RGB, FALSE, 8, HR_WIDTH, HR_HEIGHT, HR_WIDTH * 3,
        (GdkPixbufDestroyNotify)take_photo_free_frame, frame);
	
	/* resize the original image using a very cheap algorithm */
	GdkPixbuf *resized = gdk_pixbuf_scale_simple (buf, LR_WIDTH, LR_HEIGHT,
        GDK_INTERP_NEAREST);
	
	/* draw the resized image on the screen */
	gdk_draw_pixbuf (booth->videobox->window, booth->videobox->style->white_gc,
        resized, 0, 0, 0, 0, LR_WIDTH, LR_HEIGHT, GDK_RGB_DITHER_NONE, 0, 0);
	
	/* remove a reference from the original buffer (should destroy it) */
	g_object_unref(buf);

	/* remove a reference from the resized buffer (should destroy it) */
	g_object_unref(resized);

    /* return true to cause the task to be constantly scheduled */
    return TRUE;
}

/******************************************************************************
 *
 *  Function:       take_photo_process
 *  Description:    Callback function which captures and process a photo from
 *                  the video stream.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: get_image_filename_pointer, g_sprintf, capture_hr_jpg,
 *                  image_resize, g_idle_add, take_photo_timer_start,
 *                  gtk_widget_hide, gtk_widget_show
 *
 *****************************************************************************/
gboolean take_photo_process (DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* make sure a capture object exists */
    if (booth->capture != NULL)
    {
        /* get a pointer to the image filenames */
        gchar *filename =
            get_image_filename_pointer (booth->num_photos_taken, NONE, FULL,
            booth);
        gchar *filename_sm =
            get_image_filename_pointer (booth->num_photos_taken, NONE, SMALL,
            booth);
        gchar *filename_lg =
            get_image_filename_pointer (booth->num_photos_taken, NONE, LARGE,
            booth);

        /* create the image filenames */
        g_sprintf (filename, "%s/img%04d.jpg", booth->tempdir, booth->num_photos_taken);
        g_sprintf (filename_sm, "%s/img%04d_sm.jpg", booth->tempdir, booth->num_photos_taken);
        g_sprintf (filename_lg, "%s/img%04d_lg.jpg", booth->tempdir, booth->num_photos_taken);
        
        /* capture a frame and convert it to jpg */
        capture_hr_jpg (booth->capture, filename, 85);
        
        /* spawn a process to resize the output images for display */
        image_resize (filename, filename_sm, "160x120", NULL);
        image_resize (filename, filename_lg, "640x480", NULL);
        
        /* pre-increment num_photos_taken */
        if (++booth->num_photos_taken < NUM_PHOTOS)
        {
            /* create an idle source which updates the drawing area */
            booth->take_photo_video_source = g_idle_add
                ((GSourceFunc)take_photo_live_feed_idle, booth);
            
            /* start another countdown timer */
            take_photo_timer_start(booth);
        }
        else
        {
            /* stop the video stream */
            if (booth->capture != NULL)
            {
                v4l2CaptureStopStreaming (booth->capture);
            }
            
            /* hide the progress bar, show the take photo button */
            gtk_widget_hide (booth->take_photo_progress);
            gtk_widget_show (booth->take_photo_button);
            
            /* initialize the next screen */
            preview_init (booth);

            /* switch to the next panel */
            gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
        }
    }

    /* return false to cause no further scheduling to occur */
    return FALSE;
}

/******************************************************************************
 *
 *  Function:       take_photo_timer_start
 *  Description:    This function sets up and starts the countdown timer
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_progress_bar_set_fraction, g_sprintf,
 *                  gtk_progress_bar_set_text, g_timeout_add_seconds
 *
 *****************************************************************************/
void take_photo_timer_start (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];

    /* initialize the timer fields */
    booth->take_photo_timer_left = TAKE_PHOTO_TIMER_SECONDS;
    
    /* set the initial state of the progress bar */
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress,
        1.0 );

    /* create the progress bar text */
    g_sprintf (label, "The photo will be taken in %d seconds",
        booth->take_photo_timer_left);
        
    /* set the progress bar text with the buffer content */
    gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
        label);
    
    /* add a timeout source which fires every second */
    booth->take_photo_timer_source = g_timeout_add_seconds (1,
        (GSourceFunc)take_photo_timer_process, booth);
}

/******************************************************************************
 *
 *  Function:       take_photo_timer_process
 *  Description:    Callback function which processes each timer tick
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: gtk_progress_bar_set_fraction, g_sprintf, g_source_remove
 *                  gtk_progress_bar_set_text, g_idle_add
 *
 *****************************************************************************/
gboolean take_photo_timer_process (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* set the progress bar to the current time left */
    double progress = --booth->take_photo_timer_left / (double)TAKE_PHOTO_TIMER_SECONDS;

    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress,
        progress );

    /* if the timer hasn't expired, continue the timeout schedule */
    if (booth->take_photo_timer_left > 0)
    {
        /* create the progress bar text */
        g_sprintf (label, "The photo will be taken in %d seconds",
            booth->take_photo_timer_left);
            
        /* set the progress bar text with the buffer content */
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
            label);
        
        /* return true to schedule the task again */
        return TRUE;
    }
    else
    {
        /* create the progress bar text */
        g_sprintf (label, "Please don't move, the photo is being captured");
            
        /* set the progress bar text with the buffer content */
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
            label);

        /* stop the video from updating */
        if (booth->take_photo_video_source != 0)
        {
            g_source_remove(booth->take_photo_video_source);
            booth->take_photo_video_source = 0;
        }
      
        /* add an idle function to take the photo (allow the GUI to update) */
        g_idle_add ((GSourceFunc)take_photo_process, booth);

        /* remove the source from the schedule */
        return FALSE;
    }
}

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
void on_take_photo_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);
    
    /* hide the take photo button and show the progress bar */
    gtk_widget_hide (booth->take_photo_button);
    gtk_widget_show (booth->take_photo_progress);
    
    /* start the countdown timer */
    take_photo_timer_start(booth);
}


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
void preview_init (DigitalPhotoBooth *booth)
{
    /* default the selected image index to 0 */
    booth->selected_image_index = 0;

    /* populate the first thumbnail with an image */
    gchar *thumb1_filename =
        get_image_filename_pointer (0, NONE, SMALL, booth);
    GdkPixbuf *thumb1_pixbuf =
        gdk_pixbuf_new_from_file (thumb1_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->preview_thumb1_image,
        thumb1_pixbuf);

    /* populate the second thumbnail with an image */
    gchar *thumb2_filename =
        get_image_filename_pointer (1, NONE, SMALL, booth);
    GdkPixbuf *thumb2_pixbuf =
        gdk_pixbuf_new_from_file (thumb2_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->preview_thumb2_image,
        thumb2_pixbuf);

    /* populate the third thumbnail with an image */
    gchar *thumb3_filename =
        get_image_filename_pointer (2, NONE, SMALL, booth);
    GdkPixbuf *thumb3_pixbuf =
        gdk_pixbuf_new_from_file (thumb3_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->preview_thumb3_image,
        thumb3_pixbuf);

    /* populate the default preview image */
    preview_update_image (booth);
}

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
void preview_update_image (DigitalPhotoBooth *booth)
{
    /* get a pointer to the currently selected images filename */
    gchar *preview_filename =
        get_image_filename_pointer (booth->selected_image_index, NONE, LARGE,
        booth);
    
    /* load the image into a pixel buffer */
    GdkPixbuf *preview_pixbuf =
        gdk_pixbuf_new_from_file (preview_filename, NULL);
    
    /* set the image to the pixbuf content */
    gtk_image_set_from_pixbuf ((GtkImage*)booth->preview_large_image,
        preview_pixbuf);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* initialize the fourth screen */
    effects_init (booth);

    /* move to the next screen */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* initialize the fourth screen */
    take_photo_init (booth);

    /* move to the next screen */
    gtk_notebook_prev_page ((GtkNotebook*)booth->wizard_panel);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* set the current index value to 0 */
    booth->selected_image_index = 0;
    
    /* update the image preview */
    preview_update_image (booth);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* set the current index value to 1 */
    booth->selected_image_index = 1;
    
    /* update the image preview */
    preview_update_image (booth);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);
    
    /* set the current index value to 2 */
    booth->selected_image_index = 2;
    
    /* update the image preview */
    preview_update_image (booth);
}


/* Functions for the fourth screen */

/******************************************************************************
 *
 *  Function:       effects_init
 *  Description:    Initialize the fourth screen to preview the effects
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_image_set_from_stock, gtk_widget_set_sensitive,
 *                  effects_update_image
 *
 *****************************************************************************/
void effects_init (DigitalPhotoBooth *booth)
{
    /* set the selected effect to NONE */
    booth->selected_effect_enum = NONE;
    
    /* reset the thumbnails to stock imagery */
    gtk_image_set_from_stock ((GtkImage*)booth->effects_thumb1_image,
        "gtk-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_image_set_from_stock ((GtkImage*)booth->effects_thumb2_image,
        "gtk-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_image_set_from_stock ((GtkImage*)booth->effects_thumb3_image,
        "gtk-refresh", GTK_ICON_SIZE_BUTTON);
    
    /* make the thumbnail buttons unavailable */
    gtk_widget_set_sensitive (booth->effects_thumb1_button, FALSE);
    gtk_widget_set_sensitive (booth->effects_thumb2_button, FALSE);
    gtk_widget_set_sensitive (booth->effects_thumb3_button, FALSE);

    /* add an idle function to spawn the child processes */
    g_idle_add ((GSourceFunc)effects_init_idle, booth);

    /* update the effects preview image */
    effects_update_image (booth);
}

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
gboolean effects_init_idle (DigitalPhotoBooth *booth)
{
    /* hold the pid value for the async calls */
    GPid pid;
    
    /* get a pointer to the original filename */
    gchar *filename = get_image_filename_pointer
        (booth->selected_image_index, NONE, FULL, booth);
    
    /* get a pointer to each of the OILBLOB image filenames */
    gchar *filename_ob = get_image_filename_pointer
        (booth->selected_image_index, OILBLOB, FULL, booth);
    gchar *filename_ob_sm = get_image_filename_pointer
        (booth->selected_image_index, OILBLOB, SMALL, booth);
    gchar *filename_ob_lg = get_image_filename_pointer
        (booth->selected_image_index, OILBLOB, LARGE, booth);
    
    /* create the OILBLOB image filenames */
    g_sprintf (filename_ob, "%s/img%04d_ob.jpg", booth->tempdir, 
        booth->selected_image_index);
    g_sprintf (filename_ob_sm, "%s/img%04d_ob_sm.jpg", booth->tempdir, 
        booth->selected_image_index);
    g_sprintf (filename_ob_lg, "%s/img%04d_ob_lg.jpg", booth->tempdir,
        booth->selected_image_index);

    /* spawn an async process and add a watch callback to it */
    create_oil_blob_image (filename, filename_ob, &pid, NULL);
    g_child_watch_add (pid, (GChildWatchFunc)effects_oilblob_complete, booth);
    
    /* get a pointer to each of the CHARCOAL image filenames */
    gchar *filename_ch = get_image_filename_pointer
        (booth->selected_image_index, CHARCOAL, FULL, booth);
    gchar *filename_ch_sm = get_image_filename_pointer
        (booth->selected_image_index, CHARCOAL, SMALL, booth);
    gchar *filename_ch_lg = get_image_filename_pointer
        (booth->selected_image_index, CHARCOAL, LARGE, booth);
    
    /* create the CHARCOAL image filenames */
    g_sprintf (filename_ch, "%s/img%04d_ch.jpg", booth->tempdir,
        booth->selected_image_index);
    g_sprintf (filename_ch_sm, "%s/img%04d_ch_sm.jpg", booth->tempdir,
        booth->selected_image_index);
    g_sprintf (filename_ch_lg, "%s/img%04d_ch_lg.jpg", booth->tempdir,
        booth->selected_image_index);
    
    /* spawn an async process and add a watch callback to it */
    create_charcoal_image (filename, filename_ch, &pid, NULL);
    g_child_watch_add (pid, (GChildWatchFunc)effects_charcoal_complete, booth);
    
    /* get a pointer to each of the TEXTURE image filenames */
    gchar *filename_tx = get_image_filename_pointer
        (booth->selected_image_index, TEXTURE, FULL, booth);
    gchar *filename_tx_sm = get_image_filename_pointer
        (booth->selected_image_index, TEXTURE, SMALL, booth);
    gchar *filename_tx_lg = get_image_filename_pointer
        (booth->selected_image_index, TEXTURE, LARGE, booth);
    
    /* create the TEXTURE image filenames */
    g_sprintf (filename_tx, "%s/img%04d_tx.jpg", booth->tempdir,
        booth->selected_image_index);
    g_sprintf (filename_tx_sm, "%s/img%04d_tx_sm.jpg", booth->tempdir,
        booth->selected_image_index);
    g_sprintf (filename_tx_lg, "%s/img%04d_tx_lg.jpg", booth->tempdir,
        booth->selected_image_index);
    
    /* spawn an async process and add a watch callback to it */
    create_textured_image (filename, TEXTURE_FILE, filename_tx, &pid, NULL);
    g_child_watch_add (pid, (GChildWatchFunc)effects_texture_complete, booth);
    
    return FALSE;
}

/******************************************************************************
 *
 *  Function:       effects_oilblob_complete
 *  Description:    Callback function for the oilblob process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void effects_oilblob_complete (GPid pid, gint status, DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    if (gtk_notebook_get_current_page ((GtkNotebook*)booth->wizard_panel) >= 3)
    {
        /* get a pointer to each of the image filenames */
        gchar *in = get_image_filename_pointer(booth->selected_image_index,
            OILBLOB, FULL, booth);
        gchar *small = get_image_filename_pointer(booth->selected_image_index,
            OILBLOB, SMALL, booth);
        gchar *large = get_image_filename_pointer(booth->selected_image_index,
            OILBLOB, LARGE, booth);
        
        /* resize the image */
        image_resize (in, small, "160x120", NULL);
        image_resize (in, large, "640x480", NULL);

        /* assign the image to a thumbnail */    
        GdkPixbuf *thumb1_pixbuf =
            gdk_pixbuf_new_from_file (small, NULL);
        gtk_image_set_from_pixbuf ((GtkImage*)booth->effects_thumb1_image,
            thumb1_pixbuf);

        /* make the image button available */
        gtk_widget_set_sensitive (booth->effects_thumb1_button, TRUE);
    }
}

/******************************************************************************
 *
 *  Function:       effects_charcoal_complete
 *  Description:    Callback function for the charcoal process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void effects_charcoal_complete (GPid pid, gint status, 
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    if (gtk_notebook_get_current_page ((GtkNotebook*)booth->wizard_panel) >= 3)
    {
        /* get a pointer to each of the image filenames */
        gchar *in = get_image_filename_pointer(booth->selected_image_index,
            CHARCOAL, FULL, booth);
        gchar *small = get_image_filename_pointer(booth->selected_image_index,
            CHARCOAL, SMALL, booth);
        gchar *large = get_image_filename_pointer(booth->selected_image_index,
            CHARCOAL, LARGE, booth);
        
        /* resize the image */
        image_resize (in, small, "160x120", NULL);
        image_resize (in, large, "640x480", NULL);
        
        /* assign the image to a thumbnail */
        GdkPixbuf *thumb2_pixbuf =
            gdk_pixbuf_new_from_file (small, NULL);
        gtk_image_set_from_pixbuf ((GtkImage*)booth->effects_thumb2_image,
            thumb2_pixbuf);

        /* make the image button available */
        gtk_widget_set_sensitive (booth->effects_thumb2_button, TRUE);
    }
}

/******************************************************************************
 *
 *  Function:       effects_texture_complete
 *  Description:    Callback function for the texture process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void effects_texture_complete (GPid pid, gint status, DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    if (gtk_notebook_get_current_page ((GtkNotebook*)booth->wizard_panel) >= 3)
    {
        /* get a pointer to each of the image filenames */
        gchar *in = get_image_filename_pointer(booth->selected_image_index,
            TEXTURE, FULL, booth);
        gchar *small = get_image_filename_pointer(booth->selected_image_index,
            TEXTURE, SMALL, booth);
        gchar *large = get_image_filename_pointer(booth->selected_image_index,
            TEXTURE, LARGE, booth);
        
        /* resize the image */
        image_resize (in, small, "160x120", NULL);
        image_resize (in, large, "640x480", NULL);
        
        /* assign the image to a thumbnail */
        GdkPixbuf *thumb3_pixbuf =
            gdk_pixbuf_new_from_file (small, NULL);
        gtk_image_set_from_pixbuf ((GtkImage*)booth->effects_thumb3_image,
            thumb3_pixbuf);
        
        /* make the image button available */
        gtk_widget_set_sensitive (booth->effects_thumb3_button, TRUE);
    }
}

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
void effects_update_image (DigitalPhotoBooth *booth)
{
    /* get a pointer to the desired image filename */
    gchar *effects_filename =
        get_image_filename_pointer (booth->selected_image_index,
        booth->selected_effect_enum, LARGE, booth);
    
    /* load the image into a pixel buffer */
    GdkPixbuf *effects_pixbuf =
        gdk_pixbuf_new_from_file (effects_filename, NULL);
        
    /* set the image to the pixbuf content */
    gtk_image_set_from_pixbuf ((GtkImage*)booth->effects_large_image,
        effects_pixbuf);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* initialize the delivery screen */
    delivery_init (booth);
    
    /* move to the next screen */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* initialize the preview screen */
    preview_init (booth);
    
    /* move to the next screen */
    gtk_notebook_prev_page ((GtkNotebook*)booth->wizard_panel);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* set the effect type to OILBLOB */
    booth->selected_effect_enum = OILBLOB;
    
    /* update the preview image */
    effects_update_image (booth);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* set the effect type to CHARCOAL */
    booth->selected_effect_enum = CHARCOAL;
    
    /* update the preview image */
    effects_update_image (booth);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* set the effect type to TEXTURE */
    booth->selected_effect_enum = TEXTURE;
    
    /* update the preview image */
    effects_update_image (booth);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    /* set the effect type to NONE */
    booth->selected_effect_enum = NONE;
    
    /* update the preview image */
    effects_update_image (booth);
}


/* Functions for the fifth screen */

/******************************************************************************
 *
 *  Function:       delivery_init
 *  Description:    Initialize the delivery screen
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_toggle_button_set_active, delivery_update,
 *                  get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf
 *
 *****************************************************************************/
void delivery_init (DigitalPhotoBooth *booth)
{
    /* initialize the delivery method booleans */
    booth->delivery_usb = FALSE;
    booth->delivery_print = FALSE;
    
    /* default the toggle buttons to have both unselected */
    gtk_toggle_button_set_active
        ((GtkToggleButton*)booth->delivery_usb_toggle, FALSE);
    gtk_toggle_button_set_active
        ((GtkToggleButton*)booth->delivery_print_toggle, FALSE);
    
    /* make sure the delivery screen is updated */
    delivery_update (booth);

    /* get a pointer to the filename of the delivery photo */
    gchar *delivery_filename =
        get_image_filename_pointer (booth->selected_image_index,
        booth->selected_effect_enum, LARGE, booth);
    
    /* load the photo into a pixel buffer */
    GdkPixbuf *delivery_pixbuf =
        gdk_pixbuf_new_from_file (delivery_filename, NULL);
    
    /* set the image to the pixbuf content */
    gtk_image_set_from_pixbuf ((GtkImage*)booth->delivery_large_image,
        delivery_pixbuf);
}

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
gint delivery_required (gint price, DigitalPhotoBooth *booth)
{
    gint required = price - booth->money_inserted;
    
    if (required < 0)
    {
        required = 0;
    }
    
    return required;
}

/******************************************************************************
 *
 *  Function:       delivery_update
 *  Description:    Recalculate the costs of delivery
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_toggle_button_get_active, g_sprintf, strchr,
 *                  gtk_widget_set_sensitive, gtk_label_set_text,
 *                  gtk_button_set_label
 *
 *****************************************************************************/
void delivery_update (DigitalPhotoBooth *booth)
{
    /* create and initialize variables */
    gint required = 0;
    booth->delivery_total_cost = 0;

    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* create the button label for usb delivery */
    g_sprintf (label, "Send to Thumb Drive\nCost: %d Additional Quarters",
        delivery_required (booth->usb_cost, booth));
    
    /* set the button label to the string buffer content */
    gtk_button_set_label ((GtkButton*)booth->delivery_usb_toggle, label);
    
    /* create the button label for print delivery */
    g_sprintf (label, "Send to Printer\nCost: %d Additional Quarters",
        delivery_required (booth->print_cost, booth));
    
    /* set the button label to the string buffer content */
    gtk_button_set_label ((GtkButton*)booth->delivery_print_toggle, label);
    
    /* if usb delivery, add the usb cost to the total */
    if (gtk_toggle_button_get_active 
        ((GtkToggleButton*)booth->delivery_usb_toggle))
    {
        booth->delivery_total_cost += booth->usb_cost;
    }
    
    /* if print delivery, add the print cost to the total */
    if (gtk_toggle_button_get_active 
        ((GtkToggleButton*)booth->delivery_print_toggle))
    {
        booth->delivery_total_cost += booth->print_cost;
    }
    
    /* if both, subtract the usb cost from the total */    
    if (gtk_toggle_button_get_active 
        ((GtkToggleButton*)booth->delivery_usb_toggle)
        && gtk_toggle_button_get_active
        ((GtkToggleButton*)booth->delivery_print_toggle))
    {
        booth->delivery_total_cost -= booth->usb_cost;
    }

    /* calculate how much more money is required */
    required = booth->delivery_total_cost - booth->money_inserted;
    
    /* start building the string with the information available */
    g_sprintf (label, "Inserted: %d Quarters\nTotal Cost: %d Quarters\n",
        booth->money_inserted, booth->delivery_total_cost);
    
    /* if sufficient funds */
    if (required <= 0)
    {
        /* required can't be negative, set it to zero */
        required = 0;
        
        /* concatenate the string */
        g_sprintf (strchr (label, 0), "Required: %d Quarters", required);
        
        /* if at least one toggle button is active, then we can continue */
        if (gtk_toggle_button_get_active 
            ((GtkToggleButton*)booth->delivery_usb_toggle)
            || gtk_toggle_button_get_active
            ((GtkToggleButton*)booth->delivery_print_toggle))
        {
            /* make the forward button available */
            gtk_widget_set_sensitive (booth->delivery_forward_button, TRUE);
        }
        else
        {
            /* make the forward button unavailable */
            gtk_widget_set_sensitive (booth->delivery_forward_button, FALSE);
        }
    }
    else
    {
        /* concatenate the string twice */
        g_sprintf (strchr(label, 0), "Required: %d Quarters\n\n", required);
        g_sprintf (strchr(label, 0), 
            "Please insert additional quarters to continue.");

        /* make the forward button unavailable */
        gtk_widget_set_sensitive (booth->delivery_forward_button, FALSE);
    }
    
    /* update the label text with the current information */
    gtk_label_set_text ((GtkLabel*)booth->delivery_total_label, label);
}

/******************************************************************************
 *
 *  Function:       on_delivery_forward_button_clicked
 *  Description:    Callback function for the delivery_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: money_pay, gtk_toggle_button_get_active,
 *                  gtk_toggle_button_set_active, gtk_notebook_next_page
 *
 *****************************************************************************/
void on_delivery_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    if (money_pay (booth->delivery_total_cost, booth))
    {
        gchar *filename = get_image_filename_pointer
            (booth->selected_image_index, booth->selected_effect_enum, FULL,
            booth);
        
        if (gtk_toggle_button_get_active 
            ((GtkToggleButton*)booth->delivery_usb_toggle))
        {
            booth->delivery_usb = TRUE;
            /* call usb code */
            printf ("USB DELIVERY!\n");
            writeFileToUSBDrive (filename);
            fs_sync (NULL);
        }
        
        if (gtk_toggle_button_get_active 
            ((GtkToggleButton*)booth->delivery_print_toggle))
        {
            booth->delivery_print = TRUE;
            printf ("PRINTER DELIVERY!\n");
            printImage (filename, NULL);
        }
        
        finish_init (booth);
        
        gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
    }
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);
    
    gtk_notebook_prev_page ((GtkNotebook*)booth->wizard_panel);
}

/******************************************************************************
 *
 *  Function:       on_delivery_usb_toggle_toggled
 *  Description:    Callback function for the delivery_usb_toggle
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: delivery_update
 *
 *****************************************************************************/
void on_delivery_usb_toggle_toggled (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);
    
    delivery_update (booth);
}

/******************************************************************************
 *
 *  Function:       on_delivery_print_toggle_toggled
 *  Description:    Callback function for the delivery_print_toggle
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: delivery_update
 *
 *****************************************************************************/
void on_delivery_print_toggle_toggled (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    delivery_update (booth);
}


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
void finish_init (DigitalPhotoBooth *booth)
{
    if (booth->delivery_usb)
    {
        gtk_widget_show (booth->finish_usb_frame);
        finish_timer_start (booth);
    }
    else
    {
        gtk_widget_hide (booth->finish_usb_frame);
    }
    
    if (booth->delivery_print)
    {
        gtk_widget_show (booth->finish_print_frame);
    }
    else
    {
        gtk_widget_hide (booth->finish_print_frame);
    }

    /* get a pointer to the filename of the delivery photo */
    gchar *filename =
        get_image_filename_pointer (booth->selected_image_index,
        booth->selected_effect_enum, LARGE, booth);
    
    /* load the photo into a pixel buffer */
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    
    /* set the image to the pixbuf content */
    gtk_image_set_from_pixbuf ((GtkImage*)booth->finish_large_image, pixbuf);
}

/******************************************************************************
 *
 *  Function:       finish_timer_start
 *  Description:    This function sets up and starts the usb transfer timer
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_progress_bar_set_fraction, g_timeout_add_seconds
 *
 *****************************************************************************/
void finish_timer_start (DigitalPhotoBooth *booth)
{
    /* initialize the timer fields */
    booth->finish_timer_left = FINISH_USB_TIMER_SECONDS;
    
    /* set the initial state of the progress bar */
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->finish_usb_progress,
        0.0 );
    
    /* add a timeout source which fires every second */
    g_timeout_add_seconds (1, (GSourceFunc)finish_timer_process, booth);
}

/******************************************************************************
 *
 *  Function:       finish_timer_process
 *  Description:    Callback function which processes each timer tick
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: gtk_progress_bar_set_fraction, g_sprintf
 *                  gtk_progress_bar_set_text
 *
 *****************************************************************************/
gboolean finish_timer_process (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* set the progress bar to the current time left */
    double progress =
        (FINISH_USB_TIMER_SECONDS - (--booth->finish_timer_left))
        / (double)FINISH_USB_TIMER_SECONDS;

    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->finish_usb_progress,
        progress );

    /* if the timer hasn't expired, continue the timeout schedule */
    if (booth->finish_timer_left > 0)
    {
        /* return true to schedule the task again */
        return TRUE;
    }
    else
    {
        /* create the progress bar text */
        g_sprintf (label, "Transfer complete.");
            
        /* set the progress bar text with the buffer content */
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->finish_usb_progress,
            label);

        /* remove the source from the schedule */
        return FALSE;
    }
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_cleanup (booth);

    gtk_notebook_set_current_page ((GtkNotebook*)booth->wizard_panel, 0);
}

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
    DigitalPhotoBooth *booth)
{
    /* reset the application timeout */
    app_timeout_reset (booth);

    gtk_notebook_set_current_page ((GtkNotebook*)booth->wizard_panel, 2);
}

