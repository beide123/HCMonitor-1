
//Size of memory in the FPGA, used to config the registers in FPGA
#define	fpga_ram_size  0x100000                //1MB

// HMTT Control Registers
#define	PA_TRACE_CTRL_ADDR		0x0000

// HMTT Config Registers
#define	PA_TRACE_CFG_ADDR		0x0100

// HMTT Physical Address Map Registers
#define	PA_MAP_1_2_ADDR			0x0200
#define	PA_MAP_3_4_ADDR			0x0300
#define	PA_MAP_5_6_ADDR			0x0400
#define	PA_MAP_7_8_ADDR			0x0500
#define	PA_MAP_9_10_ADDR		0x0600
#define	PA_MAP_11_12_ADDR		0x0700
#define	PA_MAP_13_14_ADDR		0x0800
#define	PA_MAP_15_16_ADDR		0x0900
#define	PA_MAP_17_18_ADDR		0x0a00
#define	PA_MAP_19_20_ADDR		0x0b00
#define	PA_MAP_21_22_ADDR		0x0c00
#define	PA_MAP_23_24_ADDR		0x0d00
#define	PA_MAP_25_26_ADDR		0x0e00
#define	PA_MAP_27_28_ADDR		0x0f00
#define	PA_MAP_29_30_ADDR		0x1000
#define	PA_MAP_31_32_ADDR		0x1100
#define	PA_MAP_33_34_ADDR		0x1200
#define	PA_MAP_35_36_ADDR		0x1300

// HMTT Config Space Range Registers
#define	CFG_START_ADDR			0x1400
#define	CFG_END_ADDR			0x1500

// HMTT Valid Trace Range Registers
#define	PA_RANGE_START_ADDR		0x1600
#define	PA_RANGE_END_ADDR		0x1700

// HMTT DMA Buffer Registers
#define	DMA_BUF_ADDR_ADDR		0x2000
#define	DMA_BUF_SIZE_ADDR		0x2100
#define	DMA_REG_ADDR_ADDR		0x2200
#define	DMA_SEG_SIZE_ADDR		0x2300
