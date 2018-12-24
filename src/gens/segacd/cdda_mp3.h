#ifndef GENS_CDDA_MP3_H
#define GENS_CDDA_MP3_H
	
#ifdef __cplusplus
extern "C" {
#endif

int MP3_Init(void);
void MP3_Reset(void);
int MP3_Get_Bitrate(FILE *f);
int MP3_Length_LBA(FILE *f);
int MP3_Find_Frame(FILE *f, int pos_wanted);
int MP3_Play(int track, int lba_pos, int async);
int MP3_Update(char *buf, int *rate, int *channel, unsigned int length_dest);
void MP3_Test(FILE* f);

// Various MP3 stuff, needed for Gens Rerecording
extern unsigned int Current_OUT_Pos;
extern unsigned int Current_OUT_Size;

// fatal_mp3_error indicates an error occurred while reading an MP3 for a Sega CD game.
extern int fatal_mp3_error;	// cdda_mp3.c

#ifdef __cplusplus
}
#endif

#endif /* GENS_CDDA_MP3_H */
