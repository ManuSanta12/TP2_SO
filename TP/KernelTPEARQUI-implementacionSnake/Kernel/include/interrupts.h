#ifndef INTERRUPS_H_
#define INTERRUPS_H_

#include <stdint.h>

void interrupt_keyboard(void);
void interrupt_timerTick(void);
void interrupt_syscall(void);
void exception_invalidOpCode(void);
void exception_divideByZero(void);



void _cli(void);

void _sti(void);

void _hlt(void);

void picMasterMask(uint8_t mask);

void picSlaveMask(uint8_t mask);

//Termina la ejecución de la cpu.
void haltcpu(void);

#endif /* INTERRUPS_H_ */