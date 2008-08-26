/**
 * GENS: (GTK+) General Options Window - Miscellaneous Functions.
 */


#include <string.h>

#include "general_options_window.h"
#include "general_options_window_callbacks.h"
#include "general_options_window_misc.h"
#include "gens/gens_window.h"

#include "g_main.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"


// Includes containing variables required for this dialog.
#include "g_sdldraw.h"


/**
 * Open_General_Options(): Opens the General Options window.
 */
void Open_General_Options(void)
{
	GtkWidget *go;
	GtkWidget *check_system_autofixchecksum, *check_system_autopause;
	GtkWidget *check_system_fastblur, *check_system_segacd_leds;
	GtkWidget *check_fps_enable, *check_fps_doublesized;
	GtkWidget *check_fps_transparency, *radio_button_fps_color;
	GtkWidget *check_message_enable, *check_message_doublesized;
	GtkWidget *check_message_transparency, *radio_button_message_color;
	GtkWidget *radio_button_intro_effect_color;
	char tmp[64];
	
	go = create_general_options_window();
	if (!go)
	{
		// Either an error occurred while creating the General Options window,
		// or the General Options window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(go), GTK_WINDOW(gens_window));
	
	// Get the current options.
	
	// System
	check_system_autofixchecksum = lookup_widget(go, "check_system_autofixchecksum");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_autofixchecksum), Auto_Fix_CS);
	check_system_autopause = lookup_widget(go, "check_system_autopause");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_autopause), Auto_Pause);
	check_system_fastblur = lookup_widget(go, "check_system_fastblur");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_fastblur), Fast_Blur);
	check_system_segacd_leds = lookup_widget(go, "check_system_segacd_leds");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_system_segacd_leds), Show_LED);
	
	// FPS counter
	check_fps_enable = lookup_widget(go, "check_fps_enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fps_enable), Show_FPS);
	check_fps_doublesized = lookup_widget(go, "check_fps_doublesized");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fps_doublesized), (FPS_Style & 0x10));
	check_fps_transparency = lookup_widget(go, "check_fps_transparency");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fps_transparency), (FPS_Style & 0x08));
	
	// FPS counter color
	sprintf(tmp, "radio_button_fps_color_%s", GO_MsgColors[((FPS_Style & 0x06) >> 1) * 3]);
	radio_button_fps_color = lookup_widget(go, tmp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button_fps_color), TRUE);
	
	// Message
	check_message_enable = lookup_widget(go, "check_message_enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_message_enable), Show_Message);
	check_message_doublesized = lookup_widget(go, "check_message_doublesized");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_message_doublesized), (Message_Style & 0x10));
	check_message_transparency = lookup_widget(go, "check_message_transparency");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_message_transparency), (Message_Style & 0x08));
	
	// Message color
	sprintf(tmp, "radio_button_message_color_%s", GO_MsgColors[((Message_Style & 0x06) >> 1) * 3]);
	radio_button_message_color = lookup_widget(go, tmp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button_message_color), TRUE);
	
	// Intro effect color
	sprintf(tmp, "radio_button_misc_intro_effect_color_%s",
		GO_IntroEffectColors[Effect_Color * 3]);
	radio_button_intro_effect_color = lookup_widget(go, tmp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_button_intro_effect_color), TRUE);
	
	// Show the General Options window.
	gtk_widget_show_all(go);
}


/**
 * General_Options_Save(): Save the General Options.
 */
void General_Options_Save(void)
{
	GtkWidget *check_system_autofixchecksum, *check_system_autopause;
	GtkWidget *check_system_fastblur, *check_system_segacd_leds;
	GtkWidget *check_fps_enable, *check_fps_doublesized;
	GtkWidget *check_fps_transparency, *radio_button_fps_color;
	GtkWidget *check_message_enable, *check_message_doublesized;
	GtkWidget *check_message_transparency, *radio_button_message_color;
	GtkWidget *radio_button_intro_effect_color;
	char tmp[64]; short i;
	
	// Save the current options.
	
	// System
	check_system_autofixchecksum = lookup_widget(general_options_window, "check_system_autofixchecksum");
	Auto_Fix_CS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_autofixchecksum));
	check_system_autopause = lookup_widget(general_options_window, "check_system_autopause");
	Auto_Pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_autopause));
	check_system_fastblur = lookup_widget(general_options_window, "check_system_fastblur");
	Fast_Blur = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_fastblur));
	check_system_segacd_leds = lookup_widget(general_options_window, "check_system_segacd_leds");
	Show_LED = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_segacd_leds));
	
	// FPS counter
	check_fps_enable = lookup_widget(general_options_window, "check_fps_enable");
	Show_FPS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_enable));
	FPS_Style &= ~0x18;
	check_fps_doublesized = lookup_widget(general_options_window, "check_fps_doublesized");
	FPS_Style |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_doublesized)) ? 0x10 : 0x00);
	check_fps_transparency = lookup_widget(general_options_window, "check_fps_transparency");
	FPS_Style |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_transparency)) ? 0x08 : 0x00);
	
	// FPS counter color
	for (i = 0; i < 4; i++)
	{
		if (!GO_MsgColors[i])
			break;
		sprintf(tmp, "radio_button_fps_color_%s", GO_MsgColors[i * 3]);
		radio_button_fps_color = lookup_widget(general_options_window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_fps_color)))
		{
			FPS_Style &= ~0x06;
			FPS_Style |= (i << 1);
		}
	}
	
	// Message
	check_message_enable = lookup_widget(general_options_window, "check_message_enable");
	Show_Message = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_enable));
	Message_Style &= ~0x18;
	check_message_doublesized = lookup_widget(general_options_window, "check_message_doublesized");
	Message_Style |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_doublesized)) ? 0x10 : 0x00);
	check_message_transparency = lookup_widget(general_options_window, "check_message_transparency");
	Message_Style |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_transparency)) ? 0x08 : 0x00);
	
	// Message color
	for (i = 0; i < 4; i++)
	{
		if (!GO_MsgColors[i])
			break;
		sprintf(tmp, "radio_button_message_color_%s", GO_MsgColors[i * 3]);
		radio_button_message_color = lookup_widget(general_options_window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_message_color)))
		{
			Message_Style &= ~0x06;
			Message_Style |= (i << 1);
			break;
		}
	}
	
	// Intro effect color
	for (i = 0; i < 8; i++)
	{
		if (!GO_IntroEffectColors[i])
			break;
		sprintf(tmp, "radio_button_misc_intro_effect_color_%s", GO_IntroEffectColors[i * 3]);
		radio_button_intro_effect_color = lookup_widget(general_options_window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_intro_effect_color)))
		{
			Effect_Color = i;
			break;
		}
	}
}
