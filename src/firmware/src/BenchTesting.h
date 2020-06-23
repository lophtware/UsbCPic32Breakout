#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_BENCHTESTING_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_BENCHTESTING_H
#ifdef ENABLE_BENCH_TESTING

extern void benchTestingInitialise(void);
extern void benchTestingSendRawCommand(uint32_t command);
extern void benchTestingToggleAllPinsForSeconds(uint16_t seconds);
extern void benchTestingToggleAllPinsWithPwmForSeconds(uint16_t seconds, uint8_t period);

#else

static inline void benchTestingInitialise(void) { }

#endif
#endif
