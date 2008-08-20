#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h> 
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "ui_proxy.h"
#include "gens.h"
#include "g_md.h"
#include "g_mcd.h"
#include "g_32x.h"
#include "g_sdlsound.h"
#include "g_sdldraw.h"
#include "g_main.h"
#include "z80.h"
#include "cd_aspi.h"
#include "save.h"
#include "sh2.h"
#include "wave.h"
#include "g_sdlinput.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "ggenie.h"
#include "mem_m68k.h"
#include "io.h"
#include "scrshot.h"
#include "gym.h"

void main68k_reset();
void sub68k_reset();

void
on_rom_history_activate				   (GtkMenuItem     *menuitem,
										gpointer         user_data)
{
//	if ((Check_If_Kaillera_Running())) return 0;
	if (GYM_Playing) Stop_Play_GYM();
	Open_Rom( Recent_Rom[GPOINTER_TO_INT(user_data)]);
	sync_gens_ui(UPDATE_GTK);
}

void
on_open_rom_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
//	if ((Check_If_Kaillera_Running())) return 0;
	if (GYM_Playing) Stop_Play_GYM();
	if (Get_Rom() != -1)
	{
		sync_gens_ui(UPDATE_GTK);
	}
	else {
		open_msgbox("Error opening ROM");
	}
}


void
on_close_rom_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
		if (Sound_Initialised) Clear_Sound_Buffer();
		Debug = 0;
		if (Net_Play)
		{
			if (Full_Screen) Set_Render( 0, -1, 1);
		}
		Free_Rom(Game);
}


void
on_boot_cd_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!Num_CD_Drive) return;
//	if (Check_If_Kaillera_Running()) return 0;
	if (GYM_Playing) Stop_Play_GYM();
	Free_Rom(Game);			// Don't forget it !
	SegaCD_Started = Init_SegaCD(NULL);
}


void
on_netplay_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{open_msgbox("Net Play unavailable");}

void
on_game_genie_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{open_game_genie();}


void
on_load_state_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	Str_Tmp[0] = 0;
	Change_File_L(Str_Tmp, State_Dir);
	Load_State(Str_Tmp);
}


void
on_save_state_as_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	Change_File_S(Str_Tmp, State_Dir);
	Save_State(Str_Tmp);
}


void
on_quick_load_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	Str_Tmp[0] = 0;
	Get_State_File_Name(Str_Tmp);
	Load_State(Str_Tmp);
}


void
on_quick_save_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	Str_Tmp[0] = 0;
	Get_State_File_Name(Str_Tmp);
	Save_State(Str_Tmp);
}


void
on_change_state_slot0_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 0);}


void
on_change_state_slot1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 1);}


void
on_change_state_slot2_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 2);}


void
on_change_state_slot3_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 3);}


void
on_change_state_slot4_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 4);}


void
on_change_state_slot5_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 5);}


void
on_change_state_slot6_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 6);}


void
on_change_state_slot7_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 7);}


void
on_change_state_slot8_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 8);}


void
on_change_state_slot9_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Current_State( 9);}


void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{close_gens();}

gboolean
on_gens_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	close_gens();
	return 0;
}

gboolean
on_gens_window_destroy_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	close_gens();
	return 0;
}

void
on_full_screen_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
//	if (Full_Screen) Set_Render( 0, -1, 1);
//	else Set_Render( 1, Render_FS, 1);
   if (do_callbacks)
   {
       Full_Screen = !Full_Screen;
       Set_Render(Full_Screen, Render_Mode, 0);
   }
}


void
on_vsync_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_VSync();}


void
on_stretch_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Stretch();}


void
on_color_adjust_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget* color_adj, *greyscale, *invert, *contrast, *brightness;

	color_adj = create_color_adjust();
	gtk_window_set_transient_for (GTK_WINDOW(color_adj),
															GTK_WINDOW(gens_window));
	greyscale = lookup_widget(color_adj, "greyscale");
	invert = lookup_widget(color_adj,"invert");
	contrast = lookup_widget(color_adj, "contrast");
	brightness = lookup_widget(color_adj, "brightness");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(greyscale), Greyscale);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(invert), Invert_Color);
	gtk_range_set_value(GTK_RANGE(contrast),Contrast_Level-100);
	gtk_range_set_value(GTK_RANGE(brightness),Brightness_Level-100);

	gtk_widget_show_all(color_adj);
}

void
on_normal_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, NORMAL, 0);}

void
on_double_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, DOUBLE, 0);}

void
on_interpolated_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, INTERPOLATED, 0);}

void
on_scanline_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, FULL_SCANLINE, 0);}

void
on_50_scanline_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, SCANLINE_50, 0);}

void
on_25_scanline_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, SCANLINE_25, 0);}

void
on_interpolated_scanline_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, INTERPOLATED_SCANLINE, 0);}

void
on_interpolated_50_scanline_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, INTERPOLATED_SCANLINE_50, 0);}

void
on_interpolated_25_scanline_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, INTERPOLATED_SCANLINE_25, 0);}

void
on_2xsai_kreed_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, KREED, 0);}

void
on_scale2x_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, SCALE2X, 0);}

void
on_hq2x_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{if(do_callbacks) Set_Render( Full_Screen, HQ2X, 0);}

void
on_sprite_limit_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Sprite_Over( Sprite_Over ^ 1);}

void
on_frame_auto_skip_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( -1);}

void
on_frame_0_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 0);}

void
on_frame_1_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 1);}

void
on_frame_2_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 2);}

void
on_frame_3_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 3);}

void
on_frame_4_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 4);}

void
on_frame_5_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 5);}

void
on_frame_6_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 6);}

void
on_frame_7_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 7);}

void
on_frame_8_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Set_Frame_Skip( 8);}

void
on_screen_shot_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	Clear_Sound_Buffer();
	Take_Shot();
}

void
on_auto_detect_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country( -1);}

void
on_japan_ntsc_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country( 0);}

void
on_usa_ntsc_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country( 1);}

void
on_europe_pal_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country( 2);}

void
on_japan_pal_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country( 3);}

void
on_detection_order_usa_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country_Order(0);}

void
on_detection_order_japan_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country_Order(1);}

void
on_detection_order_europe_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Country_Order(2);}

void
on_hard_reset_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;

	if (Genesis_Started)
	{
		Reset_Genesis();
		MESSAGE_L("Genesis reseted", "Genesis reseted", 1500)
	}
	else if (_32X_Started)
	{
		Reset_32X();
		MESSAGE_L("32X reseted", "32X reseted", 1500)
	}
	else if (SegaCD_Started)
	{
		Reset_SegaCD();
		MESSAGE_L("SegaCD reseted", "SegaCD reseted", 1500)
	}
}

void
on_reset_68000_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	if (Game)
	{
		Paused = 0;
		main68k_reset();
		if (Genesis_Started) {
		MESSAGE_L("68000 CPU reseted", "68000 CPU reseted", 1000)
		}
		else if (SegaCD_Started) {
		MESSAGE_L("Main 68000 CPU reseted", "Main 68000 CPU reseted", 1000);
		}
	}
}

void
on_reset_z80_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	if (Game)
	{
		z80_Reset(&M_Z80);
		MESSAGE_L("CPU Z80 reseted", "CPU Z80 reseted", 1000)
	}
}

void
on_perfect_synchro_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_SegaCD_Synchro();}

void
on_enable_sound_activate                     (GtkMenuItem     *menuitem,
											  gpointer         user_data)
{
    // Function modified by Josh Mattila
    // Reason: Bug fixes
    if (Change_Sound ())
    {
        // Update the GUI as necessary so
        // that it doesn't get out of sync
        // with the actual program variables.

        // UPDATE_GTK_SOUNDOFF = 1
        // UPDATE_GTK_SOUNDON = 2
        // #defined in g_main.h

        if (Sound_Enable)
        {
            sync_gens_ui (UPDATE_GTK_SOUNDOFF);
        }
        if (!Sound_Enable)
        {
            sync_gens_ui (UPDATE_GTK_SOUNDON);
        }

    }
}

void
on_rate_11025_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sample_Rate( 0);}

void
on_rate_22050_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sample_Rate( 1);}

void
on_rate_44100_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sample_Rate( 2);}

void
on_rate_16000_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sample_Rate( 3);}

void
on_rate_32000_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sample_Rate( 4);}

void
on_rate_48000_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sample_Rate( 5);}

void
on_stereo_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Sound_Stereo();}

void
on_z80_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Z80();}

void
on_psg_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_PSG();}

void
on_psg_improved_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_PSG_Improv();}

void
on_dac_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_DAC();}

void
on_pcm_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_PCM();}

void
on_pwm_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_PWM();}

void
on_ym2612_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_YM2612();}

void
on_ym2612_improved_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_YM2612_Improv();}

void
on_genesis_68000_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 1);}

void
on_genesis_z80_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 2);}

void
on_genesis_vdp_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 3);}

void
on_segacd_68000_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 4);}

void
on_segacd_cdc_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 5);}

void
on_segacd_gfx_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 6);}

void
on_32x_main_sh2_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 7);}

void
on_32x_sub_sh2_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 8);}

void
on_32x_vdp_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_Debug( 9);}

void
on_reset_main_68000_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	if (Game)
	{
		Paused = 0;
		main68k_reset();
		if (Genesis_Started) MESSAGE_L("68000 CPU reseted", "68000 CPU reseted", 1000)
		else if (SegaCD_Started) MESSAGE_L("Main 68000 CPU reseted", "Main 68000 CPU reseted", 1000)
	}
}

void
on_reset_sub_68000_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	if ((Game) && (SegaCD_Started))
	{
		Paused = 0;
		sub68k_reset();
		MESSAGE_L("Sub 68000 CPU reseted", "Sub 68000 CPU reseted", 1000)
	}
}

void
on_reset_main_sh2_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	if ((Game) && (_32X_Started))
	{
		Paused = 0;
		SH2_Reset(&M_SH2, 1);
		MESSAGE_L("Master SH2 reseted", "Master SH2 reseted", 1000)
	}
}

void
on_reset_sub_sh2_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	if ((Game) && (_32X_Started))
	{
		Paused = 0;
		SH2_Reset(&S_SH2, 1);
		MESSAGE_L("Slave SH2 reseted", "Slave SH2 reseted", 1000)
	}
}

void
on_dac_improved_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_DAC_Improv();}

void
on_cdda_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_CDDA();}

void
on_start_wav_dump_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gchar* label;
	if (WAV_Dumping)
	{
		Stop_WAV_Dump();
		label = "Start WAV Dump";
	} else {
		Start_WAV_Dump();
		label = "Stop WAV Dump";
	}
	gtk_label_set_text(GTK_LABEL(GTK_BIN(menuitem)->child), label);
}

void
on_start_gym_dump_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gchar* label;
	if (GYM_Dumping)
	{
		Stop_GYM_Dump();
		label = "Start GYM Dump";
	} else {
		Start_GYM_Dump();
		label = "Stop GYM Dump";
	}
	gtk_label_set_text(GTK_LABEL(GTK_BIN(menuitem)->child), label);
}

void
on_misc_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget* misc;

	GtkWidget* autofixcs, *fastblur, *autopause, *showled;
	GtkWidget* showfps, *fps_transparency, *fps_x2;
	GtkWidget* showmsg, *msg_transparency, *msg_x2;
	GtkWidget* fpsWhite, *fpsBlue, *fpsGreen, *fpsRed;
	GtkWidget* msgWhite, *msgBlue, *msgGreen, *msgRed;
	GtkWidget* introBlack, *introBlue, *introGreen, *introLightBlue, *introRed, *introPink, *introYellow, *introWhite;
	
	misc = create_general_option();
	gtk_window_set_transient_for (GTK_WINDOW(misc),
															GTK_WINDOW(gens_window));

	autofixcs = lookup_widget(misc, "autoFixChecksum");
	fastblur = lookup_widget(misc, "fastBlur");
	autopause = lookup_widget(misc, "autoPause");
	showled = lookup_widget(misc, "showCDLED");
	
	showfps = lookup_widget(misc, "fpsEnable");
	fps_transparency = lookup_widget(misc, "fpsTransparency");
	fps_x2 = lookup_widget(misc, "fpsDoubleSized");
	
	fpsWhite = lookup_widget(misc, "fpsWhite");
	fpsBlue = lookup_widget(misc, "fpsBlue");
	fpsGreen = lookup_widget(misc, "fpsGreen");
	fpsRed = lookup_widget(misc, "fpsRed");
	
	showmsg = lookup_widget(misc, "messageEnable");
	msg_transparency = lookup_widget(misc, "messageTransparency");
	msg_x2 = lookup_widget(misc, "messageDoubleSized");
	
	msgWhite = lookup_widget(misc, "messageWhite");
	msgBlue = lookup_widget(misc, "messageBlue");
	msgGreen = lookup_widget(misc, "messageGreen");
	msgRed = lookup_widget(misc, "messageRed");
	
	introBlack = lookup_widget(misc, "miscBlack");
	introBlue = lookup_widget(misc, "miscBlue");
	introGreen = lookup_widget(misc, "miscGreen");
	introLightBlue = lookup_widget(misc, "miscLightBlue");
	introRed = lookup_widget(misc, "miscRed");
	introPink = lookup_widget(misc, "miscPink");
	introYellow = lookup_widget(misc, "miscYellow");
	introWhite = lookup_widget(misc, "miscWhite");
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autofixcs),Auto_Fix_CS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fastblur),Fast_Blur);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autopause),Auto_Pause);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(showled),Show_LED);	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(showfps),Show_FPS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(showmsg),Show_Message);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fps_transparency),(FPS_Style & 0x8));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(msg_transparency),(Message_Style & 0x8));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fps_x2),(FPS_Style & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(msg_x2),(Message_Style & 0x10));
	
	switch ((FPS_Style & 0x6)>>1)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fpsWhite),1);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fpsBlue),1);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fpsGreen),1);
			break;
		case 3:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fpsRed),1);
			break;
	}

	switch ((Message_Style & 0x6)>>1)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(msgWhite),1);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(msgBlue),1);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(msgGreen),1);
			break;
		case 3:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(msgRed),1);
			break;
	}
		
	switch (Effect_Color)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introBlack),1);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introBlue),1);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introGreen),1);
			break;
		case 3:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introLightBlue),1);
			break;
		case 4:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introRed),1);
			break;
		case 5:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introPink),1);
			break;
		case 6:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introYellow),1);
			break;
		case 7:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(introWhite),1);
			break;
	}

	gtk_widget_show_all(misc);
}

void
on_joypads_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{open_joypads();}

void
on_directories_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{open_dir_config();}

void
on_bios_misc_files_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{open_bios_cfg();}

void
on_item10_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{open_cdrom_select();}

void
on_none_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_SegaCD_SRAM_Size(-1);}

void
on_8_kb_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_SegaCD_SRAM_Size(0);}

void
on_16_kb_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_SegaCD_SRAM_Size(1);}

void
on_32_kb_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_SegaCD_SRAM_Size(2);}

void
on_64_kb_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Change_SegaCD_SRAM_Size(3);}

void
on_load_config_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	//if (Check_If_Kaillera_Running()) return 0;
	Load_As_Config( Game);
	sync_gens_ui(UPDATE_GTK);
}

void
on_save_config_as_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{Save_As_Config();}

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget* about;
	Clear_Sound_Buffer();
	about = create_about();
	gtk_window_set_transient_for (GTK_WINDOW(about),GTK_WINDOW(gens_window));
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}

void
on_buttonGGAddCode_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *codewdg;
	GtkWidget* namewdg;
	gchar* code;
	gchar* name;

	codewdg = lookup_widget(GTK_WIDGET(button), "code");
	namewdg = lookup_widget(GTK_WIDGET(button), "name");

	code = strdup(gtk_entry_get_text(GTK_ENTRY(codewdg)));
	name = strdup(gtk_entry_get_text(GTK_ENTRY(namewdg)));
	
	if ((strlen(code) == 9) || (strlen(code) == 11))
	{						
		addCode(NULL,name, code, 0);	
	}
	
	g_free(code);
	g_free(name);
}

void
on_buttonGGDelete_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* treeview;
	GtkTreeSelection* selection;
	GtkTreeIter iter;
	gboolean valid;
	gboolean row_erased;
	gboolean need_check;

	treeview = lookup_widget(GTK_WIDGET(button), "ggListCode");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));		
	
	need_check = TRUE;
	while(need_check)
	{
		row_erased = FALSE;
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel), &iter);
		while (valid && !row_erased)
		{
			if (gtk_tree_selection_iter_is_selected(selection, &iter))
			{
				gtk_list_store_remove(listmodel, &iter);
				row_erased = TRUE;
			}
			else
				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel),&iter);
		}
		if (!valid && !row_erased)
			need_check = FALSE;
	}
}

void
on_buttonGGDesactivate_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* treeview;
	GtkTreeSelection* select;

	treeview = lookup_widget(GTK_WIDGET(button), "ggListCode");
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_unselect_all(select);
}

void
on_buttonGGOK_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	int i;
	gboolean valid;
	GtkTreeIter iter;
	gchar* code;
	gchar* name;
	GtkWidget* treeview;
	GtkTreeSelection* selection;
	GtkWidget* ggenie;

	treeview = lookup_widget(GTK_WIDGET(button), "ggListCode");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));		

	for(i = 0; i < 256; i++)
	{
		Liste_GG[i].code[0] = 0;
		Liste_GG[i].name[0] = 0;
		Liste_GG[i].active = 0;
		Liste_GG[i].addr = 0xFFFFFFFF;
		Liste_GG[i].data = 0;
		Liste_GG[i].restore = 0xFFFFFFFF;
	}
	
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel), &iter);
	i=0;
	while (valid && i<256)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(listmodel), &iter, 0, &name, 1, &code, -1);
		strcpy(Liste_GG[i].name, name);
		strcpy(Liste_GG[i].code, code);
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
			Liste_GG[i].active = 1;
		else
			Liste_GG[i].active = 0;
		fprintf(stderr, "%s active : %d\n", Liste_GG[i].name, Liste_GG[i].active);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel),&iter);
		i++;
	}
	
	for(i = 0; i < 256; i++)
	{
		if ((Liste_GG[i].code[0] != 0) && (Liste_GG[i].addr == 0xFFFFFFFF) && (Liste_GG[i].data == 0))
		{
			decode(Liste_GG[i].code, (struct patch *) (&(Liste_GG[i].addr)));

			if ((Liste_GG[i].restore == 0xFFFFFFFF) && (Liste_GG[i].addr < Rom_Size) && (Genesis_Started))
			{
				Liste_GG[i].restore = (unsigned int) (Rom_Data[Liste_GG[i].addr] & 0xFF);
				Liste_GG[i].restore += (unsigned int) ((Rom_Data[Liste_GG[i].addr + 1] & 0xFF) << 8);
			}
		}
	}
	ggenie = lookup_widget(GTK_WIDGET(button), "game_genie");
	gtk_widget_destroy(ggenie);
}

void
on_buttonColorClose_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* color;
	color = lookup_widget(GTK_WIDGET(button), "color_adjust");
	gtk_widget_destroy(color);
}

void
on_buttonColorApply_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *greyscale, *invert, *contrast, *brightness;

	greyscale = lookup_widget(GTK_WIDGET(button), "greyscale");
	invert = lookup_widget(GTK_WIDGET(button),"invert");
	contrast = lookup_widget(GTK_WIDGET(button), "contrast");
	brightness = lookup_widget(GTK_WIDGET(button), "brightness");

	Greyscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(greyscale));
	Invert_Color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(invert));
	Contrast_Level = gtk_range_get_value(GTK_RANGE(contrast)) + 100;
	Brightness_Level = gtk_range_get_value(GTK_RANGE(brightness)) + 100;

	Recalculate_Palettes();
	if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
		CRam_Flag = 1;
		if (!Paused) Update_Emulation_One();
	}
}

void
on_buttonColorDefault_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *greyscale, *invert, *contrast, *brightness;

	greyscale = lookup_widget(GTK_WIDGET(button), "greyscale");
	invert = lookup_widget(GTK_WIDGET(button),"invert");
	contrast = lookup_widget(GTK_WIDGET(button), "contrast");
	brightness = lookup_widget(GTK_WIDGET(button), "brightness");

	Greyscale = 0;
	Invert_Color = 0;
	Contrast_Level = 100;
	Brightness_Level = 100;	

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(greyscale), Greyscale);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(invert), Invert_Color);
	gtk_range_set_value(GTK_RANGE(contrast), Contrast_Level-100);
	gtk_range_set_value(GTK_RANGE(brightness),Brightness_Level-100);
}

void
on_buttonOptionOK_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* autofixcs, *fastblur, *autopause, *showled;
	GtkWidget* showfps, *fps_transparency, *fps_x2;
	GtkWidget* showmsg, *msg_transparency, *msg_x2;
	GtkWidget* fpsWhite, *fpsBlue, *fpsGreen, *fpsRed;
	GtkWidget* msgWhite, *msgBlue, *msgGreen, *msgRed;
	GtkWidget* introBlack, *introBlue, *introGreen, *introLightBlue, *introRed, *introPink, *introYellow, *introWhite;
	GtkWidget* option;

	gint res = 0;
	
	autofixcs = lookup_widget(GTK_WIDGET(button), "autoFixChecksum");
	fastblur = lookup_widget(GTK_WIDGET(button), "fastBlur");
	autopause = lookup_widget(GTK_WIDGET(button), "autoPause");
	showled = lookup_widget(GTK_WIDGET(button), "showCDLED");
	
	showfps = lookup_widget(GTK_WIDGET(button), "fpsEnable");
	fps_transparency = lookup_widget(GTK_WIDGET(button), "fpsTransparency");
	fps_x2 = lookup_widget(GTK_WIDGET(button), "fpsDoubleSized");
	
	fpsWhite = lookup_widget(GTK_WIDGET(button), "fpsWhite");
	fpsBlue = lookup_widget(GTK_WIDGET(button), "fpsBlue");
	fpsGreen = lookup_widget(GTK_WIDGET(button), "fpsGreen");
	fpsRed = lookup_widget(GTK_WIDGET(button), "fpsRed");
	
	showmsg = lookup_widget(GTK_WIDGET(button), "messageEnable");
	msg_transparency = lookup_widget(GTK_WIDGET(button), "messageTransparency");
	msg_x2 = lookup_widget(GTK_WIDGET(button), "messageDoubleSized");
	
	msgWhite = lookup_widget(GTK_WIDGET(button), "messageWhite");
	msgBlue = lookup_widget(GTK_WIDGET(button), "messageBlue");
	msgGreen = lookup_widget(GTK_WIDGET(button), "messageGreen");
	msgRed = lookup_widget(GTK_WIDGET(button), "messageRed");
	
	introBlack = lookup_widget(GTK_WIDGET(button), "miscBlack");
	introBlue = lookup_widget(GTK_WIDGET(button), "miscBlue");
	introGreen = lookup_widget(GTK_WIDGET(button), "miscGreen");
	introLightBlue = lookup_widget(GTK_WIDGET(button), "miscLightBlue");
	introRed = lookup_widget(GTK_WIDGET(button), "miscRed");
	introPink = lookup_widget(GTK_WIDGET(button), "miscPink");
	introYellow = lookup_widget(GTK_WIDGET(button), "miscYellow");
	introWhite = lookup_widget(GTK_WIDGET(button), "miscWhite");
	
	Auto_Fix_CS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autofixcs));
	Fast_Blur = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fastblur));
	Auto_Pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autopause));
	Show_LED = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(showled));
	
	Show_FPS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(showfps));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fpsWhite))) res = 0;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fpsBlue))) res = 1;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fpsGreen))) res = 2;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fpsRed))) res = 3;
	FPS_Style = (FPS_Style & ~0x6) | ((res << 1) & 0x6);
	FPS_Style = (FPS_Style & ~0x8) | ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fps_transparency)))?0x8:0);
	FPS_Style = (FPS_Style & ~0x10) | ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fps_x2)))?0x10:0);
	
	Show_Message = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(showmsg));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(msgWhite))) res = 0;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(msgBlue))) res = 1;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(msgGreen))) res = 2;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(msgRed))) res = 3;
	Message_Style = (Message_Style & 0xF9) | ((res << 1) & 0x6);
	Message_Style = (Message_Style & ~0x8) | ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(msg_transparency)))?0x8:0);
	Message_Style = (Message_Style & ~0x10) | ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(msg_x2)))?0x10:0);
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introBlack))) Effect_Color = 0;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introBlue))) Effect_Color = 1;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introGreen))) Effect_Color = 2;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introLightBlue))) Effect_Color = 3;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introRed))) Effect_Color = 4;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introPink))) Effect_Color = 5;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introYellow))) Effect_Color = 6;
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(introWhite))) Effect_Color = 7;

	option = lookup_widget(GTK_WIDGET(button), "general_option");
	gtk_widget_destroy(option);
}

void
on_buttonControlSetOK_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* port1, *port2;
	GtkWidget* padp1, * padp1b, * padp1c, * padp1d;
	GtkWidget* padp2, * padp2b, * padp2c, * padp2d;
	GtkWidget* window;

	port1 = lookup_widget(GTK_WIDGET(button), "port1");
	port2 = lookup_widget(GTK_WIDGET(button), "port2");

	padp1 = lookup_widget(GTK_WIDGET(button), "padp1");
	padp1b = lookup_widget(GTK_WIDGET(button), "padp1b");
	padp1c = lookup_widget(GTK_WIDGET(button), "padp1c");
	padp1d = lookup_widget(GTK_WIDGET(button), "padp1d");
	padp2 = lookup_widget(GTK_WIDGET(button), "padp2");
	padp2b = lookup_widget(GTK_WIDGET(button), "padp2b");
	padp2c = lookup_widget(GTK_WIDGET(button), "padp2c");
	padp2d = lookup_widget(GTK_WIDGET(button), "padp2d");

	Controller_1_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(port1));
	Controller_1_Type <<= 4;

	Controller_1_Type |= gtk_option_menu_get_history(GTK_OPTION_MENU(padp1)) ;
	Controller_1B_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(padp1b)) ;
	Controller_1C_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(padp1c)) ;
	Controller_1D_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(padp1d)) ;

	Controller_2_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(port2)) ;
	Controller_2_Type <<= 4;

	Controller_2_Type |= gtk_option_menu_get_history(GTK_OPTION_MENU(padp2)) ;
	Controller_2B_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(padp2b)) ;
	Controller_2C_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(padp2c)) ;
	Controller_2D_Type = gtk_option_menu_get_history(GTK_OPTION_MENU(padp2d)) ;
	
	Make_IO_Table();
	End_Input();
	window = lookup_widget(GTK_WIDGET(button), "controllers_settings");
	gtk_widget_destroy(window);
}

void
on_buttonDirOK_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* states, *sram, *bram, *wav, *gym, *screenshot, *pat, *ips;
	GtkWidget* dircfg;
	gchar* txt;

	states = lookup_widget(GTK_WIDGET(button), "statesDir");
	sram = lookup_widget(GTK_WIDGET(button), "sramDir");
	bram = lookup_widget(GTK_WIDGET(button), "bramDir");
	wav = lookup_widget(GTK_WIDGET(button), "wavDir");
	gym = lookup_widget(GTK_WIDGET(button), "gymDir");
	screenshot = lookup_widget(GTK_WIDGET(button), "screenshotDir");
	pat = lookup_widget(GTK_WIDGET(button), "patDir");
	ips = lookup_widget(GTK_WIDGET(button), "ipsDir");
			
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(states));
	if (strlen(txt))
	{
		strcpy(State_Dir, txt);
		strcat(State_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(sram));
	if (strlen(txt))
	{
		strcpy(SRAM_Dir, txt);
		strcat(SRAM_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(bram));
	if (strlen(txt))
	{
		strcpy(BRAM_Dir, txt);
		strcat(BRAM_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(wav));
	if (strlen(txt))
	{
		strcpy(Dump_Dir, txt);
		strcat(Dump_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(gym));
	if (strlen(txt))
	{
		strcpy(Dump_GYM_Dir, txt);
		strcat(Dump_GYM_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(screenshot));
	if (strlen(txt))
	{
		strcpy(ScrShot_Dir, txt);
		strcat(ScrShot_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(pat));
	if (strlen(txt))
	{
		strcpy(Patch_Dir, txt);
		strcat(Patch_Dir, "/");
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(ips));
	if (strlen(txt))
	{
		strcpy(IPS_Dir, txt);
		strcat(IPS_Dir, "/");
	}
	
	dircfg = lookup_widget(GTK_WIDGET(button), "directories_configuration");
	gtk_widget_destroy(dircfg);
}

void
on_buttonCfgBios_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* genesis, *m68000, *msh2, *ssh2, *usabios, *eurbios, *japbios, *cgoffline, *manual;
	GtkWidget* dircfg;
	gchar* txt;

	genesis = lookup_widget(GTK_WIDGET(button), "genesisBios");
	m68000 = lookup_widget(GTK_WIDGET(button), "M68000");
	msh2 = lookup_widget(GTK_WIDGET(button), "MSH2");
	ssh2= lookup_widget(GTK_WIDGET(button), "SSH2");
	usabios = lookup_widget(GTK_WIDGET(button), "USAbios");
	eurbios = lookup_widget(GTK_WIDGET(button), "EURbios");
	japbios = lookup_widget(GTK_WIDGET(button), "JAPbios");
	cgoffline = lookup_widget(GTK_WIDGET(button), "CGOffline");
	manual = lookup_widget(GTK_WIDGET(button), "manual");

	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(genesis));
	if (strlen(txt))
	{
		strcpy(Genesis_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(m68000));
	if (strlen(txt))
	{
		strcpy(_32X_Genesis_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(msh2));
	if (strlen(txt))
	{
		strcpy(_32X_Master_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(ssh2));
	if (strlen(txt))
	{
		strcpy(_32X_Slave_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(usabios));
	if (strlen(txt))
	{
		strcpy(US_CD_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(eurbios));
	if (strlen(txt))
	{
		strcpy(EU_CD_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(japbios));
	if (strlen(txt))
	{
		strcpy(JA_CD_Bios, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(cgoffline));
	if (strlen(txt))
	{
		strcpy(CGOffline_Path, txt);
	}
	
	txt = (gchar*)gtk_entry_get_text(GTK_ENTRY(manual));
	if (strlen(txt))
	{
		strcpy(Manual_Path, txt);
	}
	
	dircfg = lookup_widget(GTK_WIDGET(button), "bios_files");
	gtk_widget_destroy(dircfg);
}

void
on_opengl_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       if(do_callbacks)
       {
               Change_backend();
       }
}

void
on_320x240_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       Set_gl_resolution(320,240);
       MESSAGE_L("Selected 320x240 Resolution", "Selected 320x240 Resolution", 1500)
}


void
on_640x480_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       Set_gl_resolution(640,480);
       MESSAGE_L("Selected 640x480 Resolution", "Selected 640x480 Resolution", 1500)
}


void
on_800x600_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       Set_gl_resolution(800,600);
       MESSAGE_L("Selected 800x600 Resolution", "Selected 800x600 Resolution", 1500)
}


void
on_1024x768_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       Set_gl_resolution(1024,768);
       MESSAGE_L("Selected 1024x768 Resolution", "Selected 1024x768 Resolution", 1500)
}

void
on_custom_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       if(do_callbacks)
       {       
	       GtkWidget* gl_cust_res;       
	       gl_cust_res=create_opengl_resolution_custom();       
	       gtk_widget_show(gl_cust_res);       
       }
}

void
on_glres_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
       GtkWidget* gl_cust_res, *spin_width, *spin_height;
       
       gl_cust_res = lookup_widget(GTK_WIDGET(button), "opengl_resolution_custom");
       spin_width  = lookup_widget(GTK_WIDGET(button), "spinbutton4");       
       spin_height = lookup_widget(GTK_WIDGET(button), "spinbutton5");       
       Set_gl_resolution(gtk_spin_button_get_value(GTK_SPIN_BUTTON (spin_width)),gtk_spin_button_get_value(GTK_SPIN_BUTTON (spin_height) )     );       
       gtk_widget_destroy(gl_cust_res);       
}

void
on_bpp16_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       if(do_callbacks)
       {
               if(Opengl)
               {
                       Set_Bpp(16);
                       MESSAGE_L("Selected 16bits Depth", "Selected 16bits Depth", 1500)
               }       
       }
}

void
on_bpp24_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       if(do_callbacks)
       {
               if(Opengl)
               {
                       Set_Bpp(24);
                       MESSAGE_L("Selected 24bits Depth", "Selected 24bits Depth", 1500)
               }
       }
}

void
on_bpp32_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
       if(do_callbacks)
       {
               if(Opengl)
               {
                       Set_Bpp(32);
                       MESSAGE_L("Selected 32bits Depth", "Selected 32bits Depth", 1500)
               }
       }
}
void
on_opengl_resolution_custom_show       (GtkWidget       *widget,
                                        gpointer         user_data)
{
       GtkSpinButton* spin_width, *spin_height;       
       spin_width  = GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(widget), "spinbutton4"));       
       spin_height = GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(widget), "spinbutton5"));       
       gtk_spin_button_set_value(spin_width,Width_gl);       
       gtk_spin_button_set_value(spin_height,Height_gl);
       
}

void
on_buttonokcd_clicked			(GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *bottone, *cdrw, *casella, *comboscatola;
	gchar* testo;
	int k;
	int vettore[14] = {0,1,2,4,8,10,12,20,32,36,40,48,50,54};

	bottone = lookup_widget(GTK_WIDGET(button), "button59");
	casella = lookup_widget(GTK_WIDGET(button), "cd_drive");
	comboscatola = lookup_widget(GTK_WIDGET(button), "comboboxentry1");
	testo = strdup(gtk_entry_get_text(GTK_ENTRY(casella)));
	if (strlen(testo))
	{
		strcpy(CDROM_DEV, testo);
	}
	k = gtk_combo_box_get_active (comboscatola);
	CDROM_SPEED = vettore[k];

	cdrw = lookup_widget(GTK_WIDGET(button), "cdrom_window");
	gtk_widget_destroy(cdrw);
}

void
on_sdlok_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	// Function created by Josh Mattila
	// Reason: GTK callback function for the "OK" button
	// Parent: sdl_test window
	
	// This callback function closes the window, no settings are saved
	
	GtkWidget *sdl_window;
	sdl_window = lookup_widget (GTK_WIDGET(button), "sdl_test");
	close_sdltest(sdl_window);
}


void
on_RunTest_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	// Function created by Josh Mattila
	// Reason: GTK callback function for the "Run Test" button
	// Parent: sdl_test
	
	// This callback function runs the SDL sound test in support.c

	
	GtkWidget *sdl_window;
	sdl_window = lookup_widget (GTK_WIDGET(button), "sdl_test");
	
	if(run_sdltest(sdl_window) == 0)
	{
		//maybe make a window popup here someday
	}
	else
	{
		
	}
}


void
on_sdl_test_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// Function created by Josh Mattila
	// Reason: GTK callback function for the "SDL Sound Test" menu item
	// Parent: gens_window
	
	// This callback function opens the sdl_test window
	
	open_sdltest();
}
