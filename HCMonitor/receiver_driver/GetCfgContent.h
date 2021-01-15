#include <string.h>

#define KEYVALLEN 50

#ifdef UP4G	//if use the memory space above 4G, the gap should be subtracted
#define	gap	0x40000000ULL	//0.5G,4G以上总线地址与物理内存地址的差值
		//0.5G for i7@81
		//1G for xeon@232
#else
#define	gap	0
#endif

#ifdef DMA
unsigned long long DMA_BUFFER_ADDRESS;
unsigned long long DMA_BUFFER_TOTAL_SIZE;
unsigned long long DMA_BUFFER_SEGMENT_SIZE;
unsigned long long DMA_CONTROL_REGISTER;
#endif

#ifdef COMMAND
unsigned long long chg_cfg_start;   //删除了第7个bit，得到在第2根内存条上的内存地址，下同
unsigned long long chg_cfg_end;
#endif

#ifdef SAMPLE
unsigned long long chg_sample_start1;
unsigned long long chg_sample_end1;
unsigned long long chg_sample_start2;
unsigned long long chg_sample_end2;
unsigned long long chg_sample_start3;
unsigned long long chg_sample_end3;
unsigned long long chg_sample_start4;
unsigned long long chg_sample_end4;
#endif

#ifdef COMMAND
unsigned long long cfg_start;
unsigned long long cfg_end;
#endif

#ifdef SAMPLE
unsigned long long sample_start1;
unsigned long long sample_end1;
unsigned long long sample_start2;
unsigned long long sample_end2;
unsigned long long sample_start3;
unsigned long long sample_end3;
unsigned long long sample_start4;
unsigned long long sample_end4;
#endif


int GetProfileString(char* profile, char* AppName, char* KeyName, char *KeyVal)
{
	char appname[KEYVALLEN], keyname[KEYVALLEN];
	char buf[KEYVALLEN], *c;
	FILE *fp;
	int found = 0; /* 1 AppName 2 Keyname */

	if( (fp=fopen( profile,"r" ))==NULL ){
		printf( "openfile [%s] error [%s]\n",
				profile,strerror(errno) );
		return(-1);
	}
	fseek( fp, 0, SEEK_SET );

	sprintf( appname,"[%s]", AppName );
	memset( keyname, 0, sizeof(keyname) );

	while( !feof(fp) && fgets( buf, KEYVALLEN, fp )!=NULL ){
	/*	if( l_trim( buf )==0 )
			continue;
	*/
		if( found==0 ){
			if( buf[0]!='[' ) {
				continue;
			} else if ( strncmp(buf,appname,strlen(appname))==0 ){
				found=1;
				continue;
			}
		} else if( found==1 ){
			if( buf[0]=='#' ){
				continue;
			} else if ( buf[0]=='[' ) {
				break;
			} else {
				if( (c=(char*)strchr(buf,'='))==NULL )
					continue;
				memset( keyname, 0, sizeof(keyname) );
				sscanf( buf, "%[^=]", keyname );
				if( strncmp(keyname, KeyName, strlen(KeyName))==0 ){
					sscanf( ++c, "%[^\n]", KeyVal );
					found=2;
					break;
				} else {
					continue;
				}
			}
		}
	}

	fclose( fp );

	if( found==2 )
		return(0);
	else
		return(-1);
}



void GetCfgContent()
{	
    char *content = "../cfg_content";    //cfg_content的存放位置，根据需要修改
    char keyvalue[KEYVALLEN];
#ifdef DMA
    //-------  DMA --------/
    //DMA_BUFFER_ADDRESS
    GetProfileString(content, "DMA" , "DMA_BUFFER_ADDRESS", keyvalue);
    DMA_BUFFER_ADDRESS = (unsigned long long)strtol(keyvalue,NULL,16);
    //DMA_BUFFER_TOTAL_SIZE
    GetProfileString(content, "DMA" , "DMA_BUFFER_TOTAL_SIZE", keyvalue);
    DMA_BUFFER_TOTAL_SIZE = (unsigned long long)strtol(keyvalue,NULL,16);
    //DMA_BUFFER_SEGMENT_SIZE
    GetProfileString(content, "DMA" , "DMA_BUFFER_SEGMENT_SIZE", keyvalue);
    DMA_BUFFER_SEGMENT_SIZE = (unsigned long long)strtol(keyvalue,NULL,16);
    //DMA_CONTROL_REGISTER
    GetProfileString(content, "DMA" , "DMA_CONTROL_REGISTER", keyvalue);
    DMA_CONTROL_REGISTER = (unsigned long long)strtol(keyvalue,NULL,16);
#endif
#ifdef COMMAND
    //---------- Configure Space ----------//
    //cfg_start
    GetProfileString(content, "COMMAND" , "cfg_start", keyvalue);
    cfg_start = (unsigned long long)strtol(keyvalue,NULL,16);
    //cfg_start
    GetProfileString(content, "COMMAND" , "cfg_end", keyvalue);
    cfg_end = (unsigned long long)strtol(keyvalue,NULL,16);
#endif
#ifdef SAMPLE
    //---------- Sample Address Space -----------//
    //sample_start1
    GetProfileString(content, "SAMPLE" , "sample_start1", keyvalue);
    sample_start1 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_end1
    GetProfileString(content, "SAMPLE" , "sample_end1", keyvalue);
    sample_end1 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_start2
    GetProfileString(content, "SAMPLE" , "sample_start2", keyvalue);
    sample_start2 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_end2
    GetProfileString(content, "SAMPLE" , "sample_end2", keyvalue);
    sample_end2 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_start3
    GetProfileString(content, "SAMPLE" , "sample_start3", keyvalue);
    sample_start3 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_end3
    GetProfileString(content, "SAMPLE" , "sample_end3", keyvalue);
    sample_end3 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_start4
    GetProfileString(content, "SAMPLE" , "sample_start4", keyvalue);
    sample_start4 = (unsigned long long)strtol(keyvalue,NULL,16);
    //sample_end4
    GetProfileString(content, "SAMPLE" , "sample_end4", keyvalue);
    sample_end4 = (unsigned long long)strtol(keyvalue,NULL,16);
#endif

#ifdef DualChannel	
    //delete the 7th bit of address to be traced
   #ifdef COMMAND
	chg_cfg_start     =( ((cfg_start-gap) & 0xffffffffffffff80ULL)>>1 ) | ((cfg_start-gap) & 0x040ULL) ;
	chg_cfg_end       =( ((cfg_end  -gap) & 0xffffffffffffff80ULL)>>1 ) | ((cfg_end  -gap) & 0x040ULL) ;
   #endif
   #ifdef SAMPLE
	chg_sample_start1 =( ((sample_start1-gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_start1-gap) & 0x040ULL) ;
	chg_sample_end1   =( ((sample_end1  -gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_end1  -gap) & 0x040ULL) ;
	chg_sample_start2 =( ((sample_start2-gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_start2-gap) & 0x040ULL) ;
	chg_sample_end2   =( ((sample_end2  -gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_end2  -gap) & 0x040ULL) ;
	chg_sample_start3 =( ((sample_start3-gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_start3-gap) & 0x040ULL) ;
	chg_sample_end3   =( ((sample_end3  -gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_end3  -gap) & 0x040ULL) ;
	chg_sample_start4 =( ((sample_start4-gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_start4-gap) & 0x040ULL) ;
	chg_sample_end4   =( ((sample_end4  -gap) & 0xffffffffffffff80ULL)>>1 ) | ((sample_end4  -gap) & 0x040ULL) ;
   #endif

#else
	//single channel, don't neet to delete the 7th bit
   #ifdef COMMAND
	chg_cfg_start		= cfg_start - gap;
	chg_cfg_end		= cfg_end   - gap;
   #endif
   #ifdef SAMPLE
	chg_sample_start1	= sample_start1 - gap;
	chg_sample_end1		= sample_end1   - gap;
	chg_sample_start2	= sample_start2 - gap;
	chg_sample_end2		= sample_end2   - gap;
	chg_sample_start3	= sample_start3 - gap;
	chg_sample_end3		= sample_end3   - gap;
	chg_sample_start4	= sample_start4 - gap;
	chg_sample_end4		= sample_end4   - gap;
   #endif

#endif//of DualChannel
     return;
}
