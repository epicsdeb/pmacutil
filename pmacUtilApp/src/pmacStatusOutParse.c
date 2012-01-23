
#include <stdio.h>
#include <genSubRecord.h>
#include <epicsExport.h>
#include <registryFunction.h>

long parsePlcBitString(struct genSubRecord *pgsub)
{
    static int pcount = 0; /* processing count */
    long bits1;       /* holds 16 bits for plc 0 .. plc 15 */
    long bits2;       /* holds 16 bits for plc 16.. plc 31 */
    short int *val;

    int DEBUG = 0;    /* debug flag 0=off */
    const int NBITS = 32; /* values read for 32 PLC*/
    int i;

    long mask = 0x00000001; /* bit number 1 */
                            /* where most significant bit is bit 32 */
    
    bits1 = 0x0;
    bits2 = 0x0;
    val = (short int *)pgsub->a;
    for (i = 0; i < NBITS / 2; i++) {
        if (val[i] != 0) bits1 |= mask;
        if (val[i+16] != 0) bits2 |= mask;
        mask <<= 1;
    }
    if (DEBUG) {
        for ( i = 0; i < NBITS; i++) {
            printf("%d", val[i]);
            /* separate on 4 bit bunches for comparison to hex digits*/
            if ( (i+1) % 4 == 0)
                printf(" ");
            if ( i == 15)
                printf("::");
        }
        printf(" MSW=0x%lX LSW=0x%lX\n", bits1, bits2); 
    }

    /* store the values to the output fields */
    *(long *)(pgsub->vala) = bits1;
    *(long *)(pgsub->valb) = bits2;

    if (DEBUG) {
        pcount++;
        printf("Processing parsePlcBitString: %d\n", pcount);
    }
    return 0;
}

long parseProgBitString(struct genSubRecord *pgsub)
{
    static int pcount = 0; /* processing count */
    long bits1;       /* holds 16 bits for coordinate systems 1 .. 16 */
    short int *val;

    int DEBUG = 0;    /* debug flag 0=off */
    const int NBITS = 16; 
    int i;

    long mask = 0x00000001; /* bit number 1 */
                            /* where most significant bit is bit 16 */
    
    bits1 = 0x0;
    val = (short int *)pgsub->a;
    for (i = 0; i < NBITS ; i++) {
        if (val[i] != 0) bits1 |= mask;
        mask <<= 1;
    }
    if (DEBUG) {
        for ( i = 0; i < NBITS; i++) {
            printf("%d", val[i]);
            /* separate on 4 bit bunches for comparison to hex digits*/
            if ( (i+1) % 4 == 0)
                printf(" ");
        }
        printf("VAL=0x%lX\n", bits1); 
    }

    /* store the values to the output fields */
    *(long *)(pgsub->vala) = bits1;

    if (DEBUG) {
        pcount++;
        printf("Processing parseProgBitString: %d\n", pcount);
    }
    return 0;
}

long parseGPIOBitString(struct genSubRecord *pgsub)
{
    static int pcount = 0; /* processing count */
    long bits1;       /* holds 16 bits for coordinate systems 1 .. 16 */
    short int *val;

    int DEBUG = 0;    /* debug flag 0=off */
    const int NBITS = 16;
    int i;

    long mask = 0x00000001; /* bit number 1 */
                            /* where most significant bit is bit 16 */

    bits1 = 0x0; 
    val = (short int *)pgsub->a; 
    for (i = 0; i < NBITS ; i++) {
        if (val[i] != 0) bits1 |= mask;
        mask <<= 1;
    }
    if (DEBUG) {
        for ( i = 0; i < NBITS; i++) {
            printf("%d", val[i]);
            /* separate on 4 bit bunches for comparison to hex digits*/
            if ( (i+1) % 4 == 0)
                printf(" ");
        }
        printf("VAL=0x%lX\n", bits1);
    }

    /* store the values to the output fields */
    *(long *)(pgsub->vala) = bits1;

    if (DEBUG) {
        pcount++;
        printf("Processing GPIOBitString: %d\n", pcount);
    }
    return 0;
}


epicsRegisterFunction( parsePlcBitString );
epicsRegisterFunction( parseProgBitString );
epicsRegisterFunction( parseGPIOBitString );
