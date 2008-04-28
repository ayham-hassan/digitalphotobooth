/*
 * To compile, use this command (those are backticks, not single quotes):
 * gcc -Wall -g -o photobooth photobooth.c -export-dynamic `pkg-config gtk+-2.0 libglade-2.0 --cflags --libs`
 */

#include <gtk/gtk.h>
#include <string.h>
#include "proj-nprosser/frame.h"
#include "proj-nprosser/cam.h"
#include "ImageManipulations.h"
#include "FileHandler.h"
#include "photobooth.h"

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
    
    /* enter GTK+ main loop */
    gtk_main ();
    
    /* free memory we allocated for DigitalPhotoBooth struct */
    g_slice_free (DigitalPhotoBooth, booth);

    return 0;
}

/*
We call error_message() any time we want to display an error message to the
user. It will both show an error dialog and log the error to the terminal
window.
*/
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

/*
We call init_app() when our program is starting to load our DigitalPhotoBooth
struct with references to the widgets we need. This is done using GtkBuilder
to read the XML file we created using Glade.
*/
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
    booth->take_photo_forward_button =
        GTK_WIDGET (gtk_builder_get_object (builder, "take_photo_forward_button"));
    booth->take_photo_progress =
        GTK_WIDGET (gtk_builder_get_object (builder, "take_photo_progress"));
    booth->videobox =
        GTK_WIDGET (gtk_builder_get_object (builder, "videobox"));
    booth->image1_preview_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "image1_preview_image"));
    booth->image2_preview_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "image2_preview_image"));
    booth->image3_preview_image =
        GTK_WIDGET (gtk_builder_get_object (builder, "image3_preview_image"));
    booth->image_preview_area =
        GTK_WIDGET (gtk_builder_get_object (builder, "image_preview_area"));
    
    /* setup money handler variables */
    booth->money_total = 4;
    booth->money_inserted = 0;
    money_update(booth);
    
    /* reset the number of photos taken */
    booth->num_photos_taken = 0;
    
    /* set the streaming video pointers to NULL */
    booth->capture = NULL;
    
    /* set the source id fields to zero */
    booth->video_source_id = 0;
	booth->timer_source_id = 0;
	
	booth->selected_image_index = 0;
	
	memset (booth->photos_filenames, 0, NUM_PHOTOS * NUM_PHOTO_TYPES * MAX_STRING_LENGTH);
    
    /* connect signals, passing our DigitalPhotoBooth struct as user data */
    gtk_builder_connect_signals (builder, booth);
                
    /* free memory used by GtkBuilder object */
    g_object_unref (G_OBJECT (builder));
    
    return TRUE;
}

/* 
When our window is destroyed, we want to break out of the GTK main loop. We do
this by calling gtk_main_quit().
*/
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
    
    gtk_main_quit();
}

/* General utility functions */
gchar* get_image_filename_pointer (guint index, enum PHOTO_TYPE pt, DigitalPhotoBooth *booth)
{
    return booth->photos_filenames[index * NUM_PHOTO_TYPES + pt];
}

/* Functions for the first screen */

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
        money_update (booth);
    }

    return FALSE;
}

void money_insert (DigitalPhotoBooth *booth)
{
    /* increase the amount of money inserted into the machine */
    ++booth->money_inserted;
}

void money_update (DigitalPhotoBooth *booth)
{
    /* calculate the money still required */
    gint remaining = booth->money_total - booth->money_inserted;
    
    /* if there is enough, allow the user to proceed */
    if (remaining <= 0)
    {
        remaining = 0;
        gtk_widget_set_sensitive (booth->money_forward_button, TRUE);
    }
    
    /* update the display text */
    sprintf (booth->money_str, 
        "%d Quarters Inserted\nPlease insert %d more quarters to continue",
        booth->money_inserted, remaining);

    /* update the textual display */
    gtk_label_set_text ((GtkLabel*)booth->money_message_label,
        booth->money_str);
}

void on_money_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    /* switch to the next wizard page */
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
    
    /* make sure camera is not already open and open if necessary */
    if (booth->capture == NULL)
    {
        /* open the camera */
        booth->capture = open_camera();
        
        /* start the video stream */
        v4l2CaptureStartStreaming (booth->capture, 0, 4);
    }
	
	/* start a idle source which updates the drawing area */
    booth->video_source_id = g_idle_add ((GSourceFunc)camera_process, booth);
}

/* Functions for the second screen */

void free_frame (guchar *pixels, VidFrame *frame)
{
    /* release the received video frame */
    vidFrameRelease (&frame);
}

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

gboolean take_photo_process (DigitalPhotoBooth *booth)
{
    /* make sure a capture object exists */
    if (booth->capture != NULL)
    {
        /* create the filenames to be used for writing the images */
        gchar *filename = get_image_filename_pointer (booth->num_photos_taken, FULL, booth);
        gchar *filename_sm = get_image_filename_pointer (booth->num_photos_taken, SMALL, booth);
        gchar *filename_lg = get_image_filename_pointer (booth->num_photos_taken, LARGE, booth);

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
            
            /* show the take photo button and hide the progress bar */
            //gtk_widget_show (booth->take_photo_button);
            //gtk_widget_hide (booth->take_photo_progress);
            timer_start(booth);
        }
        else
        {
            /* switch to the next panel */
            //gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
            gtk_widget_hide (booth->take_photo_progress);
            gtk_widget_show (booth->take_photo_forward_button);
        }
    }

    /* return false to cause no further scheduling to occur */
    return FALSE;
}

void on_take_photo_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth)
{
    /* hide the take photo button and show the progress bar */
    gtk_widget_hide (booth->take_photo_button);
    gtk_widget_show (booth->take_photo_progress);
    
    /* start the countdown timer */
    timer_start(booth);
}

void timer_start (DigitalPhotoBooth *booth)
{
    /* initialize the timer fields */
    booth->timer_left = TIMER_PHOTO_SECONDS;
    booth->timer_total = TIMER_PHOTO_SECONDS;
    
    /* set the initial state of the progress bar */
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress,
        1.0 );

    sprintf (booth->progress_bar_label, "The photo will be taken in %d seconds",
        booth->timer_left);
    gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
        booth->progress_bar_label);
    
    /* add a timeout source which fires every second */
    booth->timer_source_id = g_timeout_add_seconds (1,
        (GSourceFunc)timer_process, booth);
}

gboolean timer_process (DigitalPhotoBooth *booth)
{
    /* set the progress bar to the current time left */
    double progress = --booth->timer_left / (double)booth->timer_total;
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress,
        progress );

    /* if the timer hasn't expired, continue the timeout schedule */
    if (booth->timer_left > 0)
    {
        sprintf (booth->progress_bar_label, "The photo will be taken in %d seconds",
            booth->timer_left);
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
            booth->progress_bar_label);
        return TRUE;
    }
    else
    {
        sprintf (booth->progress_bar_label, "Please don't move, the photo is being captured");
        gtk_progress_bar_set_text ((GtkProgressBar*)booth->take_photo_progress,
            booth->progress_bar_label);

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

void on_take_photo_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    gchar *thumb1_filename = get_image_filename_pointer (0, SMALL, booth);
    GdkPixbuf *thumb1_pixbuf = gdk_pixbuf_new_from_file (thumb1_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->image1_preview_image, thumb1_pixbuf);
    
    
    gchar *thumb2_filename = get_image_filename_pointer (1, SMALL, booth);
    GdkPixbuf *thumb2_pixbuf = gdk_pixbuf_new_from_file (thumb2_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->image2_preview_image, thumb2_pixbuf);

    gchar *thumb3_filename = get_image_filename_pointer (2, SMALL, booth);
    GdkPixbuf *thumb3_pixbuf = gdk_pixbuf_new_from_file (thumb3_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->image3_preview_image, thumb3_pixbuf);

    gchar *preview_filename = get_image_filename_pointer (0, LARGE, booth);
    GdkPixbuf *preview_pixbuf = gdk_pixbuf_new_from_file (preview_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->image_preview_area, preview_pixbuf);
    booth->selected_image_index = 0;

    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

/* Functions for the third screen */

void on_photo_select_forward_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    GPid pid;
    printImage ("Img0000.jpg", &pid, NULL);
    
}

void update_preview_image (DigitalPhotoBooth *booth)
{
    gchar *preview_filename = get_image_filename_pointer (booth->selected_image_index, LARGE, booth);
    GdkPixbuf *preview_pixbuf = gdk_pixbuf_new_from_file (preview_filename, NULL);
    gtk_image_set_from_pixbuf ((GtkImage*)booth->image_preview_area, preview_pixbuf);
}

void on_image1_preview_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    booth->selected_image_index = 0;
    update_preview_image (booth);
}

void on_image2_preview_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    booth->selected_image_index = 1;
    update_preview_image (booth);
}

void on_image3_preview_button_clicked (GtkWidget *button,
    DigitalPhotoBooth *booth)
{
    booth->selected_image_index = 2;
    update_preview_image (booth);
}
