#include <gtk/gtk.h>


void
on_rom_history_activate				   (GtkMenuItem     *menuitem,
										gpointer         user_data);
void
on_open_rom_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_rom_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_boot_cd_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_netplay_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_game_genie_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_load_state_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_state_as_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quick_load_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quick_save_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot0_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot2_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot3_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot4_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot5_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot6_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot7_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot8_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_state_slot9_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_full_screen_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_vsync_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_stretch_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_color_adjust_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_normal_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_double_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_interpolated_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_scanline_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_50_scanline_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_25_scanline_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_interpolated_scanline_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_interpolated_50_scanline_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_interpolated_25_scanline_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_2xsai_kreed_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_scale2x_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_hq2x_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
                                        
void
on_sprite_limit_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_auto_skip_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_0_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_1_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_2_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_3_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_4_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_5_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_6_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_7_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frame_8_skip_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_screen_shot_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_genesis_68000_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_genesis_z80_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_genesis_vdp_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_segacd_68000_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_segacd_cdc_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_segacd_gfx_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_32x_main_sh2_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_32x_sub_sh2_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_32x_vdp_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_auto_detect_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_japan_ntsc_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_usa_ntsc_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_europe_pal_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_japan_pal_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_detection_order_usa_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_detection_order_japan_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_detection_order_europe_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_hard_reset_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_main_68000_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_sub_68000_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_68000_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_main_sh2_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_sub_sh2_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_z80_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_perfect_synchro_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_enable_sound_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rate_11025_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rate_22050_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rate_44100_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_rate_16000_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_rate_32000_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_rate_48000_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_stereo_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_z80_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_ym2612_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_dac_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_psg_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pcm_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pwm_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_cdda_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_start_wav_dump_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_start_gym_dump_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_misc_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_joypads_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_directories_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_bios_misc_files_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_none_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_8_kb_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_16_kb_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_32_kb_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_64_kb_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_load_config_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_config_as_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_buttonGGAddCode_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonGGDelete_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonGGDesactivate_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonGGOK_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonColorApply_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonColorClose_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonColorDefault_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonOptionCancel_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonOptionOK_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonControlSetOK_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_button26_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDirOK_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button47_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonCfgBios_clicked               (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_gens_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_gens_window_destroy_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_ym2612_improved_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_dac_improved_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_psg_improved_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
 void
on_bpp16_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_bpp24_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_bpp32_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_opengl_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_320x240_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_640x480_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_800x600_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_1024x768_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_custom_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void                                        
on_opengl_resolution_custom_show       (GtkWidget       *widget,
                                         gpointer         user_data);                                        
void on_glres_ok_button_clicked             (GtkButton       *button,
                                         gpointer         user_data);
void
on_item10_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_buttonokcd_clicked			(GtkButton       *button,
                                        gpointer         user_data);
void
on_sdlok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_RunTest_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_sdl_test_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
