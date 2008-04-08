/*
 * To compile, use this command (those are backticks, not single quotes):
 * gcc -Wall -g -o photobooth photobooth.c -export-dynamic `pkg-config gtk+-2.0 libglade-2.0 --cflags --libs`
 */

#include <gtk/gtk.h>

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE "photobooth.xml"

typedef struct
{
    GtkWidget *window;
    GtkWidget *wizard_panel;
    GtkWidget *take_photo_progress;
    GtkWidget *money_message_label;
    GtkWidget *money_forward_button;
    gint timer_left;
    gint timer_total;
    gint timer_id;
    gint money_inserted;
    gint money_total;
    gchar money_str[255];
} DigitalPhotoBooth;

/* window callback prototypes */
void on_window_destroy (GtkObject *object, DigitalPhotoBooth *booth);

/* button callback prototypes */
void on_forward_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth);
void on_back_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth);
void on_money_forward_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth);
void on_take_photo_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth);

/* keyboard event callback prototypes */
gboolean on_window_key_press_event (GtkWidget *window, GdkEventKey *event, DigitalPhotoBooth *booth);

/* countdown timer prototypes */
void timer_start (DigitalPhotoBooth *booth);
gboolean timer_process (DigitalPhotoBooth *booth);

/* money handling functions */
void money_insert (DigitalPhotoBooth *booth);
void money_update (DigitalPhotoBooth *booth);

/* misc. function prototypes */
void error_message (const gchar *message);
gboolean init_app (DigitalPhotoBooth *booth);

int main (int argc, char *argv[])
{
    DigitalPhotoBooth *booth;
    
    /* allocate the memory needed by our DigitalPhotoBooth struct */
    booth = g_slice_new (DigitalPhotoBooth);

    /* initialize GTK+ libraries */
    gtk_init (&argc, &argv);
    
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
                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_OK,
                                     message);
    
    gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
    gtk_dialog_run (GTK_DIALOG (dialog));      
    gtk_widget_destroy (dialog);         
}

/*
We call init_app() when our program is starting to load our TutorialTextEditor struct
with references to the widgets we need. This is done using GtkBuilder to read
the XML file we created using Glade.
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
    booth->wizard_panel = GTK_WIDGET (gtk_builder_get_object (builder, "wizard_panel"));
    booth->money_message_label = GTK_WIDGET (gtk_builder_get_object (builder, "money_message_label"));
    booth->take_photo_progress = GTK_WIDGET (gtk_builder_get_object (builder, "take_photo_progress"));
    booth->money_forward_button = GTK_WIDGET (gtk_builder_get_object (builder, "money_forward_button"));
    
    /* setup money handler variables */
    booth->money_total = 4;
    booth->money_inserted = 0;
    money_update(booth);
    
    /* connect signals, passing our TutorialTextEditor struct as user data */
    gtk_builder_connect_signals (builder, booth);
                
    /* free memory used by GtkBuilder object */
    g_object_unref (G_OBJECT (builder));
    
    return TRUE;
}

/* 
When our window is destroyed, we want to break out of the GTK main loop. We do
this by calling gtk_main_quit(). We could have also just specified gtk_main_quit
as the handler in Glade!
*/
void on_window_destroy (GtkObject *object, DigitalPhotoBooth *booth)
{
    gtk_main_quit();
}

void on_forward_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth)
{
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

void on_back_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth)
{
    gtk_notebook_prev_page ((GtkNotebook*)booth->wizard_panel);
}

void on_money_forward_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth)
{
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
}

void on_take_photo_button_clicked (GtkWidget *button, DigitalPhotoBooth *booth)
{
    gtk_notebook_next_page ((GtkNotebook*)booth->wizard_panel);
    timer_start(booth);
}

void timer_start (DigitalPhotoBooth *booth)
{
    booth->timer_left = 4;
    booth->timer_total = 5;
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress, 1.0 );
    booth->timer_id = gtk_timeout_add (1000, (GtkFunction) timer_process, booth);
}

gboolean timer_process (DigitalPhotoBooth *booth)
{
    gtk_progress_bar_set_fraction ((GtkProgressBar*)booth->take_photo_progress, booth->timer_left / (double)booth->timer_total );
    (booth->timer_left)--;
    
    if ((booth->timer_left) >= 0)
    {
        return TRUE;
    }
    else
    {
        gtk_timeout_remove(booth->timer_id);
        gtk_notebook_prev_page((GtkNotebook*)booth->wizard_panel);
        return FALSE;
    }
}

void money_insert (DigitalPhotoBooth *booth)
{
    ++booth->money_inserted;
}

void money_update (DigitalPhotoBooth *booth)
{
    gint remaining = booth->money_total - booth->money_inserted;
    
    if (remaining <= 0)
    {
        remaining = 0;
        gtk_widget_set_sensitive (booth->money_forward_button, TRUE);
    }
    
    sprintf (booth->money_str, "%d Coins Inserted\nPlease insert %d more coins to continue", booth->money_inserted, remaining);
    gtk_label_set_text ((GtkLabel*)booth->money_message_label, booth->money_str);
}

gboolean on_window_key_press_event (GtkWidget *window, GdkEventKey *event, DigitalPhotoBooth *booth)
{
    if (event->keyval == 102 || event->keyval == 70)
    {
        gtk_window_fullscreen ((GtkWindow*)booth->window);
    }
    else if (event->keyval == 71 || event->keyval == 103 )
    {
        gtk_window_unfullscreen ((GtkWindow*)booth->window);
    }
    else if (event->keyval == 67 || event->keyval == 99)
    {
        money_insert (booth);
        money_update (booth);
    }

    return FALSE;
}
