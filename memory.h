#ifndef __MEMORY_H_
#define __MEMORY_H_

uint8_t mem_read8(uint16_t location);
uint16_t mem_read16(uint16_t location);
void mem_write8(uint16_t location, uint8_t data);
void mem_write16(uint16_t location, uint8_t data);

#endif /*!__MEMORY_H_*/
