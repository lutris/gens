#ifndef GENS_CD_ASPI_H
#define GENS_CD_ASPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "port/port.h"


//***************************************************************************
//                          %%% TARGET STATUS VALUES %%%
//***************************************************************************
#define STATUS_GOOD     0x00    // Status Good

//***************************************************************************
//                %%% Commands Unique to CD-ROM Devices %%%
//***************************************************************************
#define SCSI_READ_MSF   0xB9    // Read CD MSF format (O)


/* SCSI Miscellaneous Stuff */
#define SENSE_LEN			14
#define SRB_POSTING			0x01
#define SRB_DIR_IN			0x08

/* ASPI Command Definitions */
#define SC_EXEC_SCSI_CMD		0x02

/* SRB status codes */
#define SS_PENDING			0x00
#define SS_COMP				0x01

/* SRB - EXECUTE SCSI COMMAND - SC_EXEC_SCSI_CMD */
typedef struct tagSRB32_ExecSCSICmd {
  BYTE        SRB_Cmd;            /* ASPI command code = SC_EXEC_SCSI_CMD */
  BYTE        SRB_Status;         /* ASPI command status byte */
  BYTE        SRB_HaId;           /* ASPI host adapter number */
  BYTE        SRB_Flags;          /* ASPI request flags */
  DWORD       SRB_Hdr_Rsvd;       /* Reserved */
  BYTE        SRB_Target;         /* Target's SCSI ID */
  BYTE        SRB_Lun;            /* Target's LUN number */
  WORD        SRB_Rsvd1;          /* Reserved for Alignment */
  DWORD       SRB_BufLen;         /* Data Allocation Length */
  BYTE        *SRB_BufPointer;    /* Data Buffer Point */
  BYTE        SRB_SenseLen;       /* Sense Allocation Length */
  BYTE        SRB_CDBLen;         /* CDB Length */
  BYTE        SRB_HaStat;         /* Host Adapter Status */
  BYTE        SRB_TargStat;       /* Target Status */
  int        (*SRB_PostProc)(struct tagSRB32_ExecSCSICmd *s);		  /* Post routine */
  void        *SRB_Rsvd2;         /* Reserved */
  BYTE        SRB_Rsvd3[16];      /* Reserved for expansion */
  BYTE        CDBByte[16];        /* SCSI CDB */
  BYTE        SenseArea[SENSE_LEN+2];  /* Request sense buffer - var length */
} SRB_ExecSCSICmd, *PSRB_ExecSCSICmd;

#include "cd_sys.hpp"

#define STOP_DISC	0
#define START_DISC	1
#define OPEN_TRAY   2
#define CLOSE_TRAY  3

typedef struct
{
  BYTE      rsvd;
  BYTE      ADR;
  BYTE      trackNumber;
  BYTE      rsvd2;
  BYTE      addr[4];
} TOCTRACK;

typedef struct
{
  WORD      tocLen;
  BYTE      firstTrack;
  BYTE      lastTrack;
  TOCTRACK tracks[100];
} TOC, *PTOC, *LPTOC;

extern int Num_CD_Drive;
extern int CUR_DEV; // a che serve?
extern int DEV_PAR[8][3];

int ASPI_Init(void);
int ASPI_End(void);

void ASPI_Reset_Drive(char *buf);

int ASPI_Test_Unit_Ready(int timeout);
int ASPI_Lock(int flock);
int ASPI_Star_Stop_Unit(int op, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
void LINUXCD_Select_Speed(void);
int ASPI_Read_TOC(int MSF, int format, int st, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Stop_Play_Scan(int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Seek(int pos, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));
int ASPI_Read_CD_LBA(int adr, int length, unsigned char sector, unsigned char flag, unsigned char sub_chan, int async, int (*PostProc) (struct tagSRB32_ExecSCSICmd *));

// Default Callback

int ASPI_Star_Stop_Unit_COMP(SRB_ExecSCSICmd *s);
int ASPI_Stop_Play_Scan_COMP(SRB_ExecSCSICmd *s);
int ASPI_Seek_COMP(SRB_ExecSCSICmd *s);

// Customize Callback

int ASPI_Stop_CDD_c1_COMP(SRB_ExecSCSICmd *s);
int ASPI_Fast_Seek_COMP(SRB_ExecSCSICmd *s);
int ASPI_Seek_CDD_c4_COMP(SRB_ExecSCSICmd *s);
int ASPI_Close_Tray_CDD_cC_COMP(SRB_ExecSCSICmd *s);
int ASPI_Open_Tray_CDD_cD_COMP(SRB_ExecSCSICmd *s);


// CDC functions

void ASPI_Read_One_LBA_CDC(void);
void Wait_Read_Complete(void);


// Specials functions

void Fill_SCD_TOC_from_MSF_TOC(void);
void Fill_SCD_TOC_Zero(void);


#ifdef __cplusplus
}
#endif

#endif /* GENS_CD_ASPI_H */
