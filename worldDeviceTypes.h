/* Continental Control Systems, Watt Node Modbus */
#define DEVICE_TYPE_WORLD_WNC_BASIC           1  /* 1000, 34 */
#define DEVICE_TYPE_WORLD_WNC_ADVANCED        2
#define DEVICE_TYPE_WORLD_WNC_CONFIGURATION   3
#define DEVICE_TYPE_WORLD_WNC_DIAGNOSTIC      4
#define DEVICE_TYPE_WORLD_WNC_ADVANCED_ENERGY 5  /* 1100, 38 */

/* APRS World, XRW2G */
#define DEVICE_TYPE_WORLD_XRW2G_BASIC         100 /* 0, 46 */
#define DEVICE_TYPE_WORLD_XRW2G_DIAGNOSTIC    101
#define DEVICE_TYPE_WORLD_XRW2G_CONFIGURATION 102

/* Morningstar, Relay Driver */
#define DEVICE_TYPE_WORLD_RD1_RAM             200 /* 0, 18 */
#define DEVICE_TYPE_WORLD_RD1_COILS           201 /* 0, 4 */

/* Morningstar, Tri-Star */
#define DEVICE_TYPE_WORLD_TS_RAM              300 /* 8, 21 */
#define DEVICE_TYPE_WORLD_TS_COILS            301
#define DEVICE_TYPE_WORLD_TS_EEPROM           302
#define DEVICE_TYPE_WORLD_TS_KWH              303 /* 57388, 3 */

/* Morningstar, Tri-Star MPPT */
#define DEVICE_TYPE_WORLD_TSMPPT_SCALING                  320 /* 0, 4 */
#define DEVICE_TYPE_WORLD_TSMPPT_FILTERED_ADC             321 /* 24, 38 */

/* Advancetech, Adam 4xxx MODBUS */
#define ADAM_4XXX_ANALOG_CURRENT                          400 /* 0, 8 */

/* smaller block definitions for SatCon PVS50 ... not implemented or seen in the wild. Do not use */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_SOFTWARE           500 /* 30009, 3 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_ACTIVE_FAULTS      501 /* 30017, 3 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_DC                 502 /* 30020, 4 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_AC_LINE_CURRENT    503 /* 30035, 2 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_AC_LINE_VOLTAGE    504 /* 30040, 2 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_AC_POWER           505 /* 30043, 5 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_GROUND_IMPEDANCE   506 /* 30049, 1 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_STRING_CURRENT     507 /* 30050, 32 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_STRING_CURRENT_AVG 508 /* 30089, 1 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_STRING_KWH         509 /* 30090, 32 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_STRING_KWH_AVG     510 /* 30129, 1 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_KWH                511 /* 30131, 8 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_TIMERS             512 /* 30275, 2 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_STATE              513 /* 30279, 2 */

/* SatCon PVS50 inverter. Manual has +30001 offset. */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_0      520 /* 9, 40 registers ... error, should be 41 */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_1      521 /* 50, 40 registers */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_2      522 /* 90, 40 registers */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_3      523 /* 131, 8 registers */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_4      524 /* 175, 2 registers ... not programmed at west point! */
#define DEVICE_TYPE_WORLD_SATCON_PVS50_5      525 /* 275, 6 registers ... */


/* PC&S Multitek MP850MPDV19PE CUSTOM 1 */
/* Registers re-arranged to be consecutive, starting with reg 0 */
/* Values are 32-bit Floating Point */
/* Register list: */
/* 0 - V 1 ... Voltage */
/* 2 - I 1 ... Current */
/* 4 - kW Sum */
/* 6 - kWHr (Import) */
/* 8 - kWd  (Import) ... Demand over last 3 minutes */
/* 10 - Ad ... Demand current over last 3 minutes*/
/* 12 - kWHr (Export) */
/* 14 - A.h */
/* 16 - Max kWd (Import) */
/* 18 - Max Ad */

#define DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1	 600 /* 0, 20 registers */


#define DEVICE_TYPE_WORLD_SHUNTMONITOR_BASIC  700 /* 0, 32 registers */

#define DEVICE_TYPE_WORLD_WRENDAQ4_BASIC      800 /* 0, 39 registers */
#define DEVICE_TYPE_WORLD_WRENDAQ4_CONFIG     801 /* 1000, 15 registers */

/* Midnite Solar Classic - based on 2011-05-21 specification */
#define DEVICE_TYPE_WORLD_CLASSIC_BASIC       900 /* 4113, 31 registers */
#define DEVICE_TYPE_WORLD_CLASSIC_WIND_CURVE  901 /* 4300, 16 registers */
#define DEVICE_TYPE_WORLD_CLASSIC_RESTING     902 /* 4274, 3  registers */

/* APRS World POE Monitor */
#define DEVICE_TYPE_WORLD_POEMONITOR_BASIC    1000 
#define DEVICE_TYPE_WORLD_POEMONITOR_CONFIG   1001

/* APRS World watchdog Arlo */
#define DEVICE_TYPE_WORLD_WATCHDOGARLO_BASIC  1100
#define DEVICE_TYPE_WORLD_WATCHDOGARLO_CONFIG 1101

/* China HUM temperature and relative humidity sensor */
#define DEVICE_TYPE_WORLD_HUM_TEMPERATURE      1200
#define DEVICE_TYPE_WORLD_HUM_RELATIVEHUMIDITY 1201

/* "Center" brand soundmeter */
#define TYPE_TYPE_WORLD_SOUNDMETER_CENTER      1300

/* PS2Tap */
#define DEVICE_TYPE_WORLD_PS2TAP_BASIC         1400 /* 0, 19 registers */

/* Veris E50 Series Power Meter */
#define DEVICE_TYPE_WORLD_VERISE50_BASIC       1500 /* 256, 54 registers */


/* Note: List not complete.  Reference master list in WorldDataCollector. */


/* generic rsdata */
#define DEVICE_TYPE_WORLD_RSDATA               32000 /* Integer */
#define DEVICE_TYPE_WORLD_RSDATA_FLOAT         32001 /* IEEE-754 single precision floating point (32-bit) data */
#define DEVICE_TYPE_WORLD_RSDATA_FLOAT_LITTLE  32002 