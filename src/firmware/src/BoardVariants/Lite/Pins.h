#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_BOARDVARIANTS_LITE_PINS_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_BOARDVARIANTS_LITE_PINS_H

#define PINS_NUMBER_CONFIGURABLE 15

#define UNCONFIGURABLE_A_MASK 0
#define CONFIGURABLE_A_MASK (RA(0) | RA(1) | RA(2) | RA(3) | RA(4))

#define DEFAULT_ANSELA_MASK (RA(0) | RA(1) | RA(2) | RA(3))
#define DEFAULT_TRISA_MASK (RA(0) | RA(1) | RA(2) | RA(3) | RA(4))
#define DEFAULT_LATA_MASK 0
#define DEFAULT_ODCA_MASK 0
#define DEFAULT_CNPUA_MASK 0
#define DEFAULT_CNPDA_MASK (RA(0) | RA(1) | RA(2) | RA(3) | RA(4))

#define UNCONFIGURABLE_B_MASK (RB(6) | RB(10) | RB(11) | RB(13) | RB(14) | RB(15))
#define CONFIGURABLE_B_MASK (RB(0) | RB(1) | RB(2) | RB(3) | RB(4) | RB(5) | RB(7) | RB(8) | RB(9))

#define DEFAULT_ANSELB_MASK (RB(0) | RB(1) | RB(2) | RB(3) | RB(4) | RB(14) | RB(15))
#define DEFAULT_TRISB_MASK (RB(0) | RB(1) | RB(2) | RB(3) | RB(4) | RB(5) | RB(6) | RB(7) | RB(8) | RB(9) | RB(10) | RB(11) | RB(14) | RB(15))
#define DEFAULT_LATB_MASK (RB(6))
#define DEFAULT_ODCB_MASK (RB(6))
#define DEFAULT_CNPUB_MASK 0
#define DEFAULT_CNPDB_MASK (RB(0) | RB(1) | RB(2) | RB(3) | RB(4) | RB(5) | RB(7) | RB(8) | RB(9))

#define UNCONFIGURABLE_C_MASK 0
#define CONFIGURABLE_C_MASK RC(9)

#define DEFAULT_ANSELC_MASK 0
#define DEFAULT_TRISC_MASK RC(9)
#define DEFAULT_LATC_MASK 0
#define DEFAULT_ODCC_MASK 0
#define DEFAULT_CNPUC_MASK 0
#define DEFAULT_CNPDC_MASK (RC(9))

#define CONFIGURABLE_RPOR0_MASK (_RPOR0_RP1R_MASK | _RPOR0_RP2R_MASK | _RPOR0_RP3R_MASK | _RPOR0_RP4R_MASK)
#define CONFIGURABLE_RPOR1_MASK (_RPOR1_RP5R_MASK | _RPOR1_RP6R_MASK | _RPOR1_RP7R_MASK | _RPOR1_RP8R_MASK)
#define CONFIGURABLE_RPOR2_MASK (_RPOR2_RP9R_MASK | _RPOR2_RP10R_MASK | _RPOR2_RP11R_MASK | _RPOR2_RP12R_MASK)
#define CONFIGURABLE_RPOR3_MASK (_RPOR3_RP13R_MASK | _RPOR3_RP14R_MASK)
#define CONFIGURABLE_RPOR4_MASK (_RPOR4_RP18R_MASK)

#define PINS_CONFIGURABLE_AS_ARRAY \
	{ \
		{ .bank = 0, .index = 0 }, \
		{ .bank = 0, .index = 1 }, \
		{ .bank = 0, .index = 2 }, \
		{ .bank = 0, .index = 3 }, \
		{ .bank = 0, .index = 4 }, \
		{ .bank = 1, .index = 0 }, \
		{ .bank = 1, .index = 1 }, \
		{ .bank = 1, .index = 2 }, \
		{ .bank = 1, .index = 3 }, \
		{ .bank = 1, .index = 4 }, \
		{ .bank = 1, .index = 5 }, \
		{ .bank = 1, .index = 7 }, \
		{ .bank = 1, .index = 8 }, \
		{ .bank = 1, .index = 9 }, \
		{ .bank = 2, .index = 9 } \
	}

#define PINS_ASSIGNMENTS_AS_ARRAY \
	{ \
		{ .pin = PACK_PIN('A', 0), .suspendBehaviour = 0x41, .interface = 0, .args = 0x000000000b000100ull }, \
		{ .pin = PACK_PIN('A', 1), .suspendBehaviour = 0x41, .interface = 0, .args = 0x000000000c000100ull }, \
		{ .pin = PACK_PIN('A', 2), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000000000100ull }, \
		{ .pin = PACK_PIN('A', 3), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000001000100ull }, \
		{ .pin = PACK_PIN('A', 4), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000002000100ull }, \
		{ .pin = PACK_PIN('B', 0), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000003000100ull }, \
		{ .pin = PACK_PIN('B', 1), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000004000100ull }, \
		{ .pin = PACK_PIN('B', 2), .suspendBehaviour = 0x40, .interface = 4, .args = 0ull }, \
		{ .pin = PACK_PIN('B', 3), .suspendBehaviour = 0x40, .interface = 4, .args = 0ull }, \
		{ .pin = PACK_PIN('B', 4), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000007000100ull }, \
		{ .pin = PACK_PIN('B', 5), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000008000100ull }, \
		{ .pin = PACK_PIN('B', 7), .suspendBehaviour = 0x41, .interface = 0, .args = 0x0000000009000100ull }, \
		{ .pin = PACK_PIN('B', 8), .suspendBehaviour = 0x41, .interface = 0, .args = 0x000000000d000100ull }, \
		{ .pin = PACK_PIN('B', 9), .suspendBehaviour = 0x41, .interface = 0, .args = 0x000000000e000100ull }, \
		{ .pin = PACK_PIN('C', 9), .suspendBehaviour = 0x41, .interface = 0, .args = 0x000000000a000100ull } \
	}

#endif
