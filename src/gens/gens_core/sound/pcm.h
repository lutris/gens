#ifndef GENS_PCM_CD_H
#define GENS_PCM_CD_H

#ifdef __cplusplus
extern "C" {
#endif

struct pcm_chip_
{
	float Rate;
	int Enable;
	int Cur_Chan;
	int Bank;

	struct pcm_chan_
	{
		unsigned int ENV;		/* envelope register */
		unsigned int PAN;		/* pan register */
		unsigned int MUL_L;		/* envelope & pan product letf */
		unsigned int MUL_R;		/* envelope & pan product right */
		unsigned int St_Addr;	/* start address register */
		unsigned int Loop_Addr;	/* loop address register */
		unsigned int Addr;		/* current address register */
		unsigned int Step;		/* frequency register */
		unsigned int Step_B;	/* frequency register binaire */
		unsigned int Enable;	/* channel on/off register */
		int Data;				/* wave data */
	} Channel[8];
};

extern struct pcm_chip_ PCM_Chip;
extern unsigned char Ram_PCM[64 * 1024];
extern int PCM_Enable;

int  PCM_Init(int Rate);
void PCM_Set_Rate(int Rate);
void PCM_Reset(void);
void PCM_Write_Reg(unsigned int Reg, unsigned int Data);
int  PCM_Update(int **buf, int Length);

#ifdef __cplusplus
}
#endif

#endif
