#ifndef _JTAG_H_
#define _JTAG_H_

void jtag_init(void);
int jtag_rxtx(uint8_t *jtag_rx, uint8_t *jtag_tx);

#endif /* _JTAG_H_ */
