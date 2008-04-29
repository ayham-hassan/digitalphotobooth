/*
 * photobooth.c
 * 
 * 	 @authors -	Kyle J. Farnung - kjf8400@rit.edu
 */

#include <gtk/gtk.h>
#include <string.h>
#include "proj-nprosser/frame.h"
#include "proj-nprosser/cam.h"
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
    
    /* setup money handler variables */
    booth->usb_cost = 1;
    booth->print_cost = 2;
    booth->money_inserted = 0;
    booth->delivery_total_cost = 0;
    
    /* update money-related display features */
    money_update (booth);
    delivery_init (booth);
    delivery_update (booth);
    
    /* reset the number of photos taken */
    booth->num_photos_taken = 0;
    
    /* set the streaming video pointers to NULL */
    booth->capture = NULL;
    
    /* set the source id fields to zero */
    booth->video_source_id = 0;
	booth->timer_source_id = 0;
	
	/* initialize the user image options */
	booth->selected_image_index = 0;
	booth->selected_effect_enum = NONE;
	
	/* clear the image filename storage area */
	memset (booth->photos_filenames, 0,
	    NUM_PHOTOS * NUM_PHOTO_STYLES * NUM_PHOTO_SIZES * MAX_STRING_LENGTH);
    
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
    /* check if the video idle source still exists and remove if necessary */
    if (booth->video_source_id != 0)
    {
        g_source_remove(booth->video_source_id);
        booth->video_source_id = 0;
    }

    /* check if the timer source still exists and remove if necessary */
    if (booth->timer_source_id != 0)
    {
        g_source_remove(booth->timer_source_id);
        booth->timer_source_id = 0;
    }
    
    /* make sure the camera was open and close if necessary */
    if (booth->capture != NULL)
    {
        v4l2CaptureStopStreaming (booth->capture);
        close_camera (booth->capture);
    }
    
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
 *  Routines Called: gtk_widget_set_sensitive, sprintf, gtk_label_set_text
 *
 *****************************************************************************/
void money_update (DigitalPhotoBooth *booth)
{
    /* calculate the money still required */
    gint remaining = booth->usb_cost - booth->money_inserted;
    
    /* if there is enough, allow the user to proceed */
    if (remaining <= 0)
    {
        /* remaining cannot be less than 0, force it to zero */
        remaining = 0;
        
        /* make the forward button available */
        gtk_widget_set_sensitive (booth->money_forward_button, TRUE);
    }
    else
    {
        /* make the forward button unavailable */
        gtk_widget_set_sensitive (booth->money_forward_button, FALSE);
    }
    
    /* update the display text */
    sprintf (booth->money_str, 
        "%d Quarters Inserted\nPlease insert %d more quarters to continue",
        booth->money_inserted, remaining);

    /* update the textual display */
    gtk_label_set_text ((GtkLabel*)booth->money_message_label,
        booth->money_str);
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
    booth->video_source_id = g_idle_add ((GSourceFunc)camera_process, booth);
    
    /* switch to the next wizard page */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}


/* Functions for the second screen */

/******************************************************************************
 *
 *  Function:       free_frame
 *  Description:    Callback function for freeing frame data.
 *                  Only used when creating a pixbuf.
 *  Inputs:         pixels - a pointer to the data array
 *                  frame - a pointer to the frame struct
 *  Outputs:        
 *  Routines Called: vidFrameRelease
 *
 *****************************************************************************/
void free_frame (guchar *pixels, VidFrame *frame)
{
    /* release the received video frame */
    vidFrameRelease (&frame);
}

/******************************************************************************
 *
 *  Function:       camera_process
 *  Description:    Callback function which gets a video frame, resizes it,
 *                  and displays it.
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: getFrame, gdk_pixbuf_new_from_data, vidFrameGetImageData,
 *                  gdk_pixbuf_scale_simple, gdk_draw_pixbuf, g_object_unref
 *
 *****************************************************************************/
gboolean camera_process (DigitalPhotoBooth *booth)
{
    /* get the current RGB frame */
	VidFrame *frame = getFrame (booth->capture);
	
	/* put the frame in a pixel buffer */
	GdkPixbuf *buf = gdk_pixbuf_new_from_data (vidFrameGetImageData(frame),
        GDK_COLORSPACE_RGB, FALSE, 8, HR_WIDTH, HR_HEIGHT, HR_WIDTH * 3,
        (GdkPixbufDestroyNotify)free_frame, frame);
	
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
 *  Routines Called: get_image_filename_pointer, sprintf, capture_hr_jpg,
 *                  image_resize, g_idle_add, timer_start, gtk_widget_hide,
 *                  gtk_widget_show
 *
 *****************************************************************************/
gboolean take_photo_process (DigitalPhotoBooth *booth)
{
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
        sprintf (filename, "img%04d.jpg", booth->num_photos_taken);
        sprintf (filename_sm, "img%04d_sm.jpg", booth->num_photos_taken);
        sprintf (filename_lg, "img%04d_lg.jpg", booth->num_photos_taken);
        
        /* capture a frame and convert it to jpg */
        capture_hr_jpg (booth->capture, filename, 85);
        
        /* spawn a process to resize the output images for display */
        image_resize (filename, filename_sm, "160x120", NULL);
        image_resize (filename, filename_lg, "640x480", NULL);
        
        /* pre-increment num_photos_taken */
        if (++booth->num_photos_taken < NUM_PHOTOS)
        {
            /* create an idle source which updates the drawing area */
            booth->video_source_id = g_idle_add ((GSourceFunc)camera_process,
                booth);
            
            /* start another countdown timer */
            timer_start(booth);
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
            update_preview_image (booth);

            /* switch to the next panel */
            gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
        }
    }

    /* return false to cause no further scheduling to occur */
    return FALSE;
}

/******************************************************************************
 *
 *  Function:       timer_start
 *  Description:    This function sets up and starts the countdown timer
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_progress_bar_set_fraction, sprintf,
 *                  gtk_progress_bar_set_text, g_timeout_add_seconds
 *
 *****************************************************************************/
void timer_start (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];

    /* initialize the timer fields */
    booth->timer_left = TIMER_PHOTO_SECONDS;
    booth->timer_total = TIMER_PHOTO_SECONDS;
    
    /* set the initial state of the progress bar */
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress,
        1.0 );

    /* create the progress bar text */
    sprintf (label, "The photo will be taken in %d seconds",
        booth->timer_left);
        
    /* set the progress bar text with the buffer content */
    gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
        label);
    
    /* add a timeout source which fires every second */
    booth->timer_source_id = g_timeout_add_seconds (1,
        (GSourceFunc)timer_process, booth);
}

/******************************************************************************
 *
 *  Function:       timer_process
 *  Description:    Callback function which processes each timer tick
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        TRUE to schedule the task again, FALSE otherwise
 *  Routines Called: gtk_progress_bar_set_fraction, sprintf, g_source_remove
 *                  gtk_progress_bar_set_text, g_idle_add
 *
 *****************************************************************************/
gboolean timer_process (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* set the progress bar to the current time left */
    double progress = --booth->timer_left / (double)booth->timer_total;
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress,
        progress );

    /* if the timer hasn't expired, continue the timeout schedule */
    if (booth->timer_left > 0)
    {
        /* create the progress bar text */
        sprintf (label, "The photo will be taken in %d seconds",
            booth->timer_left);
            
        /* set the progress bar text with the buffer content */
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
            label);
        
        /* return true to schedule the task again */
        return TRUE;
    }
    else
    {
        /* create the progress bar text */
        sprintf (label, "Please don't move, the photo is being captured");
            
        /* set the progress bar text with the buffer content */
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
            label);

        /* stop the video from updating */
        if (booth->video_source_id != 0)
        {
            g_source_remove(booth->video_source_id);
            booth->video_source_id = 0;
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
    /* hide the take photo button and show the progress bar */
    gtk_widget_hide (booth->take_photo_button);
    gtk_widget_show (booth->take_photo_progress);
    
    /* start the countdown timer */
    timer_start(booth);
}


/* Functions for the third screen */

/******************************************************************************
 *
 *  Function:       update_preview_image
 *  Description:    This function updates the larger preview image
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf
 *
 *****************************************************************************/
void update_preview_image (DigitalPhotoBooth *booth)
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
 * *  Function:       on_photo_select_forward_button_clicked
 *  Description:    Callback function for the photo_select_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:         *  Routines Called: gtk_notebook_next_page
 *
 *****************************************************************************/
void on_photo_select_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* hold the pid value for the async calls */
    GPid pid;
    
    /* set the selected effect to NONE */
    booth->selected_effect_enum = NONE;

    /* get a pointer to the original filename */
    gchar *filename = get_image_filename_pointer(booth->selected_image_index, NONE, FULL, booth);
    
    /* get a pointer to each of the OILBLOB image filenames */
    gchar *filename_ob = get_image_filename_pointer(booth->selected_image_index, OILBLOB, FULL, booth);
    gchar *filename_ob_sm = get_image_filename_pointer(booth->selected_image_index, OILBLOB, SMALL, booth);
    gchar *filename_ob_lg = get_image_filename_pointer(booth->selected_image_index, OILBLOB, LARGE, booth);
    
    /* create the OILBLOB image filenames */
    sprintf (filename_ob, "img%04d_ob.jpg", booth->selected_image_index);
    sprintf (filename_ob_sm, "img%04d_ob_sm.jpg", booth->selected_image_index);
    sprintf (filename_ob_lg, "img%04d_ob_lg.jpg", booth->selected_image_index);

    /* spawn an async process and add a watch callback to it */
    create_oil_blob_image (filename, filename_ob, &pid, NULL);
    g_child_watch_add (pid, (GChildWatchFunc)oilblob_complete, booth);
    
    /* get a pointer to each of the CHARCOAL image filenames */
    gchar *filename_ch = get_image_filename_pointer(booth->selected_image_index, CHARCOAL, FULL, booth);
    gchar *filename_ch_sm = get_image_filename_pointer(booth->selected_image_index, CHARCOAL, SMALL, booth);
    gchar *filename_ch_lg = get_image_filename_pointer(booth->selected_image_index, CHARCOAL, LARGE, booth);
    
    /* create the CHARCOAL image filenames */
    sprintf (filename_ch, "img%04d_ch.jpg", booth->selected_image_index);
    sprintf (filename_ch_sm, "img%04d_ch_sm.jpg", booth->selected_image_index);
    sprintf (filename_ch_lg, "img%04d_ch_lg.jpg", booth->selected_image_index);
    
    /* spawn an async process and add a watch callback to it */
    create_charcoal_image (filename, filename_ch, &pid, NULL);
    g_child_watch_add (pid, (GChildWatchFunc)charcoal_complete, booth);
    
    /* get a pointer to each of the TEXTURE image filenames */
    gchar *filename_tx = get_image_filename_pointer(booth->selected_image_index, TEXTURE, FULL, booth);
    gchar *filename_tx_sm = get_image_filename_pointer(booth->selected_image_index, TEXTURE, SMALL, booth);
    gchar *filename_tx_lg = get_image_filename_pointer(booth->selected_image_index, TEXTURE, LARGE, booth);
    
    /* create the TEXTURE image filenames */
    sprintf (filename_tx, "img%04d_tx.jpg", booth->selected_image_index);
    sprintf (filename_tx_sm, "img%04d_tx_sm.jpg", booth->selected_image_index);
    sprintf (filename_tx_lg, "img%04d_tx_lg.jpg", booth->selected_image_index);
    
    /* spawn an async process and add a watch callback to it */
    create_textured_image (filename, "texture_fabric.gif", filename_tx, &pid, NULL);
    g_child_watch_add (pid, (GChildWatchFunc)texture_complete, booth);

    /* update the effects preview image */
    update_effects_image (booth);

    /* move to the next screen */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

/******************************************************************************
 *
 *  Function:       on_preview_thumb1_button_clicked
 *  Description:    Callback function for the preview_thumb1_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_preview_image
 *
 *****************************************************************************/
void on_preview_thumb1_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the current index value to 0 */
    booth->selected_image_index = 0;
    
    /* update the image preview */
    update_preview_image (booth);
}

/******************************************************************************
 *
 *  Function:       on_preview_thumb2_button_clicked
 *  Description:    Callback function for the preview_thumb2_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_preview_image
 *
 *****************************************************************************/
void on_preview_thumb2_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the current index value to 1 */
    booth->selected_image_index = 1;
    
    /* update the image preview */
    update_preview_image (booth);
}

/******************************************************************************
 *
 *  Function:       on_preview_thumb3_button_clicked
 *  Description:    Callback function for the preview_thumb3_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_preview_image
 *
 *****************************************************************************/
void on_preview_thumb3_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the current index value to 2 */
    booth->selected_image_index = 2;
    
    /* update the image preview */
    update_preview_image (booth);
}


/* Functions for the fourth screen */

/******************************************************************************
 *
 *  Function:       oilblob_complete
 *  Description:    Callback function for the oilblob process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void oilblob_complete (GPid pid, gint status, DigitalPhotoBooth *booth )
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

/******************************************************************************
 *
 *  Function:       charcoal_complete
 *  Description:    Callback function for the charcoal process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void charcoal_complete (GPid pid, gint status, DigitalPhotoBooth *booth)
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

/******************************************************************************
 *
 *  Function:       texture_complete
 *  Description:    Callback function for the texture process completion
 *  Inputs:         pid - the pid of the exiting process
 *                  status - the exit status of the process *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, image_resize,
 *                  gdk_pixbuf_new_from_file, gtk_image_set_from_pixbuf,
 *                  gtk_image_set_sensitive
 *
 *****************************************************************************/
void texture_complete (GPid pid, gint status, DigitalPhotoBooth *booth)
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

/******************************************************************************
 *
 *  Function:       update_effects_image
 *  Description:    This function updates the larger effects image
 *  Inputs:         booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf
 *
 *****************************************************************************/
void update_effects_image (DigitalPhotoBooth *booth)
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
 *  Function:       on_effects_select_forward_button_clicked
 *  Description:    Callback function for the effects_select_forward_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: gtk_image_set_from_stock, gtk_widget_set_sensitive,
 *                  get_image_filename_pointer, gdk_pixbuf_new_from_file,
 *                  gtk_image_set_from_pixbuf, gtk_notebook_next_page
 *
 *****************************************************************************/
void on_effects_select_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
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
    
    /* move to the next screen */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

/******************************************************************************
 *
 *  Function:       on_effects_thumb1_button_clicked
 *  Description:    Callback function for the effects_thumb1_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_effects_image
 *
 *****************************************************************************/
void on_effects_thumb1_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the effect type to OILBLOB */
    booth->selected_effect_enum = OILBLOB;
    
    /* update the preview image */
    update_effects_image (booth);
}

/******************************************************************************
 *
 *  Function:       on_effects_thumb2_button_clicked
 *  Description:    Callback function for the effects_thumb2_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_effects_image
 *
 *****************************************************************************/
void on_effects_thumb2_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the effect type to CHARCOAL */
    booth->selected_effect_enum = CHARCOAL;
    
    /* update the preview image */
    update_effects_image (booth);
}

/******************************************************************************
 *
 *  Function:       on_effects_thumb3_button_clicked
 *  Description:    Callback function for the effects_thumb3_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_effects_image
 *
 *****************************************************************************/
void on_effects_thumb3_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the effect type to TEXTURE */
    booth->selected_effect_enum = TEXTURE;
    
    /* update the preview image */
    update_effects_image (booth);
}

/******************************************************************************
 *
 *  Function:       on_effects_none_button_clicked
 *  Description:    Callback function for the effects_none_button
 *  Inputs:         button - a pointer to the button object
 *                  booth - a pointer to the DigitalPhotoBooth struct
 *  Outputs:        
 *  Routines Called: update_effects_image
 *
 *****************************************************************************/
void on_effects_none_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* set the effect type to NONE */
    booth->selected_effect_enum = NONE;
    
    /* update the preview image */
    update_effects_image (booth);
}


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
void delivery_init (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* create the button label for usb delivery */
    sprintf (label, "Send to Thumb Drive\nCost: %d Quarters", booth->usb_cost);
    
    /* set the button label to the string buffer content */
    gtk_button_set_label ((GtkButton*)booth->delivery_usb_toggle, label);
    
    /* create the button label for print delivery */
    sprintf (label, "Send to Printer\nCost: %d Quarters",
        booth->print_cost);
    
    /* set the button label to the string buffer content */
    gtk_button_set_label ((GtkButton*)booth->delivery_print_toggle, label);
}

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
void delivery_update (DigitalPhotoBooth *booth)
{
    /* create a label string buffer */
    gchar label[MAX_STRING_LENGTH];
    
    /* create and initialize variables */
    gint required = 0;
    booth->delivery_total_cost = 0;
    
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
    sprintf (label, "Inserted: %d Quarters\nTotal Cost: %d Quarters\n",
        booth->money_inserted, booth->delivery_total_cost);
    
    /* if sufficient funds */
    if (required <= 0)
    {
        /* required can't be negative, set it to zero */
        required = 0;
        
        /* concatenate the string */
        sprintf (strchr (label, 0), "Required: %d Quarters", required);
        
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
        sprintf (strchr(label, 0), "Required: %d Quarters\n\n", required);
        sprintf (strchr(label, 0), 
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
    if (money_pay (booth->delivery_total_cost, booth))
    {
        if (gtk_toggle_button_get_active 
            ((GtkToggleButton*)booth->delivery_usb_toggle))
        {
            /* call usb code */
            printf ("USB DELIVERY!\n");
        }
        
        if (gtk_toggle_button_get_active 
            ((GtkToggleButton*)booth->delivery_print_toggle))
        {
            printf ("PRINTER DELIVERY!\n");
        }
        
        gtk_toggle_button_set_active ((GtkToggleButton*)booth->delivery_usb_toggle, FALSE);
        gtk_toggle_button_set_active ((GtkToggleButton*)booth->delivery_print_toggle, FALSE);
        
        gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
    }
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
    delivery_update (booth);
}


/* Functions for the sixth screen */

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
    gtk_notebook_set_current_page ((GtkNotebook*)booth->wizard_panel, 0);
}

