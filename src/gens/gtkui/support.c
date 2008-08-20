#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "support.h"
#include "interface.h"
#include "ym2612.h"
#include "gens.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "g_main.h"
#include "cd_sys.h"
#include "mem_m68k.h"
#include "vdp_rend.h"
#include "vdp_io.h"
#include "ggenie.h"
#include "callbacks.h"
#include "save.h"
#include "scrshot.h"
#include "io.h"
#include "ui_proxy.h"
#include <assert.h>

GtkListStore *listmodel = NULL;
int do_callbacks = 1;

GtkWidget *
lookup_widget (GtkWidget * widget, const gchar * widget_name)
{
	GtkWidget *parent, *found_widget;

	for (;;)
	{
		if (GTK_IS_MENU (widget))
			parent = gtk_menu_get_attach_widget (GTK_MENU
							     (widget));
		else
			parent = widget->parent;
		if (!parent)
			parent = g_object_get_data (G_OBJECT (widget),
						    "GladeParentKey");
		if (parent == NULL)
			break;
		widget = parent;
	}

	found_widget = (GtkWidget *) g_object_get_data (G_OBJECT (widget),
							widget_name);
	if (!found_widget)
		g_warning ("Widget not found: %s", widget_name);
	return found_widget;
}

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory (const gchar * directory)
{
	pixmaps_directories = g_list_prepend (pixmaps_directories,
					      g_strdup (directory));
}

/* This is an internally used function to find pixmap files. */
static gchar *
find_pixmap_file (const gchar * filename)
{
	GList *elem;

	/* We step through each of the pixmaps directory to find it. */
	elem = pixmaps_directories;
	while (elem)
	{
		gchar *pathname =
			g_strdup_printf ("%s%s%s", (gchar *) elem->data,
					 G_DIR_SEPARATOR_S, filename);
		if (g_file_test (pathname, G_FILE_TEST_EXISTS))
			return pathname;
		g_free (pathname);
		elem = elem->next;
	}
	return NULL;
}

/* This is an internally used function to create pixmaps. */
GtkWidget *
create_pixmap (GtkWidget * widget, const gchar * filename)
{
	gchar *pathname = NULL;
	GtkWidget *pixmap;

	if (!filename || !filename[0])
		return gtk_image_new ();

	pathname = find_pixmap_file (filename);

	if (!pathname)
	{
		g_warning ("Couldn't find pixmap file: %s", filename);
		return gtk_image_new ();
	}

	pixmap = gtk_image_new_from_file (pathname);
	g_free (pathname);
	return pixmap;
}

/* This is an internally used function to create pixmaps. */
GdkPixbuf *
create_pixbuf (const gchar * filename)
{
	gchar *pathname = NULL;
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	if (!filename || !filename[0])
		return NULL;

	pathname = find_pixmap_file (filename);

	if (!pathname)
	{
		g_warning ("Couldn't find pixmap file: %s", filename);
		return NULL;
	}

	pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
	if (!pixbuf)
	{
		fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
			 pathname, error->message);
		g_error_free (error);
	}
	g_free (pathname);
	return pixbuf;
}

/* This is used to set ATK action descriptions. */
void
glade_set_atk_action_description (AtkAction * action,
				  const gchar * action_name,
				  const gchar * description)
{
	gint n_actions, i;

	n_actions = atk_action_get_n_actions (action);
	for (i = 0; i < n_actions; i++)
	{
		if (!strcmp (atk_action_get_name (action, i), action_name))
			atk_action_set_description (action, i, description);
	}
}

void
addCode (GtkWidget * treeview, const char *name, const char *code, int selected)
{
	GtkTreeIter iter;
	gtk_list_store_append (listmodel, &iter);
	gtk_list_store_set (GTK_LIST_STORE (listmodel), &iter,
			    0, name, 1, code, -1);
	if (selected && treeview)
	{
		GtkTreeSelection *select;
		select = gtk_tree_view_get_selection (GTK_TREE_VIEW
						      (treeview));
		gtk_tree_selection_select_iter (select, &iter);
	}
}



void initializeConsoleRomsView()
{
	GtkWidget *romList,*consoleList;
	GtkListStore *romListModel,*consoleListModel;
	GtkCellRenderer *rendererRom,*rendererConsole;
	GtkTreeViewColumn *columnRom, *columnConsole;
	GtkTreeIter iterRom;//,iterConsole;
	
	romList = lookup_widget (gens_window, "romList");
	consoleList = lookup_widget (gens_window, "consoleList");
	
	romListModel = gtk_list_store_new (1, G_TYPE_STRING);
	consoleListModel = gtk_list_store_new (1, G_TYPE_STRING);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW (romList), GTK_TREE_MODEL (romListModel));
	gtk_tree_view_set_model(GTK_TREE_VIEW (consoleList), GTK_TREE_MODEL (consoleListModel));
	
	rendererRom = gtk_cell_renderer_text_new();
	rendererConsole = gtk_cell_renderer_text_new();
	columnRom = gtk_tree_view_column_new_with_attributes ("Name", rendererRom, "text", 0, NULL);
	columnConsole = gtk_tree_view_column_new_with_attributes ("Name", rendererConsole, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (romList), columnRom);
	gtk_tree_view_append_column (GTK_TREE_VIEW (consoleList), columnConsole);
	
	gtk_list_store_append (consoleListModel, &iterRom);
	gtk_list_store_set (GTK_LIST_STORE (consoleListModel), &iterRom, 0, "Genesis",  -1);
	gtk_list_store_append (consoleListModel, &iterRom);
	gtk_list_store_set (GTK_LIST_STORE (consoleListModel), &iterRom, 0, "32X",  -1);
	gtk_list_store_append (consoleListModel, &iterRom);
	gtk_list_store_set (GTK_LIST_STORE (consoleListModel), &iterRom, 0, "SegaCD",  -1);
	
/*	buildGenesisRomList();
	buildGenesis32xList();
	buildGenesisIsoList();*/
}


void
open_game_genie ()
{
	GtkWidget *game_genie;
	GtkWidget *treeview;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column, *column2;
	GtkTreeSelection *select;
	int i;

	game_genie = create_game_genie ();
	gtk_window_set_transient_for (GTK_WINDOW(game_genie),GTK_WINDOW(gens_window));
	
	treeview = lookup_widget (game_genie, "ggListCode");
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_MULTIPLE);

	if (listmodel)
	{
		gtk_list_store_clear (listmodel);
	}
	else
	{
		listmodel = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	}
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (listmodel));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", 0, NULL);
	column2 = gtk_tree_view_column_new_with_attributes ("Code", renderer, "text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column2);

	for (i = 0; i < 256; i++)
	{
		if (Liste_GG[i].code[0] != 0)
		{
			addCode (treeview, Liste_GG[i].name, Liste_GG[i].code,
				 Liste_GG[i].active);

			if ((Liste_GG[i].restore != 0xFFFFFFFF)
			    && (Liste_GG[i].addr < Rom_Size)
			    && (Genesis_Started))
			{
				Rom_Data[Liste_GG[i].addr] =
					(unsigned char) (Liste_GG[i].
							 restore & 0xFF);
				Rom_Data[Liste_GG[i].addr + 1] =
					(unsigned
					 char) ((Liste_GG[i].
						 restore & 0xFF00) >> 8);
			}
		}
	}

	gtk_widget_show_all (game_genie);
}


void
sync_gens_ui (int updateSoundGTK)
{
	GtkWidget *vsync, *stretch, *sprite_limit, *perfect_synchro;
	GtkWidget *sram_size_0;
	GtkWidget *sram_size[4];

	GtkWidget *opengl,*bpp16,*bpp24,*bpp32;
	GtkWidget *res320x240,*res640x480,*res800x600,*res1024x768, *rescustom;
	GtkWidget *enable_sound, *rate_11, *rate_16, *rate_22, *rate_32,*rate_44, *rate_48 ,*stereo;
	GtkWidget *ym2612, *ym2612imp, *dac, *dacimp, *psg, *psgimp, *z80,
		*pcm, *pwm, *cdda;
	GtkWidget *wav_dump, *gym_dump;

	GtkWidget *rst_main_68k, *rst_sub_68k, *rst_68k, *rst_msh2, *rst_ssh2;
	

	GtkWidget *gen_68k, *gen_z80, *gen_vdp, //Unused variables lie here
		*cd_68k, *cd_cdc, *cd_gfx, *_32x_msh2, *_32x_ssh2, *_32x_vdp;
	
	GtkWidget *render[NB_FILTER];
	GtkWidget *state[10];
	GtkWidget *frame_skip_auto;
	GtkWidget *frame_skip[9];

	GtkWidget *rom_menu;
	GtkWidget *rom_item;
	int i;

	do_callbacks = 0;
	
	enable_sound = lookup_widget (gens_window, "enable_sound");
	rate_11 = lookup_widget (gens_window, "rate_11025");
	rate_16 = lookup_widget (gens_window, "rate_16000");
	rate_22 = lookup_widget (gens_window, "rate_22050");
	rate_32 = lookup_widget (gens_window, "rate_32000");
	rate_44 = lookup_widget (gens_window, "rate_44100");
	rate_48 = lookup_widget (gens_window, "rate_48000");
	ym2612 = lookup_widget (gens_window, "ym2612");
	ym2612imp = lookup_widget (gens_window, "ym2612_improved");
	dac = lookup_widget (gens_window, "dac");
	dacimp = lookup_widget (gens_window, "dac_improved");
	psg = lookup_widget (gens_window, "psg");
	psgimp = lookup_widget (gens_window, "psg_improved");
	z80 = lookup_widget (gens_window, "z80");
	pcm = lookup_widget (gens_window, "pcm");
	pwm = lookup_widget (gens_window, "pwm");
	cdda = lookup_widget (gens_window, "cdda");
	stereo = lookup_widget (gens_window, "stereo");
	wav_dump = lookup_widget (gens_window, "start_wav_dump");
	gym_dump = lookup_widget (gens_window, "start_gym_dump");
	
	switch (updateSoundGTK)
	{
		
		// UPDATE_GTK = 0
		// #defined in g_main.h
		case 0:
			
		rst_main_68k = lookup_widget (gens_window, "reset_main_68000");
		rst_sub_68k = lookup_widget (gens_window, "reset_sub_68000");
		rst_68k = lookup_widget (gens_window, "reset_68000");
		rst_msh2 = lookup_widget (gens_window, "reset_main_sh2");
		rst_ssh2 = lookup_widget (gens_window, "reset_sub_sh2");
	
		vsync = lookup_widget (gens_window, "vsync");
		stretch = lookup_widget (gens_window, "stretch");
		opengl = lookup_widget (gens_window, "opengl");
		sprite_limit = lookup_widget (gens_window, "sprite_limit");
		perfect_synchro = lookup_widget (gens_window, "perfect_synchro");
	
		bpp16 = lookup_widget (gens_window, "bpp16");
		bpp24 = lookup_widget (gens_window, "bpp24");
		bpp32 = lookup_widget (gens_window, "bpp32");
	
		opengl = lookup_widget (gens_window, "opengl");
		res320x240 = lookup_widget (gens_window, "_320x240");
		res640x480 = lookup_widget (gens_window, "_640x480");
		res800x600 = lookup_widget (gens_window, "_800x600");
		res1024x768 = lookup_widget (gens_window, "_1024x768");
		rescustom = lookup_widget (gens_window, "custom");
	
	
		sram_size_0 = lookup_widget (gens_window, "none");
		sram_size[0] = lookup_widget (gens_window, "_8_kb");
		sram_size[1] = lookup_widget (gens_window, "_16_kb");
		sram_size[2] = lookup_widget (gens_window, "_32_kb");
		sram_size[3] = lookup_widget (gens_window, "_64_kb");
	
		render[NORMAL] = lookup_widget (gens_window, "normal");
		render[DOUBLE] = lookup_widget (gens_window, "_double");
		render[INTERPOLATED] = lookup_widget (gens_window, "interpolated");
		render[FULL_SCANLINE] = lookup_widget (gens_window, "scanline");
		render[SCANLINE_50] = lookup_widget (gens_window, "_50_scanline");
		render[SCANLINE_25] = lookup_widget (gens_window, "_25_scanline");
		render[INTERPOLATED_SCANLINE] = lookup_widget (gens_window, "interpolated_scanline");
		render[INTERPOLATED_SCANLINE_50] = lookup_widget (gens_window, "interpolated_50_scanline");
		render[INTERPOLATED_SCANLINE_25] = lookup_widget (gens_window, "interpolated_25_scanline");
		render[KREED] = lookup_widget (gens_window, "_2xsai_kreed");
		render[SCALE2X] = lookup_widget (gens_window, "_scale2x");
		render[HQ2X] = lookup_widget (gens_window, "hq2x");
	
		state[0] = lookup_widget (gens_window, "change_state_slot0");
		state[1] = lookup_widget (gens_window, "change_state_slot1");
		state[2] = lookup_widget (gens_window, "change_state_slot2");
		state[3] = lookup_widget (gens_window, "change_state_slot3");
		state[4] = lookup_widget (gens_window, "change_state_slot4");
		state[5] = lookup_widget (gens_window, "change_state_slot5");
		state[6] = lookup_widget (gens_window, "change_state_slot6");
		state[7] = lookup_widget (gens_window, "change_state_slot7");
		state[8] = lookup_widget (gens_window, "change_state_slot8");
		state[9] = lookup_widget (gens_window, "change_state_slot9");
	
		frame_skip_auto = lookup_widget (gens_window, "frame_auto_skip");
		frame_skip[0] = lookup_widget (gens_window, "frame_0_skip");
		frame_skip[1] = lookup_widget (gens_window, "frame_1_skip");
		frame_skip[2] = lookup_widget (gens_window, "frame_2_skip");
		frame_skip[3] = lookup_widget (gens_window, "frame_3_skip");
		frame_skip[4] = lookup_widget (gens_window, "frame_4_skip");
		frame_skip[5] = lookup_widget (gens_window, "frame_5_skip");
		frame_skip[6] = lookup_widget (gens_window, "frame_6_skip");
		frame_skip[7] = lookup_widget (gens_window, "frame_7_skip");
		frame_skip[8] = lookup_widget (gens_window, "frame_8_skip");
	
		cd_68k = lookup_widget (gens_window, "segacd_68000");
		cd_cdc = lookup_widget (gens_window, "segacd_cdc");
		cd_gfx = lookup_widget (gens_window, "segacd_gfx");
		_32x_msh2 = lookup_widget (gens_window, "_32x_main_sh2");
		_32x_ssh2 = lookup_widget (gens_window, "_32x_sub_sh2");
		_32x_vdp = lookup_widget (gens_window, "_32x_vdp");
	
		gtk_widget_hide (cd_68k);
		gtk_widget_hide (cd_cdc);
		gtk_widget_hide (cd_gfx);
		gtk_widget_hide (_32x_msh2);
		gtk_widget_hide (_32x_ssh2);
		gtk_widget_hide (_32x_vdp);
	
		if (SegaCD_Started)
		{
			gtk_widget_show (cd_68k);
			gtk_widget_show (cd_cdc);
			gtk_widget_show (cd_gfx);
		}
	
		if (_32X_Started)
		{
			gtk_widget_show (_32x_msh2);
			gtk_widget_show (_32x_ssh2);
			gtk_widget_show (_32x_vdp);
		}
	
	
		gtk_widget_hide (rst_main_68k);
		gtk_widget_hide (rst_sub_68k);
		gtk_widget_hide (rst_68k);
		gtk_widget_hide (rst_msh2);
		gtk_widget_hide (rst_ssh2);
	
		if (SegaCD_Started)
		{
			gtk_widget_show (rst_main_68k);
			gtk_widget_show (rst_sub_68k);
		}
		else if (_32X_Started)
		{
			gtk_widget_show (rst_68k);
			gtk_widget_show (rst_msh2);
			gtk_widget_show (rst_ssh2);
		}
		else
		{
			gtk_widget_show (rst_68k);
		}
		
		// All of these commands manage to trigger the
		// activaion events in callbacks.c
			
		// Hopefully there is a work around to this
		
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (vsync), W_VSync);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (stretch),
						Stretch);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (opengl), Opengl);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (sprite_limit),
						Sprite_Over);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (perfect_synchro),
						SegaCD_Accurate);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (enable_sound),
						Sound_Enable);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (stereo),
						Sound_Stereo);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (z80), Z80_State);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (ym2612),
						YM2612_Enable);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (ym2612imp),
						YM2612_Improv);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (dac),
						DAC_Enable);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (dacimp),
						DAC_Improv);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (psg),
						PSG_Enable);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (psgimp),
						PSG_Improv);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (pcm),
						PCM_Enable);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (pwm),
						PWM_Enable);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (cdda),
						CDDA_Enable);
	
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
						(state[Current_State]), 1);
		switch(Bpp)
			{
						 case 16:
										 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (bpp16),1);
						 break;  
						 
						 case 24:
										 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (bpp24),1);
						 break;
						 
						 case 32:
										 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (bpp32),1);
						 break;
			}
			if(Width_gl==320 && Height_gl==240)
			{
						 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (res320x240),1);
			}
		 
			else if (Width_gl==640 && Height_gl==480)
			{
						 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (res640x480),1);
			}
		else if (Width_gl==800 && Height_gl==600)
			{
						 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (res800x600),1);
			}
			else if (Width_gl==1024 && Height_gl==768)
			{
						 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (res1024x768),1);
			}
			/*else 
			{
						 gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rescustom),1);
			}*/
		if (-1 == Frame_Skip)
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
							(frame_skip_auto), 1);
		else
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
							(frame_skip[Frame_Skip]), 1);
	
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
						(render[Render_Mode]), 1);
	
		switch (Sound_Rate)
		{
		case 11025:
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rate_11),
							1);
			break;
		case 16000:
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rate_16),
							1);
			break;
		case 22050:
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rate_22),
							1);
			break;
		case 32000:
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rate_32),
							1);
			break;	
		case 44100:
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rate_44),
							1);
			break;
		case 48000:
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (rate_48),
							1);
			break;
			
		}
	
		if (BRAM_Ex_State & 0x100)
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
							(sram_size[BRAM_Ex_Size]), 1);
		else
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
							(sram_size_0), 1);
	
	
		rom_item = lookup_widget (gens_window, "rom_history");
		gtk_menu_item_remove_submenu (GTK_MENU_ITEM (rom_item));
		rom_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (rom_item), rom_menu);
	
		for (i = 0; i < 9; i++)
		{
			if (strcmp (Recent_Rom[i], ""))
			{
				char tmp[1024];
				GtkWidget *item;
	
				switch (Detect_Format (Recent_Rom[i]) >> 1)	// do not exist anymore
				{
				default:
					strcpy (tmp, "[---]\t- ");
					break;
	
				case 1:
					strcpy (tmp, "[MD]\t- ");
					break;
	
				case 2:
					strcpy (tmp, "[32X]\t- ");
					break;
	
				case 3:
					strcpy (tmp, "[SCD]\t- ");
					break;
	
				case 4:
					strcpy (tmp, "[SCDX]\t- ");
					break;
				}
	
				Get_Name_From_Path (Recent_Rom[i], Str_Tmp);
				strcat (tmp, Str_Tmp);
	
				item = gtk_menu_item_new_with_label (tmp);
				gtk_widget_show (item);
				gtk_container_add (GTK_CONTAINER (rom_menu), item);
				g_signal_connect ((gpointer) item, "activate",
							G_CALLBACK
							(on_rom_history_activate),
							GINT_TO_POINTER (i));
			}
			else
				break;
		}
		break;
	
		// UPDATE_GTK_SOUNDON = 1
		// #defined in g_main.h
		case 1:
		
		// Makes the sound related widgets sensitive
			gtk_widget_set_sensitive (stereo, TRUE);
			gtk_widget_set_sensitive (z80, TRUE);
			gtk_widget_set_sensitive (ym2612, TRUE);
			gtk_widget_set_sensitive (ym2612imp, TRUE);
			gtk_widget_set_sensitive (dac, TRUE);
			gtk_widget_set_sensitive (dacimp, TRUE);
			gtk_widget_set_sensitive (psg, TRUE);
			gtk_widget_set_sensitive (psgimp, TRUE);
			gtk_widget_set_sensitive (pcm, TRUE);
			gtk_widget_set_sensitive (pwm, TRUE);
			gtk_widget_set_sensitive (cdda, TRUE);
							
			gtk_widget_set_sensitive (rate_11, TRUE);
			gtk_widget_set_sensitive (rate_16, TRUE);
			gtk_widget_set_sensitive (rate_22, TRUE);
			gtk_widget_set_sensitive (rate_32, TRUE);
			gtk_widget_set_sensitive (rate_44, TRUE);
			gtk_widget_set_sensitive (rate_48, TRUE);	
			gtk_widget_set_sensitive (gym_dump, TRUE);
		
			break;
		
		// UPDATE_GTK_SOUNDOFF = 2
		// #defined in g_main.h
		case 2:
			
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
							(enable_sound), FALSE);
		
		// Makes the sound related widgets insensitive
			gtk_widget_set_sensitive (stereo, FALSE);
			gtk_widget_set_sensitive (z80, FALSE);
			gtk_widget_set_sensitive (ym2612, FALSE);
			gtk_widget_set_sensitive (ym2612imp, FALSE);
			gtk_widget_set_sensitive (dac, FALSE);
			gtk_widget_set_sensitive (dacimp, FALSE);
			gtk_widget_set_sensitive (psg, FALSE);
			gtk_widget_set_sensitive (psgimp, FALSE);
			gtk_widget_set_sensitive (pcm, FALSE);
			gtk_widget_set_sensitive (pwm, FALSE);
			gtk_widget_set_sensitive (cdda, FALSE);
							
			gtk_widget_set_sensitive (rate_11, FALSE);
			gtk_widget_set_sensitive (rate_16, FALSE);
			gtk_widget_set_sensitive (rate_22, FALSE);
			gtk_widget_set_sensitive (rate_32, FALSE);
			gtk_widget_set_sensitive (rate_44, FALSE);
			gtk_widget_set_sensitive (rate_48, FALSE);	
			gtk_widget_set_sensitive (gym_dump, FALSE);
			
			break;
	}
	
	do_callbacks = 1;
}

static void
Setting_Keys_Proxy (GtkWidget * button, gpointer data)
{
	GtkWidget *control_window;
	GtkWidget *pad = NULL;
	int player;
	int type;
	control_window = lookup_widget (button, "controllers_settings");
	player = GPOINTER_TO_INT (data);

	switch (player)
	{
	case 0:
		pad = lookup_widget (control_window, "padp1");
		break;
	case 2:
		pad = lookup_widget (control_window, "padp1b");
		break;
	case 3:
		pad = lookup_widget (control_window, "padp1c");
		break;
	case 4:
		pad = lookup_widget (control_window, "padp1d");
		break;
	case 1:
		pad = lookup_widget (control_window, "padp2");
		break;
	case 5:
		pad = lookup_widget (control_window, "padp2b");
		break;
	case 6:
		pad = lookup_widget (control_window, "padp2c");
		break;
	case 7:
		pad = lookup_widget (control_window, "padp2d");
		break;
	}

	type = gtk_option_menu_get_history (GTK_OPTION_MENU (pad));

	Setting_Keys (control_window, player, type);
}


void
open_joypads ()
{
	GtkWidget *ctrlset;
	GtkWidget *btnredef1, *btnredef1b, *btnredef1c, *btnredef1d;
	GtkWidget *btnredef2, *btnredef2b, *btnredef2c, *btnredef2d;

	GtkWidget *padp1, *padp1b, *padp1c, *padp1d;
	GtkWidget *padp2, *padp2b, *padp2c, *padp2d;
	GtkWidget *port1, *port2;

	//if (Check_If_Kaillera_Running()) return 0;
	End_Input ();
	//Init_Input();
	ctrlset = create_controllers_settings ();
	gtk_window_set_transient_for (GTK_WINDOW(ctrlset),
															GTK_WINDOW(gens_window));
	port1 = lookup_widget (ctrlset, "port1");
	port2 = lookup_widget (ctrlset, "port2");

	padp1 = lookup_widget (ctrlset, "padp1");
	padp1b = lookup_widget (ctrlset, "padp1b");
	padp1c = lookup_widget (ctrlset, "padp1c");
	padp1d = lookup_widget (ctrlset, "padp1d");
	padp2 = lookup_widget (ctrlset, "padp2");
	padp2b = lookup_widget (ctrlset, "padp2b");
	padp2c = lookup_widget (ctrlset, "padp2c");
	padp2d = lookup_widget (ctrlset, "padp2d");

	btnredef1 = lookup_widget (ctrlset, "buttonRedef1");
	btnredef1b = lookup_widget (ctrlset, "buttonRedef1B");
	btnredef1c = lookup_widget (ctrlset, "buttonRedef1C");
	btnredef1d = lookup_widget (ctrlset, "buttonRedef1D");

	btnredef2 = lookup_widget (ctrlset, "buttonRedef2");
	btnredef2b = lookup_widget (ctrlset, "buttonRedef2B");
	btnredef2c = lookup_widget (ctrlset, "buttonRedef2C");
	btnredef2d = lookup_widget (ctrlset, "buttonRedef2D");

	gtk_option_menu_set_history (GTK_OPTION_MENU (port1),
				     (Controller_1_Type >> 4));
	gtk_option_menu_set_history (GTK_OPTION_MENU (port2),
				     (Controller_2_Type >> 4));

	gtk_option_menu_set_history (GTK_OPTION_MENU (padp1),
				     Controller_1_Type);
	gtk_option_menu_set_history (GTK_OPTION_MENU (padp1b),
				     Controller_1B_Type);
	gtk_option_menu_set_history (GTK_OPTION_MENU (padp1c),
				     Controller_1C_Type);
	gtk_option_menu_set_history (GTK_OPTION_MENU (padp1d),
				     Controller_1D_Type);

	gtk_option_menu_set_history (GTK_OPTION_MENU (padp2),
				     Controller_2_Type);
	gtk_option_menu_set_history (GTK_OPTION_MENU (padp2b),
				     Controller_2B_Type);
	gtk_option_menu_set_history (GTK_OPTION_MENU (padp2c),
				     Controller_2C_Type);
	gtk_option_menu_set_history (GTK_OPTION_MENU (padp2d),
				     Controller_2D_Type);

	g_signal_connect (GTK_OBJECT (btnredef1), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (0));
	g_signal_connect (GTK_OBJECT (btnredef1b), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (2));
	g_signal_connect (GTK_OBJECT (btnredef1c), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (3));
	g_signal_connect (GTK_OBJECT (btnredef1d), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (4));

	g_signal_connect (GTK_OBJECT (btnredef2), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (1));
	g_signal_connect (GTK_OBJECT (btnredef2b), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (5));
	g_signal_connect (GTK_OBJECT (btnredef2c), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (6));
	g_signal_connect (GTK_OBJECT (btnredef2d), "clicked",
			  G_CALLBACK (Setting_Keys_Proxy),
			  GINT_TO_POINTER (7));

	gtk_widget_show_all (ctrlset);
	Init_Input ();
}

void
open_sdltest ()
{
	// Function created by Josh Mattila
	// Reason: GUI initiator for the sdl_test window
	
	// Creates the window and shows it

	GtkWidget *sdltest;
	GtkWidget *btn_ok, *btn_runtest, *status_test;
	
	sdltest = create_sdl_test ();
	gtk_window_set_transient_for (GTK_WINDOW (sdltest),
				      GTK_WINDOW (gens_window));
	
	btn_runtest = lookup_widget (sdltest, "RunTest");
	status_test = lookup_widget (sdltest, "sdltest_status");

	btn_ok = lookup_widget (sdltest, "sdlok");
	
	g_signal_connect (btn_ok, "clicked",
					G_CALLBACK (on_sdlok_clicked),
					NULL);
	g_signal_connect (btn_runtest, "clicked",
					G_CALLBACK (on_RunTest_clicked),
					NULL);

	gtk_widget_show_all (sdltest);
}

void
close_sdltest (GtkWidget *sdlwindow)
{
	// Function created by Josh Mattila
	// Reason: GUI destructor of the sdl_test window
	
	// Destroys the window

	gtk_widget_destroy(sdlwindow);
}

int
run_sdltest (GtkWidget *sdlwindow)
{
	// Function created by Josh Mattila
	// Reason: This might solve issues with SDL installations
	
	// This function initiates the SDL sound subsystem and destroys
	// it exactly 5 times.  Before I had SDL installed correctly,
	// this would crash the program with a segmentation fault for
	// some reason.
	
	GtkWidget *btn_ok, *btn_runtest, *status_test;
	
	int tmpvar;
	char buf[80];
	
	btn_runtest = lookup_widget (sdlwindow, "RunTest");
	status_test = lookup_widget (sdlwindow, "sdltest_status");

	btn_ok = lookup_widget (sdlwindow, "sdlok");
	
	gtk_widget_set_sensitive (btn_runtest, FALSE);
	gtk_widget_set_sensitive (btn_ok, FALSE);
	
	gtk_label_set_markup (GTK_LABEL (status_test), "Starting Test..");
	
	End_Sound();
	gtk_label_set_markup (GTK_LABEL (status_test), "Starting Test...");
	
	for(tmpvar=0; tmpvar <= 5; tmpvar++)
	{
		
			if (Init_Sound() == 0)
			{
				gtk_label_set_markup (GTK_LABEL (status_test), 
						"<span foreground='red'>SDL Test Failed!</span>");
				return 0; // test failed
			}
			
			sprintf(buf, "<span foreground='#388c00'>SDL Test %i Successfull!</span>", tmpvar);
			gtk_label_set_markup (GTK_LABEL (status_test), buf);
		
	}
	gtk_label_set_markup (GTK_LABEL (status_test), 
			"<span foreground='#004eb7'>SDL Tests Successful!</span>");
	
	gtk_widget_set_sensitive (btn_runtest, TRUE);
	gtk_widget_set_sensitive (btn_ok, TRUE);
	
	return 1;  // return 1 when it works =)
}

static void
on_change_dir (GtkWidget * button, gpointer data)
{
	GtkWidget* widget;
	gchar* filename;
//	gchar* slashfilename;
	gint res;
	widget = create_file_chooser_dialog("Change Directory" ,GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
//	gtk_window_set_transient_for (GTK_WINDOW(widget),GTK_WINDOW(gens_window));
	
	//fileselection_set_dir (fd.filesel, Rom_Dir);
	res = gtk_dialog_run(GTK_DIALOG(widget));
	if (res == GTK_RESPONSE_OK)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	}
	else
	{
		filename = "";
	}
	
	gtk_widget_destroy(widget);
    if (res == GTK_RESPONSE_CANCEL)
    	return;
	gtk_entry_set_text (GTK_ENTRY (data), filename);
//	free (filename);
}

static void
on_change_file (GtkWidget * button, gpointer data)
{
	GtkWidget* widget;
	gchar* filename;
	gint res;
	widget = create_file_chooser_dialog("Choose file" ,GTK_FILE_CHOOSER_ACTION_OPEN);
//	gtk_window_set_transient_for (GTK_WINDOW(widget),GTK_WINDOW(gens_window));
	
	//fileselection_set_dir (fd.filesel, Rom_Dir);
	res = gtk_dialog_run(GTK_DIALOG(widget));
	
	if (res == GTK_RESPONSE_OK)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	}
	else
	{
		filename = "";
	}
	
	gtk_widget_destroy(widget);
	if (res == GTK_RESPONSE_CANCEL)
		return;
	gtk_entry_set_text (GTK_ENTRY (data), filename);
//	free (filename);
}


void
open_dir_config ()
{

	GtkWidget *dir;
	GtkWidget *states, *sram, *bram, *wav, *gym, *screenshot, *pat, *ips;
	GtkWidget *btnstates, *btnsram, *btnbram, *btnwav, *btngym,
		*btnscreenshot, *btnpat, *btnips;

	dir = create_directories_configuration ();
	gtk_window_set_transient_for (GTK_WINDOW(dir), GTK_WINDOW(gens_window));
	btnstates = lookup_widget (dir, "buttonStatesDir");
	btnsram = lookup_widget (dir, "buttonSramDir");
	btnbram = lookup_widget (dir, "buttonBramDir");
	btnwav = lookup_widget (dir, "buttonWavDir");
	btngym = lookup_widget (dir, "buttonGymDir");
	btnscreenshot = lookup_widget (dir, "buttonScreenshotDir");
	btnpat = lookup_widget (dir, "buttonPatDir");
	btnips = lookup_widget (dir, "buttonIpsDir");

	states = lookup_widget (dir, "statesDir");
	sram = lookup_widget (dir, "sramDir");
	bram = lookup_widget (dir, "bramDir");
	wav = lookup_widget (dir, "wavDir");
	gym = lookup_widget (dir, "gymDir");
	screenshot = lookup_widget (dir, "screenshotDir");
	pat = lookup_widget (dir, "patDir");
	ips = lookup_widget (dir, "ipsDir");

	gtk_entry_set_text (GTK_ENTRY (states), State_Dir);
	gtk_entry_set_text (GTK_ENTRY (sram), SRAM_Dir);
	gtk_entry_set_text (GTK_ENTRY (bram), BRAM_Dir);
	gtk_entry_set_text (GTK_ENTRY (wav), Dump_Dir);
	gtk_entry_set_text (GTK_ENTRY (gym), Dump_GYM_Dir);
	gtk_entry_set_text (GTK_ENTRY (screenshot), ScrShot_Dir);
	gtk_entry_set_text (GTK_ENTRY (pat), Patch_Dir);
	gtk_entry_set_text (GTK_ENTRY (ips), IPS_Dir);

	g_signal_connect (GTK_OBJECT (btnstates), "clicked",
			  G_CALLBACK (on_change_dir), states);
	g_signal_connect (GTK_OBJECT (btnsram), "clicked",
			  G_CALLBACK (on_change_dir), sram);
	g_signal_connect (GTK_OBJECT (btnbram), "clicked",
			  G_CALLBACK (on_change_dir), bram);
	g_signal_connect (GTK_OBJECT (btnwav), "clicked",
			  G_CALLBACK (on_change_dir), wav);
	g_signal_connect (GTK_OBJECT (btngym), "clicked",
			  G_CALLBACK (on_change_dir), gym);
	g_signal_connect (GTK_OBJECT (btnscreenshot), "clicked",
			  G_CALLBACK (on_change_dir), screenshot);
	g_signal_connect (GTK_OBJECT (btnpat), "clicked",
			  G_CALLBACK (on_change_dir), pat);
	g_signal_connect (GTK_OBJECT (btnips), "clicked",
			  G_CALLBACK (on_change_dir), ips);

	gtk_widget_show_all (dir);
}

void
addStateFilter(GtkWidget* widget)
{
	GtkFileFilter* filter;
	const char* gs = "*.[gG][sS]?";
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "State file");
	gtk_file_filter_add_pattern (filter, gs);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "All files");
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
}

void
addGymFilter(GtkWidget* widget)
{
	GtkFileFilter* filter;
	const char* gym = "*.[gG][yY][mM]";
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "GYM file");
	gtk_file_filter_add_pattern (filter, gym);
	//gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
}


void
addCfgFilter(GtkWidget* widget)
{
	GtkFileFilter* filter;
	const char* gs = "*.[cC][fF][gG]";
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "Gens Config file");
	gtk_file_filter_add_pattern (filter, gs);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
}

void
addIsoFilter(GtkWidget* widget)
{
	GtkFileFilter* filter;
// "SegaCD image file\0*.bin;*.iso;*.raw\0All files\0*.*\0\0"
	const char* bin = "*.[bb][iI][nN]";
	const char* iso = "*.[iI][sS][oO]";
	const char* chd = "*.[cC][hH][dD]";
	const char* raw = "*.[rR][aA][wW]";
	
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "SegaCD image file");
	gtk_file_filter_add_pattern (filter, bin);
	gtk_file_filter_add_pattern (filter, iso);
	gtk_file_filter_add_pattern (filter, chd);
	gtk_file_filter_add_pattern (filter, raw);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
}


void
addRomsFilter(GtkWidget* widget)
{
	GtkFileFilter* filter;
	const char* bin = "*.[bB][iI][nN]";
	const char* smd = "*.[sS][mM][dD]";
	const char* gen = "*.[gG][eE][nN]";
	const char* _32x = "*.32[xX]";
	const char* iso = "*.[iI][sS][oO]";
	const char* chd = "*.[cC][hH][dD]";
	const char* raw = "*.[rR][aA][wW]";
	const char* zip = "*.[zZ][iI][pP]";
	const char* gz ="*.[gG][zZ]";
	const char* zsg ="*.[zZ][sS][gG]";
	
	filter = gtk_file_filter_new ();
	
    gtk_file_filter_set_name (filter, "Sega CD / 32X / Genesis roms");
	gtk_file_filter_add_pattern (filter, bin);
	gtk_file_filter_add_pattern (filter, smd);
	gtk_file_filter_add_pattern (filter, gen);
	gtk_file_filter_add_pattern (filter, _32x);
	gtk_file_filter_add_pattern (filter, iso);
	gtk_file_filter_add_pattern (filter, chd);
	gtk_file_filter_add_pattern (filter, raw);
	gtk_file_filter_add_pattern (filter, zip);
	gtk_file_filter_add_pattern (filter, gz);
	gtk_file_filter_add_pattern (filter, zsg);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);

	filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "Genesis roms");
	gtk_file_filter_add_pattern (filter, bin);
	gtk_file_filter_add_pattern (filter, smd);
	gtk_file_filter_add_pattern (filter, gen);
	gtk_file_filter_add_pattern (filter, zip);
	gtk_file_filter_add_pattern (filter, gz);
	gtk_file_filter_add_pattern (filter, zsg);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);	

	filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "32X roms");
	gtk_file_filter_add_pattern (filter, zip);
	gtk_file_filter_add_pattern (filter, gz);
	gtk_file_filter_add_pattern (filter, _32x);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);

	filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "SegaCD image");
	gtk_file_filter_add_pattern (filter, iso);
	gtk_file_filter_add_pattern (filter, chd);
	gtk_file_filter_add_pattern (filter, raw);
	gtk_file_filter_add_pattern (filter, bin);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
	
	filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "All files");
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widget), filter);
}

GtkWidget*
create_file_chooser_dialog(const char* title, GtkFileChooserAction action)
{
	GtkWidget* widget;
	GdkPixbuf* icon;
	widget = gtk_file_chooser_dialog_new(title, NULL, action, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	icon = create_pixbuf ("Gens2.ico");
	gtk_window_set_icon(GTK_WINDOW(widget),icon);

	return widget;
}


void
open_bios_cfg ()
{
	GtkWidget *dir;
	GtkWidget *genesis, *m68000, *msh2, *ssh2, *usabios, *eurbios,
		*japbios, *cgoffline, *manual;
	GtkWidget *btngenesis, *btnm68000, *btnmsh2, *btnssh2, *btnusabios,
		*btneurbios, *btnjapbios, *btncgoffline, *btnmanual;

	dir = create_bios_files ();
	gtk_window_set_transient_for (GTK_WINDOW(dir),
								  GTK_WINDOW(gens_window));
															
	btngenesis = lookup_widget (dir, "buttonGenesisBios");
	btnm68000 = lookup_widget (dir, "buttonM68000");
	btnmsh2 = lookup_widget (dir, "buttonMSH2");
	btnssh2 = lookup_widget (dir, "buttonSSH2");
	btnusabios = lookup_widget (dir, "buttonUSABios");
	btneurbios = lookup_widget (dir, "buttonEURBios");
	btnjapbios = lookup_widget (dir, "buttonJAPBios");
	btncgoffline = lookup_widget (dir, "buttonCGOffline");
	btnmanual = lookup_widget (dir, "buttonManual");

	genesis = lookup_widget (dir, "genesisBios");
	m68000 = lookup_widget (dir, "M68000");
	msh2 = lookup_widget (dir, "MSH2");
	ssh2 = lookup_widget (dir, "SSH2");
	usabios = lookup_widget (dir, "USAbios");
	eurbios = lookup_widget (dir, "EURbios");
	japbios = lookup_widget (dir, "JAPbios");
	cgoffline = lookup_widget (dir, "CGOffline");
	manual = lookup_widget (dir, "manual");

	gtk_entry_set_text (GTK_ENTRY (genesis), Genesis_Bios);
	gtk_entry_set_text (GTK_ENTRY (m68000), _32X_Genesis_Bios);
	gtk_entry_set_text (GTK_ENTRY (msh2), _32X_Master_Bios);
	gtk_entry_set_text (GTK_ENTRY (ssh2), _32X_Slave_Bios);
	gtk_entry_set_text (GTK_ENTRY (usabios), US_CD_Bios);
	gtk_entry_set_text (GTK_ENTRY (eurbios), EU_CD_Bios);
	gtk_entry_set_text (GTK_ENTRY (japbios), JA_CD_Bios);
	gtk_entry_set_text (GTK_ENTRY (cgoffline), CGOffline_Path);
	gtk_entry_set_text (GTK_ENTRY (manual), Manual_Path);

	g_signal_connect (GTK_OBJECT (btngenesis), "clicked",
			  G_CALLBACK (on_change_file), genesis);
	g_signal_connect (GTK_OBJECT (btnm68000), "clicked",
			  G_CALLBACK (on_change_file), m68000);
	g_signal_connect (GTK_OBJECT (btnmsh2), "clicked",
			  G_CALLBACK (on_change_file), msh2);
	g_signal_connect (GTK_OBJECT (btnssh2), "clicked",
			  G_CALLBACK (on_change_file), ssh2);
	g_signal_connect (GTK_OBJECT (btnusabios), "clicked",
			  G_CALLBACK (on_change_file), usabios);
	g_signal_connect (GTK_OBJECT (btneurbios), "clicked",
			  G_CALLBACK (on_change_file), eurbios);
	g_signal_connect (GTK_OBJECT (btnjapbios), "clicked",
			  G_CALLBACK (on_change_file), japbios);
	g_signal_connect (GTK_OBJECT (btncgoffline), "clicked",
			  G_CALLBACK (on_change_file), cgoffline);
	g_signal_connect (GTK_OBJECT (btnmanual), "clicked",
			  G_CALLBACK (on_change_file), manual);

	gtk_widget_show_all (dir);
}

void
open_cdrom_select ()
{
	GtkWidget *dir;
	GtkWidget *CD_Drive;
	GtkWidget *btndrive;
	
	dir = create_cdrom_window ();
	gtk_window_set_transient_for (GTK_WINDOW(dir),
								  GTK_WINDOW(gens_window));
															
	btndrive = lookup_widget (dir, "button_cddrive");
	
	CD_Drive = lookup_widget (dir, "cd_drive");
	
	gtk_entry_set_text (GTK_ENTRY (CD_Drive), CDROM_DEV);
	
	g_signal_connect (GTK_OBJECT (btndrive), "clicked",
			  G_CALLBACK (on_change_file), CD_Drive);
	
	gtk_widget_show_all (dir);
}

int
Ritorna_Vel ()
{
	int valori[14] = {0,1,2,4,8,10,12,20,32,36,40,48,50,54};
	int i = 0;	

	while(i < 13)
	{
		if(valori[i] == CDROM_SPEED) return i;
		i++;
	}
	
	return 0;
}

void
open_msgbox(const char * msg)
{
  GtkWidget *dialog;
  GtkWidget *label;
  dialog = create_msgbox ();
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (gens_window));
  gtk_window_set_title (GTK_WINDOW (dialog), "Error");
  label = lookup_widget (dialog, "labelToDisplay");
  gtk_label_set_text (GTK_LABEL (label), msg);
  gtk_dialog_run (GTK_DIALOG (dialog));
}
