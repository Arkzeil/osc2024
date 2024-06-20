#include "bcm2837/rpi_mbox.h"
#include "mbox.h"
#include "uart1.h"

/* Aligned to 16-byte boundary while we have 28-bits for VC */
volatile unsigned int  __attribute__((aligned(16))) pt[64];

int mbox_call( mbox_channel_type channel, unsigned int value )
{
    // Add channel to lower 4 bit
    value &= ~(0xF);
    value |= channel;
    while ( (*MBOX_STATUS & BCM_ARM_VC_MS_FULL) != 0 ) {}
    // Write to Register
    *MBOX_WRITE = value;
    uart_sendline("%x ", MBOX_WRITE);
    uart_sendline("%x\n", *MBOX_WRITE);
    
    while(1) {
        while ( *MBOX_STATUS & BCM_ARM_VC_MS_EMPTY ) {}
        // Read from Register
        if (value == *MBOX_READ){
            uart_sendline("%x ", MBOX_READ);
            uart_sendline("%x ", *MBOX_READ);
            uart_sendline("%d\n", value);
            uart_sendline("%d ", pt[1]);
            uart_sendline("%d ", MBOX_REQUEST_SUCCEED);
            uart_sendline("%d\n", pt[1] == MBOX_REQUEST_SUCCEED);
            return pt[1] == MBOX_REQUEST_SUCCEED;
        }
    }
    return 0;
}

