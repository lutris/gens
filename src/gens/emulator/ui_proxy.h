#ifndef UI_PROXY_H
#define UI_PROXY_H

int Set_Sprite_Limit(int newLimit);
int Set_Current_State(int slot);
int Set_Frame_Skip(int frames);
int Change_Debug(int Debug_Mode);
int Change_Country_Order(int Num);
int Change_Country(int newCountry)
int Change_SegaCD_PerfectSync(newPerfectSync);

// Sound
int Change_Sound(int newSound);
int Change_Sound_Stereo(int newStereo);
int Change_Z80(int newZ80);
int Change_YM2612(int newYM2612);
int Change_YM2612_Improved(int newYM2612Improved);
int Change_DAC(int newDAC);
int Change_DAC_Improved(int newDACImproved);
int Change_PSG(int newPSG);
int Change_PSG_Improved(int newPSGImproved);
int Change_PCM(int newPCM);
int Change_PWM(int newPWM);
int Change_CDDA(int newCDDA);

int Change_Sample_Rate(int Rate);
int Change_Fast_Blur(void);
int Change_Stretch(int newStretch);
int Change_VSync(int newVSync);
int Change_Blit_Style(void);
int Change_SegaCD_SRAM_Size(int num);
void system_reset(void);
void Set_Game_Name(void);
int Set_Render(int FullScreen, int Mode, int Force);

typedef enum {
	NORMAL = 1,
	DOUBLE = 2,
	INTERPOLATED = 3,
	FULL_SCANLINE = 4,
	SCANLINE_50 = 5,
	SCANLINE_25 = 6,
	INTERPOLATED_SCANLINE = 7,
	INTERPOLATED_SCANLINE_50 = 8,
	INTERPOLATED_SCANLINE_25 = 9,
	KREED = 10,
	SCALE2X = 11,
	HQ2X = 12,
	NB_FILTER = 13} _filters;
	
extern _filters filters;

#define SELECT_RENDERER(renderer_nb, renderer_fn,msg) \
{\
	*Rend = renderer_nb;\
 	*Blit = renderer_fn;\
	MESSAGE_L("Render selected: " msg,"Render selected: " msg, 1500)\
}

#endif
