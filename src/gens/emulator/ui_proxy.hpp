/**
 * Gens: UI proxy functions.
 */

#ifndef UI_PROXY_HPP
#define UI_PROXY_HPP

#ifdef __cplusplus
extern "C" {
#endif

int Set_Sprite_Limit(const int newLimit);
int Set_Current_State(const int slot);
int Set_Frame_Skip(const int frames);
int Change_Debug(const int Debug_Mode);
int Change_Country(const int newCountry);
int Change_SegaCD_PerfectSync(const int newPerfectSync);

// Sound
int Change_Sound(const int newSound);
int Change_Sound_Stereo(const int newStereo);
int Change_Z80(const int newZ80);
int Change_YM2612(const int newYM2612);
int Change_YM2612_Improved(const int newYM2612Improved);
int Change_DAC(const int newDAC);
int Change_DAC_Improved(const int newDACImproved);
int Change_PSG(const int newPSG);
int Change_PSG_Improved(const int newPSGImproved);
int Change_PCM(const int newPCM);
int Change_PWM(const int newPWM);
int Change_CDDA(const int newCDDA);

int Change_Sample_Rate(const int Rate);
int Change_Fast_Blur(void);
int Change_Stretch(const int newStretch);
int Change_VSync(const int newVSync);
int Change_Blit_Style(void);
int Change_SegaCD_SRAM_Size(const int num);
void system_reset(void);
void Set_Game_Name(void);

#ifdef __cplusplus
}
#endif

#endif
