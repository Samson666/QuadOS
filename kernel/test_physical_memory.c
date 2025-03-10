#include "types.h"

void test_physical_memory(void)
{
    uint32_t adr;


    for(adr=0; adr<16000; adr++)
    {
        *(uint32_t*)adr=0x22;
    }
}