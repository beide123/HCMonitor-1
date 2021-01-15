
//########################################################
//##            Receiver Related Information            ##
//########################################################

// HMTT DMA Buffer Registers
#define DMA_BUF_ADDR			0x0000000200100000ULL
#define DMA_BUF_SIZE			0x0000000020000000ULL
#define DMA_REG_ADDR			0x0000000200000000ULL
#define DMA_SEG_SIZE			0x0000000004000000ULL

//#########################################################
//##             Sender Related Information              ##
//#########################################################

// HMTT Config Space Range Registers
#define CFG_START			0x00000000cf800000ULL  
#define	CFG_END				0x00000000cf900000ULL

// HMTT Valid Trace Range Registers
//#define PA_RANGE_START			0x00000000c0000000ULL
//#define PA_RANGE_END			0x00000000ffffffffULL

#define PA_RANGE_START			0x0000000000000000ULL
#define PA_RANGE_END			0x000000ffffffffffULL
// HMTT Physical Address Map Registers
#define	PA_MAP_1_2			0x0000010400000103ULL
#define	PA_MAP_3_4			0x0000010600000105ULL
#define	PA_MAP_5_6			0x0000010800000107ULL
#define	PA_MAP_7_8			0x0000010a00000109ULL
#define	PA_MAP_9_10			0x0000010c0000010bULL
#define	PA_MAP_11_12			0x0000010e0000010dULL
#define	PA_MAP_13_14			0x000001100000010fULL
#define	PA_MAP_15_16			0x0000011200000111ULL
#define	PA_MAP_17_18			0x0000011400000113ULL
#define	PA_MAP_19_20			0x0000011600000115ULL
#define	PA_MAP_21_22			0x0000011800000117ULL
#define	PA_MAP_23_24			0x0000011c00000119ULL
#define	PA_MAP_25_26			0x0000011e0000011dULL
#define PA_MAP_27_28			0x000001200000011fULL
#define	PA_MAP_29_30			0x0000012200000121ULL
#define	PA_MAP_31_32			0x0000000000000123ULL
#define	PA_MAP_33_34			0x0000000000000000ULL
#define	PA_MAP_35_36			0x0000000000000000ULL

// HMTT Physical Address Trace Config Registers
#define	PA_TRACE_CFG			0x000000000001000fULL
