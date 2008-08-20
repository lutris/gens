
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

extern GtkListStore* listmodel;
extern GtkWidget* gens_window;
extern int do_callbacks;
/*
 * Public Functions.
 */

/*
 * This function returns a widget in a component created by Glade.
 * Call it with the toplevel widget in the component (i.e. a window/dialog),
 * or alternatively any widget in the component, and the name of the widget
 * you want returned.
 */
GtkWidget*  lookup_widget              (GtkWidget       *widget,
                                        const gchar     *widget_name);


/* Use this function to set the directory containing installed pixmaps. */
void        add_pixmap_directory       (const gchar     *directory);


/*
 * Private Functions.
 */

/* This is used to create the pixmaps used in the interface. */
GtkWidget*  create_pixmap              (GtkWidget       *widget,
                                        const gchar     *filename);

/* This is used to create the pixbufs used in the interface. */
GdkPixbuf*  create_pixbuf              (const gchar     *filename);

/* This is used to set ATK action descriptions. */
void        glade_set_atk_action_description (AtkAction       *action,
                                              const gchar     *action_name,
                                              const gchar     *description);

void open_game_genie();
void open_joypads();

// SDL TEST
void open_sdltest();
void close_sdltest(GtkWidget *sdlwindow);
int run_sdltest(GtkWidget *sdlwindow);

void open_dir_config();
void open_bios_cfg();
void sync_gens_ui(int updateSoundGTK);
void open_cdrom_select ();
void addCode(GtkWidget* treeview, const char* name, const char* code, int selected);
GtkWidget* create_file_chooser_dialog(const char* title, GtkFileChooserAction action);
int Ritorna_Vel();
void addRomsFilter(GtkWidget* widget);
void addStateFilter(GtkWidget* widget);
void addIsoFilter(GtkWidget* widget);
void addCfgFilter(GtkWidget* widget);
void addGymFilter(GtkWidget* widget);
void open_msgbox(const char * msg);
void initializeConsoleRomsView();
