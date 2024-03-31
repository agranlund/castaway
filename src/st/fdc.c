/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* fdc.c - wd1772/dma emulation
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  0.02.00 JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  09.06.2002  0.02.00 JH  Renamed io.c to st.c again (io.h conflicts with system headers)
*/

/*
 TODO: Looks like MSA support assumes little endian
*/
static char 	sccsid[] = "$Id: fdc.c,v 1.2 2002/06/08 23:31:58 jhoenig Exp $";

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "st.h"
#include "68000.h"


typedef struct {
	uint16 ID;					// Word ID marker, should be $0E0F
	uint16 SectorsPerTrack; 	// Word Sectors per track
	uint16 Sides;				// Word Sides (0 or 1; add 1 to this to get correct number of sides)
	uint16 StartingTrack;		// Word Starting track (0-based)
	uint16 EndingTrack; 		// Word Ending track (0-based)
} MSAHEADERSTRUCT;


// data section
unsigned char   fdc_buffer[8000] = {0};
unsigned char   gap1[38+2]={0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA1,0xA1,0xfe,0xa1,0,0};
unsigned char   gap2[38+2]={0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x4e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA1,0xA1,0xfb,0xa1,0,0};
struct Disk     disk[2] = { { DISKA, NULL, 0, SIDES, TRACKS, SECTORS, SECSIZE, 0, 0, 0 }, { DISKB, NULL, 0, SIDES, TRACKS, SECTORS, SECSIZE, 0, 1, 0 }, };

// bss section
short           fdc_transfert, fdc_active;
unsigned int    fdc_start_adr,fdc_end_adr,fdc_current_adr;

unsigned char	fdc_data,fdc_track,fdc_sector,fdc_status;
unsigned char	fdc_command,fdc_motor,fdc_int;
char	        fdcdir;

unsigned char   *msabuff;
unsigned char   *disc[2];
int             discpos[2];


int discread(unsigned char *buf,int a,int len,int discn)
{
    int i;
    uint8 val1,val2;
    #ifdef LITTLE_ENDIAN
    uint8* dbuf = buf;
    #endif

    for (i=0;i<len;i++)
        *buf++=disc[discn][discpos[discn]+i];

    #ifdef LITTLE_ENDIAN
    for (i=0; i<len; i+=2) {
        val1 = dbuf[i];
        val2 = dbuf[i+1];
        dbuf[i] = val2;
        dbuf[i+1] =val1;
    }
    #endif
    discpos[discn]=discpos[discn]+i;
    fdc_active |= ( discn + 1 ); // jeff
    return len;
}

int discwrite(unsigned char *buf,int a,int len,int discn)
{
	int i;
	uint8 val1,val2;
    #ifdef LITTLE_ENDIAN
    uint8* dbuf = buf;
    #endif

	for (i=0;i<len;i++)
        disc[discn][discpos[discn]+i]=*buf++;
    
    #ifdef LITTLE_ENDIAN
	for (i=0; i<len; i+=2) {
		val1 = disc[discn][discpos[discn]+i];
		val2 = disc[discn][discpos[discn]+i+1];
		disc[discn][discpos[discn]+i] = val2;
		disc[discn][discpos[discn]+i+1] =val1;
	}
    #endif
	discpos[discn]=discpos[discn]+i;
    fdc_active |= ( discn + 1 ); // jeff
	return len;
}

int discseek(int discn,int pos,int a)
{
	if (pos>(1050*1024)){
		return -1;
	}
	discpos[discn]=pos;
	return 0;
}

int MSA_UnCompress(unsigned char *pBuffer)
{
	MSAHEADERSTRUCT *pMSAHeader;
	unsigned char *pMSAImageBuffer,*pImageBuffer;
	unsigned char Byte,Data;
	int ImageSize = 0;
	int i,Track,Side,DataLength,NumBytesUnCompressed,RunLength;

    memcpy(msabuff, pBuffer, 1050*1024);

	// Is an '.msa' file?? Check header
	pMSAHeader = (MSAHEADERSTRUCT *)msabuff;

#ifdef LITTLE_ENDIAN
	if (pMSAHeader->ID==0x0F0E) {
	  // First swap 'header' words around to PC format - easier later on
      #define STMemory_Swap68000Int(val) ((val<<8)|(val>>8))
	  pMSAHeader->SectorsPerTrack = STMemory_Swap68000Int(pMSAHeader->SectorsPerTrack);
	  pMSAHeader->Sides = STMemory_Swap68000Int(pMSAHeader->Sides);
	  pMSAHeader->StartingTrack = STMemory_Swap68000Int(pMSAHeader->StartingTrack);
	  pMSAHeader->EndingTrack = STMemory_Swap68000Int(pMSAHeader->EndingTrack);
#else
	if (pMSAHeader->ID==0x0E0F) {
#endif        
        // Set pointers
        pImageBuffer = (unsigned char *)pBuffer;
        pMSAImageBuffer = (unsigned char *)((unsigned long)msabuff+sizeof(MSAHEADERSTRUCT));
            
        // Uncompress to memory as '.ST' disc image - NOTE assumes 512 bytes per sector(use NUMBYTESPERSECTOR define)!!!
        for(Track=pMSAHeader->StartingTrack; Track<(pMSAHeader->EndingTrack+1); Track++)
        {
            for(Side=0; Side<(pMSAHeader->Sides+1); Side++)
            {
                // Uncompress MSA Track, first check if is not compressed
                DataLength = ((uint16)(*pMSAImageBuffer)<<8)|((uint16)(*(pMSAImageBuffer+1))&0xff);
                pMSAImageBuffer += sizeof(short int);
                if (DataLength==(512*pMSAHeader->SectorsPerTrack))
                {
                    // No compression on track, simply copy and continue
                    memcpy(pImageBuffer,pMSAImageBuffer,512*pMSAHeader->SectorsPerTrack);
                    pImageBuffer += 512*pMSAHeader->SectorsPerTrack;
                    pMSAImageBuffer += DataLength;
                }
                else
                {
                    // Uncompress track
                    NumBytesUnCompressed = 0;
                    while(NumBytesUnCompressed<(512*pMSAHeader->SectorsPerTrack))
                    {
                        Byte = *pMSAImageBuffer++;
                        if (Byte!=0xE5) {												// Compressed header??
                            *pImageBuffer++ = Byte; 									// No, just copy byte
                            NumBytesUnCompressed++;
                        }
                        else {
                            Data = *pMSAImageBuffer++;									// Byte to copy
                            RunLength =((uint16)(*pMSAImageBuffer)<<8)|((uint16)(*(pMSAImageBuffer+1))&0xff);	// For length
                            // Limit length to size of track, incorrect images may overflow
                            if ( (RunLength+NumBytesUnCompressed)>(512*pMSAHeader->SectorsPerTrack) )
                                RunLength = (512*pMSAHeader->SectorsPerTrack)-NumBytesUnCompressed;
                            pMSAImageBuffer += sizeof(short int);
                            for(i=0; i<RunLength; i++)
                                *pImageBuffer++ = Data; 								// Copy byte
                            NumBytesUnCompressed += RunLength;
                        }
                    }
                }
	        }
	    }
        // Set size of loaded image
        ImageSize = (unsigned long)pImageBuffer-(unsigned long)pBuffer;
	}
	// Return number of bytes loaded, '0' if failed
	return(ImageSize);
}


int 		   FDCInit(int i)
{
	unsigned char *buf;
	int len,len2,calcsides,calcsectors,calctracks,badbootsector;

    if (msabuff == 0) {
        msabuff = (unsigned char*) AllocateMem(1050L*1024L, 16, MEM_FAST);
    }
    if (disc[i] == 0) {
        disc[i] = (unsigned char*) AllocateMem(1050L*1024L, 16, MEM_FAST);
    }
    if ((msabuff == 0) || (disc[i] == 0)) {
        HALT("Couldn't alloc memory for disks..." );
    }

	discpos[i]=0;
	if (NULL != (disk[i].file = fopen (disk[i].name, "r+b")))
    {
		buf=&disc[i][0];
		
        fseek(disk[i].file,0,SEEK_END);
        len=ftell(disk[i].file);
        fseek(disk[i].file,0,SEEK_SET);
        fread(buf,1,len,disk[i].file);
        fclose(disk[i].file);

		disk[i].disksize = len;
		len2=MSA_UnCompress(buf);
		if (len2) len=len2;
		disk[i].head = 0;		
		disk[i].stt = 0;
#if 0
		if(*(int *)(buf)==0x4d455453){
			badbootsector=0;
			disk[i].tracks = *(unsigned short *)(buf + 10);
			disk[i].sides = *(unsigned short *) (buf + 12);
			disk[i].stt = 1;
			if (*(unsigned short*)(buf + 8)!=1) {
                return 2;
            }
		} else
#endif
        {
			disk[i].head = 0;
			disk[i].sides = (int) *(buf + 26);
			disk[i].sectors = (int) *(buf + 24);
			disk[i].secsize = 512; //(int) ((*(buf + 12) << 8) | *(buf + 11));
			if (disk[i].sectors && disk[i].sides) {
				disk[i].tracks = (int) ((*(buf + 20) << 8) | *(buf + 19)) / (disk[i].sectors * disk[i].sides);
            }
			
			// Second Check more precise 
			if (len> (500*1024)) calcsides = 2;
			else calcsides = 1;

			if (!(((len/calcsides)/512)%9)&&(((len/calcsides)/512)/9)<86) calcsectors=9;
			else if (!(((len/calcsides)/512)%10)&&(((len/calcsides)/512)/10)<86) calcsectors=10;
			else if (!(((len/calcsides)/512)%11)&&(((len/calcsides)/512)/11)<86) calcsectors=11;
			else if (!(((len/calcsides)/512)%12)) calcsectors=12;
            else calcsectors = 9;

			calctracks =((len/calcsides)/512)/calcsectors;
			if (disk[i].sides!=calcsides||disk[i].sectors!=calcsectors||disk[i].tracks!=calctracks){
				if (disk[i].sides==calcsides&&disk[i].sectors==calcsectors){
					disk[i].tracks=calctracks;
					badbootsector=0;
				}else{
					disk[i].sides=calcsides;
					disk[i].tracks=calctracks;
					disk[i].sectors=calcsectors;
					badbootsector=(i<<24)|(calcsides<<16)|(calctracks<<8)|(calcsectors);
				}
				
			}else{
				badbootsector=0;
			}
		}
		fdc_status |= 0x40;
        disk[i].ejected = 0;
        disk[i].changed = 1;
		disk[i].head = 0;
		fdc_track = 0;
	} else {
         return 1;
    }
	return badbootsector;
}

void FDCchange(int i){
	disk[(i>>24)&0xff].sides=(i>>16)&0xff;
	disk[(i>>24)&0xff].tracks=(i>>8)&0xff;
	disk[(i>>24)&0xff].sectors=i&0xff;
	
	
}

void FDCeject(int num){
	int i;
    if (disc[num]) {
        memset(disc[num], 0, 1050*1024);
    }
	disk[num].file = NULL;
	sprintf(disk[num].name,"disk%01d",num);
	disk[num].sides = SIDES;
	disk[num].tracks = TRACKS;
	disk[num].sectors = SECTORS;
	disk[num].secsize = 512;
    disk[num].ejected = 1;
	fdc_status |= 0x40;
}

void			FDCCommand(void)
{
	static char 	motor = 1;
	int 			sides, drives,sector_found;
	long			address;	/* dma target/source address */
	long			offset,offset_track,offset_sector;	   /* offset in disk file */
	unsigned long	count;		/* number of byte to transfer */
	char		   *buffer;
	int n,i,j,k;
	/* DMA target/source address */
	address = (dma_adrh << 16) + (dma_adrm << 8) + dma_adrl;
	/*	if (address>MEMSIZE){
	#ifdef DISASS
	StartDisass();
	exit(1);
	#endif
	fdc_status |= 0x10;
	   return;	
	   
		 
		   
			 }
	*/
	buffer = (char*) (membase + address);
	/* status of side select and drive select lines */
	sides = (~psg[14]) & 0x1;
	drives = (~psg[14]) & 0x6;
    if (disk[drives>>2].ejected) {
        drives = 2;
    }
	switch (drives) {
	case 2: 					/* Drive A */
		drives = 0;
		break;
	case 4: 					/* Drive B */
		drives = 1;
		break;
	case 6: 					/* both, error */
	case 0: 					/* no drive selected */
		drives = -1;
		break;
	}
	fdc_status = 0; 			/* clear fdc status */
	if (fdc_command < 0x80) {	/* TYPE-I fdc commands */
		if (drives >= 0) {		/* drive selected */
			switch (fdc_command & 0xf0) {
			case 0x00:			/* RESTORE */
				disk[drives].head = 0;
				fdc_track = 0;
				break;
			case 0x10:			/* SEEK */
				disk[drives].head += (fdc_data - fdc_track);
				fdc_track = fdc_data;
				if (disk[drives].head < 0
					|| disk[drives].head >= disk[drives].tracks)
					disk[drives].head = 0;
				break;
			case 0x30:			/* STEP */
				fdc_track += fdcdir;
			case 0x20:
				disk[drives].head += fdcdir;
				break;
			case 0x50:			/* STEP-IN */
				fdc_track++;
			case 0x40:
				if (disk[drives].head < disk[drives].tracks)
					disk[drives].head++;
				fdcdir = 1;
				break;
			case 0x70:			/* STEP-OUT */
				fdc_track--;
			case 0x60:
				if (disk[drives].head > 0)
					disk[drives].head--;
				fdcdir = -1;
				break;
			}
			if (disk[drives].head == 0) {
				fdc_status |= 0x4;
			}
			if (disk[drives].head != fdc_track && fdc_command & 0x4) {	/* Verify? */
				fdc_status |= 0x10;
			}
			if (motor) {
				fdc_status |= 0x20; 	/* spin-up flag */
			}
		} else {				/* no drive selected */
			fdc_status |= 0x10;
		}
	if (!(fdc_status & 0x01)) { /* not busy */
		mfp_iprb |= (0x80 & mfp_ierb);	/* Request Interrupt */
		mfp_gpip &= ~0x20;
	}
	} else if ((fdc_command & 0xf0) == 0xd0) {	/* FORCE INTERRUPT */
		if (fdc_command == 0xd8) {
			fdc_transfert=0;
			mfp_iprb |= (0x80 & mfp_ierb);	/* Request Interrupt */
			fdc_int = 1;
		} else if (fdc_command == 0xd0) {
			fdc_transfert=0;
			fdc_int = 0;
		}
//		mfp_gpip &= ~0x20;
	} else {					/* OTHERS */
		if (drives >= 0) {		/* drive selected */
			/* offset within floppy-file */
			if (disk[drives].stt==0){
				offset = disk[drives].secsize *
					(((disk[drives].sectors * disk[drives].sides * disk[drives].head))
					+ (disk[drives].sectors * sides) + (fdc_sector - 1));
			}else{
				offset = (long) (&disc[drives][0]+14+(sides*(disk[drives].sides-1)*disk[drives].tracks)*6+disk[drives].head*6);
				offset_track = (long) (&disc[drives][0]+(((*(unsigned short *)(offset+2))<<16)+(*(unsigned short *)offset)));
				disk[drives].sectors = *(unsigned short *)(offset_track+10);
			}
			switch (fdc_command & 0xf0) {

			case 0x80:			/* READ SECTOR */				fdc_start_adr=address;
				if (disk[drives].stt==0){
					count = 512;
				}else{
                    offset_sector = 0;
					for (n=0;n<disk[drives].sectors;n++){
						if ((*(unsigned char *)(offset_track+12+n*10+2))==fdc_sector){
							offset_sector=(*(unsigned short *)(offset_track+12+n*10+6));
							break;
						}
					}
					offset=(((*(unsigned short *)(offset+2))<<16)+(*(unsigned short *)offset))+offset_sector;
					count= 128 << (*(unsigned char *)(offset_track+12+n*10+3));
				}
				if (!discseek (drives, offset, 0)) {
					if (address<MEMSIZE){
						if (count == discread (fdc_buffer, 1, count, drives)) {
							address += count;
							fdc_end_adr=address;
							fdc_current_adr=0;
							fdc_transfert=1;
							/*dma_adrl = address & 0xff;
							dma_adrm = (address >> 8) & 0xff;
							dma_adrh = (address >> 16) & 0xff;*/
							//dma_scr = 0;
							//dma_sr = 1;
							break;
						}
					}else{
						address += count;
						dma_adrl = address & 0xff;
						dma_adrm = (address >> 8) & 0xff;
						dma_adrh = (address >> 16) & 0xff;
						dma_scr = 0;
						dma_sr = 1;
						mfp_gpip |= 0x20;
						fdc_status |= 0x1;
						break;
					}
				}
				fdc_status |= 0x10;
				dma_sr = 1;
				break;
			case 0x90:			/* READ SECTOR multiple */
				fdc_start_adr=address;
				if (disk[drives].stt==0){
					count = dma_scr * 512;
					if (count+(fdc_sector-1)*512>disk[drives].sectors*512) count=disk[drives].sectors*512-(fdc_sector-1)*512;
					if (!discseek (drives, offset, 0)) {
						if (address<MEMSIZE){
							if (count == discread (fdc_buffer, 1, count, drives)) {
								address += count;
								fdc_end_adr=address;
								fdc_current_adr=0;
								fdc_transfert=1;
								/*dma_adrl = address & 0xff;
								dma_adrm = (address >> 8) & 0xff;
								dma_adrh = (address >> 16) & 0xff;
								dma_scr = 0;*/
								//dma_sr = 1;
								fdc_sector += count/disk[drives].secsize;
								break;
							}
						}else{
							address += count;
							dma_adrl = address & 0xff;
							dma_adrm = (address >> 8) & 0xff;
							dma_adrh = (address >> 16) & 0xff;
							dma_scr = 0;
							dma_sr = 1;
							mfp_gpip |= 0x20;
							fdc_status |= 0x1;
							break;
						}
					}
					fdc_status |= 0x10;
					dma_sr = 1;
				}else{
					j=0;
					while (dma_scr>0){ 
						sector_found=0;
						for (i=0;i<disk[drives].sectors;i++){ 
							if ((*(unsigned char *)(offset_track+12+i*10+2))==fdc_sector){
								sector_found=1;
								break;
							}
						}
						
						if (sector_found){ 
							offset_sector=(((*(unsigned short *)(offset+2))<<16)+(*(unsigned short *)offset))+(*(unsigned short *)(offset_track+12+i*10+6)); 
							count= 128 << (*(unsigned char *)(offset_track+12+i*10+3)); 
							discseek (drives, offset_sector, 0); 
							buffer = (char*) (membase + address);
							/*count ==*/ discread (fdc_buffer+j, 1, count, drives); // <-- 
							j+=count;
							fdc_sector++; 
							dma_scr--; 
							address += count; 
						}else{
							//fdc_sector--; 
							break;		
						} 
					}
					fdc_end_adr=address;
					fdc_current_adr=0;
					fdc_transfert=1;
					/*dma_adrl = address & 0xff; 
					dma_adrm = (address >> 8) & 0xff; 
					dma_adrh = (address >> 16) & 0xff; */
					//dma_sr = 1; 
					break;
					
				}

				break;
			case 0xa0:			/* WRITE SECTOR */
				count = dma_scr * 512;
				if (!discseek (drives, offset, 0)) {
					if (count == discwrite (buffer, 1, count, drives)) {
						address += count;
						dma_adrl = address & 0xff;
						dma_adrm = (address >> 8) & 0xff;
						dma_adrh = (address >> 16) & 0xff;
						dma_scr = 0;
						dma_sr = 1;
						break;
					}
				}
				fdc_status |= 0x10;
				dma_sr = 1;
				break;
			case 0xb0:			/* WRITE SECTOR multiple */
				count = dma_scr * 512;
				if (!discseek (drives, offset, 0)) {
					if (count == discwrite (buffer, 1, count, drives)) {
						address += count;
						dma_adrl = address & 0xff;
						dma_adrm = (address >> 8) & 0xff;
						dma_adrh = (address >> 16) & 0xff;
						dma_scr = 0;
						dma_sr = 1;
						fdc_sector += dma_scr * (512 / disk[drives].secsize);
						break;
					}
				}
				fdc_status |= 0x10;
				dma_sr = 1;
				break;
			case 0xc0:			/* READ ADDRESS */
				fdc_status |= 0x10;
				break;
			case 0xe0:			/* READ TRACK */
				fdc_start_adr=address;
				if (disk[drives].stt==0){
				count = disk[drives].sectors * 512;
				offset = disk[drives].secsize *
					(((disk[drives].sectors * disk[drives].sides * disk[drives].head))
					+ (disk[drives].sectors * sides));
				if (!discseek (drives, offset, 0)) {
					if (address<MEMSIZE){
						if (dma_scr==0x1f){
							count=0;
							address += 302;
						}
						if (count == discread (fdc_buffer, 1, count, drives)) {
							fdc_end_adr=address;
							fdc_current_adr=0;
							fdc_transfert=1;
							dma_scr = 0;
							/*dma_adrl = address & 0xff;
							dma_adrm = (address >> 8) & 0xff;
							dma_adrh = (address >> 16) & 0xff;*/
							//dma_sr = 1;
							break;
						}
					}else{
						address += 302;
						dma_adrl = address & 0xff;
						dma_adrm = (address >> 8) & 0xff;
						dma_adrh = (address >> 16) & 0xff;
						dma_scr = 0;
						dma_sr = 1;
						mfp_gpip |= 0x20;
						fdc_status |= 0x1;
						break;
					}
				}
				fdc_status |= 0x10;
				dma_sr = 1;
				}else{
					k=0;
					while (dma_scr>0){ 
						sector_found=0;
						for (i=0;i<disk[drives].sectors;i++){ 
							if ((*(unsigned char *)(offset_track+12+i*10+2))==fdc_sector){
								sector_found=1;
								break;
							}
						}
						
						if (sector_found){ 
							for (j=0;j<38;j++) *(unsigned char*)(fdc_buffer + k++)=gap1[j];
							for (j=0;j<6;j++) *(unsigned char*)(fdc_buffer + k++)=*(unsigned char *)(offset_track+12+i*10+j^1); 
							for (j=0;j<38;j++) *(unsigned char*)(fdc_buffer + k++)=gap2[j];
							address+=82;
							offset_sector=(((*(unsigned short *)(offset+2))<<16)+(*(unsigned short *)offset))+(*(unsigned short *)(offset_track+12+i*10+6)); 
							count= 128 << (*(unsigned char *)(offset_track+12+i*10+3)); 
							discseek (drives, offset_sector, 0); 
							buffer = membase + address; 
							/*count ==*/ discread (fdc_buffer+k, 1, count, drives); 
							fdc_sector++; 
							dma_scr--; 
							address += count;
							k+=count;
						}else{
							//fdc_sector--; 
							break;		
						} 
					}
					fdc_end_adr=address;
					fdc_current_adr=0;
					fdc_transfert=1;
				/*	dma_adrl = address & 0xff; 
					dma_adrm = (address >> 8) & 0xff; 
					dma_adrh = (address >> 16) & 0xff; */
					//dma_sr = 1; 
					break;
				}
				break;
			case 0xf0:			/* WRITE TRACK */
				fdc_status |= 0x10;
				break;
			}
			if (disk[drives].head != fdc_track) {
				fdc_status |= 0x10;
			}
		} else {
			fdc_status |= 0x10; /* no drive selected */
		}
	if (!(fdc_status & 0x01)) { /* not busy */
//		fdc_transfert=1;
//		mfp_iprb |= (0x80 & mfp_ierb);	/* Request Interrupt */
		mfp_gpip |= 0x20;
	}
	}
	if (motor) {
		fdc_status |= 0x80; 	/* motor on flag */
		fdc_motor=1;
	}
//	if (!(fdc_status & 0x01)) { /* not busy */
//		fdc_transfert=1;
//		mfp_iprb |= (0x80 & mfp_ierb);	/* Request Interrupt */
//		mfp_gpip |= 0x20;
//	}
}
