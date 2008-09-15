#ifndef GENS_SDLSOUND_H
#define GENS_SDLSOUND_H

#if 0

#ifdef __cplusplus
extern "C" {
#endif

extern int Sound_Rate;
extern int Sound_Segs;
extern int Sound_Enable;
extern int Sound_Stereo;
extern int Sound_Is_Playing;
extern int Sound_Initialised;
extern int WAV_Dumping;
extern int GYM_Playing;
extern int Seg_L[882], Seg_R[882];
extern int Seg_Length;

extern unsigned int Sound_Interpol[882];
extern unsigned int Sound_Extrapol[312][2];

extern char Dump_Dir[1024];
extern char Dump_GYM_Dir[1024];

int Init_Sound(void);
void End_Sound(void);
int Get_Current_Seg(void);
int Check_Sound_Timing(void);
int Write_Sound_Buffer(void *Dump_Buf);
int Clear_Sound_Buffer(void);
int Play_Sound(void);
int Stop_Sound(void);
int Start_WAV_Dump(void);
int Update_WAV_Dump(void);
int Stop_WAV_Dump(void);
int Lots_In_Audio_Buffer(void);

extern int fast_forward;

#ifdef __cplusplus
}
#endif

#endif /* 0 */

#endif /* GENS_SDLSOUND_H */
