#ifndef __CDDA_MP3_H__
#define __CDDA_MP3_H__
	
int MP3_Init(void);
void MP3_Reset(void);
int MP3_Get_Bitrate(FILE *f);
int MP3_Lenght_LBA(FILE *f);
int MP3_Find_Frame(FILE *f, int pos_wanted);
int MP3_Play(int track, int lba_pos, int async);
int MP3_Update(char *buf, int *rate, int *channel, unsigned int lenght_dest);
void MP3_Test(FILE* f);

#endif /* __CDDA_MP3_H__ */
