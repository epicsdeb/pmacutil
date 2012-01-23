#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <iocsh.h>
#include <drvSup.h>
#include <epicsExport.h>
#include <epicsMath.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <cantProceed.h>

#include <asynDriver.h>
#include <asynOctetSyncIO.h>
#include <asynStandardInterfaces.h>

#define MAX_NTABLES 32
#define TABLE_NBYTES 1024   /* Number of bytes per table command (command is ascii string format) */
#define PMAC_COMM_BYTES 512
#define ROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define snprintf epicsSnprintf

typedef enum {
    ct_entries,             /* int32. Set number of entries in the table */
    ct_source,              /* int32. Set motor axis number of the source motor */
    ct_desired,             /* int32. Set whether to use desired or actual source position for compensation 0=actual, 1=desired */
    ct_target,              /* int32. Set motor axis number for the target motor to apply the table to */
    ct_count_length,        /* int32. Travel range of source (from 0) to apply the table to (in counts) 
                               Float64: range in engineering units */
    
    ct_target_mres,         /* float64. Target motor resolution in count/EGU */
    ct_source_mres,         /* float64. Source motor resolution in count/EGU */
    ct_table,               /* int32Array [R] The compensation table (list of target compensations) in counts
                               Float64Array [RW] The compensation table in engineering units */
    ct_source_table,        /* int32Array [R] The source step table in count (linear, based on range and number of elements in compensation)
                               Float64Array [R] The source step table in engineering units */
    ct_apply,               /* int32 [RW] write: start the download of the table to the pmac.  */
    ct_enable,              /* int32 [RW] enable (1) and disable (1) the given table */
    ct_global_enable,       /* int32 [RW] globally (on the PMAC) enable or disable the compensation tables (I51) */
    ct_table_wrap,          /* int32 [RW] globally (on the PMAC) enable or disable the table wrap around (I30) */
    CT_MAX_CMD
    } ct_command_t;
typedef struct {
	ct_command_t command;
	char *commandString;
} ct_command_struct_t;

static ct_command_struct_t ct_commands[CT_MAX_CMD] = {
    { ct_entries,      "CT_ENTRIES" },
    { ct_source,       "CT_SOURCE" },
    { ct_desired,      "CT_DESIRED" },
    { ct_target,       "CT_TARGET" },
    { ct_count_length, "CT_COUNT_LENGTH" },

    { ct_target_mres,  "CT_TARGET_MRES" },
    { ct_source_mres,  "CT_SOURCE_MRES" },
    
    { ct_table,        "CT_TABLE" },
    { ct_source_table, "CT_SOURCE_TABLE" },
    { ct_apply,        "CT_APPLY" },
    { ct_enable,       "CT_ENABLE" },
    { ct_global_enable,"CT_GLOBAL_ENABLE" },
    { ct_table_wrap,   "CT_TABLE_WRAP" },    
};


typedef struct compensationtable_t {
    char* portName;
    asynUser *pasynUser;
    char* octetPortName;
    asynUser *pasynUserOctet;
    asynStandardInterfaces asynStdInterfaces;
    
    int nTables;
    char *tableDefCmd;
    unsigned int i51;
    unsigned int i30;
    char *pmacSendBuf;      /* small allocated buffers for small PMAC write/reads */
    char *pmacRecvBuf;
    
    unsigned int enable[MAX_NTABLES];
    unsigned int entries[MAX_NTABLES];
    unsigned int source[MAX_NTABLES];
    unsigned int desired[MAX_NTABLES];
    unsigned int target[MAX_NTABLES];
    int count_length[MAX_NTABLES];
    double source_range[MAX_NTABLES];
    
    double target_mres[MAX_NTABLES];
    double source_mres[MAX_NTABLES];
    
    int *count_table[MAX_NTABLES];
    double *egu_table[MAX_NTABLES];
    
    double *source_egu_table[MAX_NTABLES];
    
    char *define_comp_cmd[MAX_NTABLES];
    } compensationtable_t;

void ct_report(void *drvPvt, FILE *fp, int details);
asynStatus ct_connect(void *drvPvt,asynUser *pasynUser);
asynStatus ct_disconnect(void *drvPvt,asynUser *pasynUser);

asynStatus ct_drvUserCreate  ( void *drvPvt, asynUser *pasynUser, const char *drvInfo, const char **pptypeName,size_t *psize);
asynStatus ct_drvUserGetType ( void *drvPvt, asynUser *pasynUser, const char **pptypeName,size_t *psize);
asynStatus ct_drvUserDestroy ( void *drvPvt, asynUser *pasynUser);

asynStatus ct_writeInt32     ( void *drvPvt, asynUser *pasynUser, epicsInt32 value);
asynStatus ct_readInt32      ( void *drvPvt, asynUser *pasynUser, epicsInt32 *value);
asynStatus ct_getBounds      ( void *drvPvt, asynUser *pasynUser, epicsInt32 *low, epicsInt32 *high);

asynStatus ct_writeFloat64   ( void *drvPvt, asynUser *pasynUser, epicsFloat64 value);
asynStatus ct_readFloat64    ( void *drvPvt, asynUser *pasynUser, epicsFloat64 *value);

asynStatus ct_writeFloat64Array ( void *drvPvt, asynUser *pasynUser, epicsFloat64 *data, size_t nElements );
asynStatus ct_readFloat64Array  ( void *drvPvt, asynUser *pasynUser, epicsFloat64 *data, size_t maxElements, size_t *nElements );

asynStatus ct_writeInt32Array   ( void *drvPvt, asynUser *pasynUser, epicsInt32 *data, size_t nElements );
asynStatus ct_readInt32Array    ( void *drvPvt, asynUser *pasynUser, epicsInt32 *data, size_t maxElements, size_t *nElements );


/* Calculate the array of compensation points on the source motors travel range based on length of table and range of motion */
void ct_sourcePoints( compensationtable_t *pPvt, asynUser *pasynUser );
void ct_defineCompCmd( compensationtable_t *pPvt, asynUser *pasynUser, unsigned int table );
void ct_allocTables( compensationtable_t *pPvt, asynUser *pasynUser, size_t nElements );
void addrToIndex( compensationtable_t *pPvt, asynUser *pasynUser, unsigned int *index );
asynStatus ct_generateDefCmd( compensationtable_t *pPvt, asynUser *pasynUser );
asynStatus ct_writeTables( compensationtable_t *pPvt, asynUser *pasynUser );

asynStatus ct_readSettings( compensationtable_t *pPvt, asynUser *pasynUser );
asynStatus ct_deleteBuffers( compensationtable_t *pPvt, asynUser *pasynUser );

asynStatus ctAsynConnect( const char * port, asynUser ** ppasynUser );
asynStatus ctWriteRead( compensationtable_t *pPvt, asynUser *pasynUser, char * command, size_t reply_buff_size, char * response );


/* Structures with function pointers for each of the asyn interfaces */
static asynCommon ifaceCommon = {
    ct_report,    /* report */
    ct_connect,   /* connect */
    ct_disconnect /* disconnect*/
};

static asynDrvUser ifaceDrvUser = {
    ct_drvUserCreate,
    ct_drvUserGetType,
    ct_drvUserDestroy
};

static asynInt32 ifaceInt32 = {
    ct_writeInt32,
    ct_readInt32,
    ct_getBounds
};

static asynFloat64 ifaceFloat64 = {
    ct_writeFloat64,
    ct_readFloat64
};

static asynFloat64Array ifaceFloat64Array = {
    ct_writeFloat64Array,
    ct_readFloat64Array
};

static asynInt32Array ifaceInt32Array = {
    ct_writeInt32Array,
    ct_readInt32Array
};

/*static asynOctet ifaceOctet = {
    NULL,
    NULL,
    ct_readOctet,
};*/

/* asynCommon methods */
void ct_report(void *drvPvt, FILE *fp, int details)
{
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    
    fprintf(fp, "=================================================\n");
    fprintf(fp, "    Compensation table report port: %s\n", pPvt->portName);
    fprintf(fp, "    Number of tables: %d\n", pPvt->nTables);
    fprintf(fp, "=================================================\n");
    
    for (addr = 0; addr < pPvt->nTables; addr++)
    {
        fprintf(fp, "Table no.: %d\n", addr+1);
        fprintf(fp, "\tentries:          %d\n", pPvt->entries[addr]);
        fprintf(fp, "\tsource:           %d\n", pPvt->source[addr]);
        fprintf(fp, "\tdesired:          %d\n", pPvt->desired[addr]);
        fprintf(fp, "\ttarget:           %d\n", pPvt->target[addr]);
        fprintf(fp, "\tcount_length:     %d\n", pPvt->count_length[addr]);
        fprintf(fp, "\tsource_range:     %lf\n", pPvt->source_range[addr]);
        fprintf(fp, "\ttarget_mres:      %lf\n", pPvt->target_mres[addr]);
        fprintf(fp, "\tsource_mres:      %lf\n", pPvt->source_mres[addr]);
        fprintf(fp, "\tcount_table:      %p\n", pPvt->count_table[addr]);
        fprintf(fp, "\tegu_table:        %p\n", pPvt->egu_table[addr]);
        fprintf(fp, "\tsource_egu_table: %p\n", pPvt->source_egu_table[addr]);
        fprintf(fp, "\tdefine command:   \'%s\'\n", pPvt->define_comp_cmd[addr]);
    }
}

asynStatus ct_connect(void *drvPvt, asynUser *pasynUser)
{
    compensationtable_t *pPvt = (compensationtable_t *)pPvt;
    int addr, yesNo;
    const char *portName;
    pasynManager->getPortName(pasynUser, &portName);
    pasynManager->getAddr(pasynUser, &addr);
    pasynManager->isConnected(pasynUser,&yesNo);
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "ct_connect: addr: %d port: %s connected: %d pPvt: %p\n",
            addr, portName, yesNo, pPvt);
	pasynManager->exceptionConnect(pasynUser);
	return(asynSuccess);
}

asynStatus ct_disconnect(void *drvPvt, asynUser *pasynUser)
{
    compensationtable_t *pPvt = (compensationtable_t *)pPvt;
    int addr, yesNo;
    const char *portName;
    pasynManager->getAddr(pasynUser, &addr);
    pasynManager->getPortName(pasynUser, &portName);
    pasynManager->isConnected(pasynUser,&yesNo);
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "ct_disconnect: addr: %d port: %d connected: %d pPvt: %p\n",
            addr, portName, yesNo, pPvt);
	pasynManager->exceptionDisconnect(pasynUser);
	return(asynSuccess);
}


/* asynDrvUser methods */
asynStatus ct_drvUserCreate(void *drvPvt, asynUser *pasynUser, const char *drvInfo, const char **pptypeName, size_t *psize)
{
    int i;
    char *pstring;
    const char *functionName = "ct_drvUserCreate";

    asynPrint(	pasynUser, ASYN_TRACE_FLOW,
    "%s: attempting to create cmd: %s (CT_MAX_CMD: %d)\n", functionName, drvInfo, CT_MAX_CMD);

    for (i=0; i < CT_MAX_CMD; i++)
    {
        pstring = ct_commands[i].commandString;
        if (epicsStrCaseCmp( drvInfo, pstring) == 0)
        {
            pasynUser->reason = ct_commands[i].command;
            if (pptypeName)
                *pptypeName = epicsStrDup(pstring);
            if (psize)
                *psize = sizeof( ct_commands[i].command);
            asynPrint(	pasynUser, ASYN_TRACE_FLOW,
                        "%s: command created: %s\n", functionName, pstring);
            return( asynSuccess );
        }
    }
    asynPrint(	pasynUser, ASYN_TRACE_ERROR,
                "%s: unknown command: %s\n", functionName, drvInfo);
    return( asynError );			
}

asynStatus ct_drvUserGetType(void *drvPvt, asynUser *pasynUser, const char **pptypeName, size_t *psize)
{
    int command = pasynUser->reason;
    *psize = 0;
    if ( pptypeName )
        *pptypeName = epicsStrDup( ct_commands[command].commandString);
    if ( psize )
        *psize = sizeof( command );
    return( asynSuccess );
}

asynStatus ct_drvUserDestroy(void *drvPvt, asynUser *pasynUser)
{
    asynPrint(	pasynUser, ASYN_TRACE_FLOW,
                "ct_drvUserDestroy: Function not implemented\n");
    return( asynSuccess );
}


asynStatus ct_getBounds      ( void *drvPvt, asynUser *pasynUser, epicsInt32 *low, epicsInt32 *high)
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    const char *functionName = "ct_getBounds";
    pasynManager->getAddr(pasynUser, &addr);
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: addr=%d port=%s\n",
                functionName, addr, pPvt->portName);
    switch( pasynUser->reason )
    {
       case ct_source:
       case ct_target:
            *low = 1;
            *high = 32;
            break;
       case ct_desired:
       case ct_enable:
       case ct_global_enable:
       case ct_table_wrap:
            *low = 0;
            *high = 1;
            break;
       default:
            *low = 0;
            *high = 0;
            break;
    }
    return status;
}



/***********************************************************************************************/



asynStatus ct_writeInt32( void *drvPvt, asynUser *pasynUser, epicsInt32 value)
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    unsigned int index;
    const char *functionName = "ct_writeInt32";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: val=%d addr=%d port=%s\n",
                functionName, value, addr, pPvt->portName);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
       case ct_entries:
            pPvt->entries[index] = value;
            break;
       case ct_source:
            pPvt->source[index] = value;
            break;
       case ct_desired:
            pPvt->desired[index] = value;
            break;
       case ct_target:
            pPvt->target[index] = value;
            break;
       case ct_enable:
            pPvt->enable[index] = value;
            break;
       case ct_global_enable:
            snprintf( pPvt->pmacSendBuf, PMAC_COMM_BYTES, "I51=%d", value );
            status = ctWriteRead( pPvt, pasynUser, pPvt->pmacSendBuf, PMAC_COMM_BYTES, pPvt->pmacRecvBuf );
            if (status !=asynError) pPvt->i51 = value;
            break;
       case ct_table_wrap:
            snprintf( pPvt->pmacSendBuf, PMAC_COMM_BYTES, "I30=%d", value );
            status = ctWriteRead( pPvt, pasynUser, pPvt->pmacSendBuf, PMAC_COMM_BYTES, pPvt->pmacRecvBuf );
            if (status !=asynError) pPvt->i30 = value;
            break;
       case ct_apply:
            status = ct_generateDefCmd( pPvt, pasynUser );
            status = ct_writeTables( pPvt, pasynUser );
            break;
       default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    return status;
}

asynStatus ct_readInt32( void *drvPvt, asynUser *pasynUser, epicsInt32 *value)
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    unsigned int index;
    const char *functionName = "ct_readInt32";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: addr=%d port=%s\n",
                functionName, addr, pPvt->portName);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
       case ct_entries:
            *value = pPvt->entries[index];
            break;
       case ct_source:
            *value = pPvt->source[index];
            break;
       case ct_desired:
            *value = pPvt->desired[index];
            break;
       case ct_target:
            *value = pPvt->target[index];
            break;
        case ct_enable:
            *value = pPvt->enable[index];
            break;
       case ct_global_enable:
            status = ct_readSettings( pPvt, pasynUser );
            *value = pPvt->i51;
            break;
       case ct_table_wrap:
            status = ct_readSettings( pPvt, pasynUser );
            *value = pPvt->i30;
            break;
        case ct_apply:
            break;
        default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    return status;
}

asynStatus ct_writeFloat64( void *drvPvt, asynUser *pasynUser, epicsFloat64 value)
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    unsigned int index;
    double dtmp;
    long itmp;
    const char *functionName = "ct_writeFloat64";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: val=%f addr=%d port=%s reason=%d\n",
                functionName, value, addr, pPvt->portName, pasynUser->reason);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
        case ct_target_mres:
            pPvt->target_mres[index] = value;
            break;
        case ct_source_mres:
            pPvt->source_mres[index] = value;
            asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: val=%f source_mres=%f\n", 
                        functionName, value, pPvt->source_mres[index]);
            break;
        case ct_count_length:
            pPvt->source_range[index] = value;
            dtmp = value / pPvt->source_mres[index];
            asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s: range=%lf mres=%lf counts=%lf\n",
                        functionName, value, pPvt->source_mres[index], dtmp);
            itmp = ROUND(dtmp);
            asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s: Rounded counts = %li int counts= %ld\n",
                      functionName, ROUND(dtmp), itmp);
            pPvt->count_length[index] = (int)itmp;
            break;
        default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    return status;
}

asynStatus ct_readFloat64( void *drvPvt, asynUser *pasynUser, epicsFloat64 *value)
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    unsigned int index;
    const char *functionName = "ct_readFloat64";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: addr=%d port=%s reason=%d\n",
                functionName, addr, pPvt->portName, pasynUser->reason);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
        case ct_target_mres:
            *value = pPvt->target_mres[index];
            break;
        case ct_source_mres:
            *value = pPvt->source_mres[index];
            break;
        case ct_count_length:
            *value = pPvt->source_range[index];
            break;
        default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    return status;
}

asynStatus ct_writeFloat64Array( void *drvPvt, asynUser *pasynUser, epicsFloat64 *data, size_t nElements )
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr, i;
    unsigned int index;
    epicsFloat64 *table;
    epicsInt32 * counts;
    const char *functionName = "ct_writeFloat64Array";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: addr=%d port=%s nitems=%d\n",
                functionName, addr, pPvt->portName, nElements);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
        case ct_table:
            if (pPvt->egu_table[index] == NULL) 
            {
                pPvt->egu_table[index] = calloc(nElements, sizeof(epicsFloat64) );
            }
            table = pPvt->egu_table[index];
            /* copy the array of doubles to the EGU table */
            memcpy( (void*)table, (void*)data, nElements * sizeof(epicsFloat64) );

            /* translate the EGU table to a table of counts that we can send to the pmac */
            counts = pPvt->count_table[index];
            for (i=0; i<nElements; i++) 
                counts[i] = (epicsInt32)(ROUND( 16.0 * table[i] / pPvt->target_mres[index] ) );
            break;

        default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    return status;
}

asynStatus ct_readFloat64Array( void *drvPvt, asynUser *pasynUser, epicsFloat64 *data, size_t maxElements, size_t *nElements )
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    unsigned int index;
    double *table;
    double **pTable;
    int nord;
    const char *functionName = "ct_readFloat64Array";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: addr=%d port=%s n=%d table=%p source=%p\n",
                functionName, addr, pPvt->portName, maxElements, 
                pPvt->egu_table[addr-1], pPvt->source_egu_table[addr-1]);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
        case ct_table:
            pTable = &(pPvt->egu_table[index]);
            if (*pTable == NULL) ct_allocTables( pPvt, pasynUser, maxElements );
            break;
        case ct_source_table:
            pTable = &(pPvt->source_egu_table[index]);
            if (*pTable == NULL) ct_allocTables( pPvt, pasynUser, maxElements );
            ct_sourcePoints( pPvt, pasynUser );
            break;
        default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    
    if (status == asynSuccess)
    {
        table = *pTable;
        if ( pPvt->entries[index] > maxElements ) nord = maxElements;
        else nord = pPvt->entries[index];
        memcpy( (void*)data, (void*)table, nord*sizeof(epicsFloat64) );
        *nElements = nord;
    }
    return status;
}

asynStatus ct_writeInt32Array( void *drvPvt, asynUser *pasynUser, epicsInt32 *data, size_t nElements )
{
    return asynError;
}

asynStatus ct_readInt32Array( void *drvPvt, asynUser *pasynUser, epicsInt32 *data, size_t maxElements, size_t *nElements )
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt = (compensationtable_t *)drvPvt;
    int addr;
    unsigned int index;
    const char *functionName = "ct_readInt32Array";
    
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: addr=%d port=%s n=%d table=%p source=%p\n",
                functionName, addr, pPvt->portName, maxElements, 
                pPvt->egu_table[addr-1], pPvt->source_egu_table[addr-1]);
    addrToIndex( pPvt, pasynUser, &index );
    switch( pasynUser->reason )
    {
        case ct_table:
            if (pPvt->count_table[index] == NULL) ct_allocTables( pPvt, pasynUser, maxElements );
            break;
        default:
            status = asynError;
            asynPrint( pasynUser, ASYN_TRACE_ERROR, "%s: error unknown reason: %d\n", pasynUser->reason);
            break;
    }
    return status;
}



/***********************************************************************************************/


int ctConfig( const char* port, const char* octetPort, unsigned int nTables )
{
    asynStatus status = asynSuccess;
    compensationtable_t *pPvt;
    asynStandardInterfaces *pInterfaces;
    int t;
    const char* functionName = "ctConfig";
    
    pPvt = callocMustSucceed(1, sizeof( compensationtable_t ), functionName);
    pPvt->portName = epicsStrDup( port );
    pPvt->octetPortName = epicsStrDup( octetPort );

    status = pasynManager->registerPort(pPvt->portName,
                                        ASYN_MULTIDEVICE | ASYN_CANBLOCK,
                                        1,  /*  autoconnect */
                                        0,  /* medium priority */
                                        0); /* default stack size */
    if (status != asynSuccess) {
        printf("%s ERROR: Can't register port %s\n", functionName, pPvt->portName);
        return(asynError);
    }

    /* Create asynUser for debugging */
    pPvt->pasynUser = pasynManager->createAsynUser(0, 0);

    pInterfaces = &pPvt->asynStdInterfaces;
    
    /* Initialize interface pointers */
    pInterfaces->common.pinterface         = (void *)&ifaceCommon;
    pInterfaces->drvUser.pinterface        = (void *)&ifaceDrvUser;
    pInterfaces->int32.pinterface          = (void *)&ifaceInt32;
    pInterfaces->float64.pinterface        = (void *)&ifaceFloat64;
    pInterfaces->float64Array.pinterface   = (void *)&ifaceFloat64Array;
    pInterfaces->int32Array.pinterface     = (void *)&ifaceInt32Array;

    /* Define which interfaces can generate interrupts */
    pInterfaces->int32CanInterrupt          = 1;
    pInterfaces->float64CanInterrupt        = 1;

    status = pasynStandardInterfacesBase->initialize(pPvt->portName, pInterfaces,
                                                     pPvt->pasynUser, pPvt);
    if (status != asynSuccess) {
        printf("%s ERROR: Can't register interfaces: %s.\n",
               functionName, pPvt->pasynUser->errorMessage);
        return(asynError);        
    }
    
    /* try to connect to the PMAC over some serial/ethernet port */
    ctAsynConnect( pPvt->octetPortName, &pPvt->pasynUserOctet );
    

    if (nTables > MAX_NTABLES) nTables = MAX_NTABLES;
    pPvt->nTables = nTables;
    pPvt->tableDefCmd = (char*)calloc( nTables * TABLE_NBYTES, sizeof(char) );
    for (t = 0; t < nTables; t++)
    {
        pPvt->target[t] = t+1;
        pPvt->source[t] = t+1;
        pPvt->enable[t] = 0;
        pPvt->entries[t] = 1;
        pPvt->source_mres[t] = 1.0;
        pPvt->target_mres[t] = 1.0;
        pPvt->count_table[t] = NULL;
        pPvt->egu_table[t] = NULL;
        pPvt->source_egu_table[t] = NULL;
        pPvt->define_comp_cmd[t] = (char*)calloc( 40, sizeof(char) );
        pPvt->pmacSendBuf = (char*)calloc(PMAC_COMM_BYTES, sizeof(char) );
        pPvt->pmacRecvBuf = (char*)calloc(PMAC_COMM_BYTES, sizeof(char) );
    }
    return 0;
}


asynStatus ctAsynConnect( const char * port, asynUser ** ppasynUser )
{
    asynStatus status = asynSuccess;
 
    status = pasynOctetSyncIO->connect( port, 0, ppasynUser, NULL);
    if (status) {
        printf( "compensationtable: unable to connect to port %s\n", 
                port);
        return status;
    }

    status = pasynOctetSyncIO->setInputEos(*ppasynUser, "\006", 1 );
    if (status) {
        asynPrint(*ppasynUser, ASYN_TRACE_ERROR,
                  "compensationtable: unable to set input EOS on %s: %s\n", 
                  port, (*ppasynUser)->errorMessage);
        pasynOctetSyncIO->disconnect(*ppasynUser);
        return status;
    }

    status = pasynOctetSyncIO->setOutputEos(*ppasynUser, "\r", 1);
    if (status) {
        asynPrint(*ppasynUser, ASYN_TRACE_ERROR,
                  "compensationtable: unable to set output EOS on %s: %s\n", 
                  port, (*ppasynUser)->errorMessage);
        pasynOctetSyncIO->disconnect(*ppasynUser);
        return status;
    }

    return status;
}

asynStatus ctWriteRead( compensationtable_t *pPvt, asynUser *pasynUser, char * command, size_t reply_buff_size, char * response )
{
    asynStatus status = asynSuccess;
    const double timeout=5.0;
    size_t nwrite, nread;
    int eomReason;
    unsigned int pmacErrCode;
    int match;
    asynUser * pasynUserOctet = pPvt->pasynUserOctet;
/*    asynUser * pasynUser = pPvt->pasynUser;*/

    asynPrint( pasynUser, ASYN_TRACEIO_DRIVER, 
                "ctWriteRead [%s]\n ======== Sending to PMAC %s command =====\n%s\n================\n",
                pPvt->portName, pPvt->octetPortName, command );
   
    status = pasynOctetSyncIO->writeRead( pasynUserOctet,
                                          command, strlen(command),
                                          response, reply_buff_size,
                                          timeout,
                                          &nwrite, &nread, &eomReason );

    if ( nread != 0 )
        asynPrint( pasynUser, ASYN_TRACEIO_DRIVER, "ctWriteRead [%s]: PMAC %s response: %s\n", 
        pPvt->portName, pPvt->octetPortName, response );

    if (status)
    {
        asynPrint( pasynUserOctet,
                   ASYN_TRACE_ERROR,
                   "ctWriteRead [%s]: Read/write error to PMAC %s command %s. Status=%d, Error=%s\n",
                   pPvt->portName, pPvt->octetPortName, command,
                   status, pasynUserOctet->errorMessage);
        return status;
    }
    
    match = sscanf( response, "ERR%03d", &pmacErrCode );
    if (match > 0)
    {
        asynPrint( pasynUser, ASYN_TRACE_ERROR, "ctWriteRead [%s]: PMAC responded with errocode: ERR%03d\n",
                    pPvt->portName, pmacErrCode );
        status = asynError;
    }   
    
    return status;
}




/* Calculate the array of compensation points on the source motors travel range based on length of table and range of motion */
void ct_sourcePoints( compensationtable_t *pPvt, asynUser *pasynUser )
{
    int addr, i;
    unsigned int index;
    double stepsize;
    double* table;
    const char *functionName = "ct_sourcePoints";
    pasynManager->getAddr(pasynUser, &addr);
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s addr=%d\n", functionName, pPvt->portName, addr);
    addrToIndex( pPvt, pasynUser, &index );

    table = pPvt->source_egu_table[index];
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: table=%p\n", functionName, table);
    stepsize = pPvt->source_range[index] / pPvt->entries[index];
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: stepsize=%f\n", functionName, stepsize);
    *table = 0.0;
    for (i=1; i<pPvt->entries[index]; i++) 
        *(table+i) = *(table+i-1) + stepsize;
    
    ct_defineCompCmd( pPvt, pasynUser, addr);
}

void ct_defineCompCmd( compensationtable_t *pPvt, asynUser *pasynUser, unsigned int table )
{
    unsigned int index = table -1;
    const char *functionName = "ct_defineCompCmd";
    const char *defcompDesired = "#%d DEF COMP %d,#%dD,#%d,%d";
    const char *defcompActual =  "#%d DEF COMP %d,#%d,#%d,%d";
    const char *cmdTemplate;
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s index=%d\n", functionName, pPvt->portName, index);
    
    if (pPvt->desired[index] == 0) cmdTemplate = defcompActual;
    else cmdTemplate = defcompDesired;
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s table=%d template: \'%s\'\n", 
               functionName, pPvt->portName, table, cmdTemplate);
    
    /*  Create the string with the define commnand of format: 
        {#motor} DEF COMP {entries},{#source[D]},{#target},{count_length} */
    sprintf( pPvt->define_comp_cmd[index], cmdTemplate,
            table, pPvt->entries[index], pPvt->source[index], 
            pPvt->target[index], pPvt->count_length[index] );
    
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: new define compensation command: \'%s\'\n", 
               functionName, pPvt->define_comp_cmd[index]);
}

void ct_allocTables( compensationtable_t *pPvt, asynUser *pasynUser, size_t nElements )
{
    int addr;
    unsigned int index;
    pasynManager->getAddr(pasynUser, &addr);
    addrToIndex( pPvt, pasynUser, &index );
    
    if (pPvt->source_egu_table[index] == NULL) 
        pPvt->source_egu_table[index] = (epicsFloat64*)calloc( nElements, sizeof(epicsFloat64) );
    if (pPvt->count_table[index] == NULL) 
        pPvt->count_table[index] = (epicsInt32*)calloc( nElements, sizeof(epicsInt32) );
    if (pPvt->egu_table[index] == NULL) 
        pPvt->egu_table[index] = (epicsFloat64*)calloc( nElements, sizeof(epicsFloat64) );
}

void addrToIndex( compensationtable_t *pPvt, asynUser *pasynUser, unsigned int *index )
{
    int tmp;
    pasynManager->getAddr(pasynUser, &tmp);
    
    if (tmp < 1 || tmp > pPvt->nTables )
    {
        asynPrint( pasynUser, ASYN_TRACE_ERROR, 
                   "addrToIndex: table address (motor) %d is not in valid range [1..%d]\n", 
                   tmp, pPvt->nTables );
        *index = 0;
    } else
    {
        *index = (unsigned int)tmp - 1;
    }
}

asynStatus ct_generateDefCmd( compensationtable_t *pPvt, asynUser *pasynUser )
{
    asynStatus status = asynSuccess;
    int i, c = 0;
    int entry, addr;
    size_t bufsize = pPvt->nTables * TABLE_NBYTES;
    const char* functionName = "ct_generateDefCmd";

    pasynManager->getAddr(pasynUser, &addr);
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s addr=%d\n", functionName, pPvt->portName, addr);

    for (i = 0; i < pPvt->nTables; i++)
    {
        ct_defineCompCmd( pPvt, pasynUser, i+1);
        if (pPvt->enable[i] != 1) continue;
        if (c > bufsize - 40 || status == asynError) { status = asynError; break; }
        c += snprintf( (pPvt->tableDefCmd + c), bufsize - c, "%s\n", pPvt->define_comp_cmd[i] );
        
        for (entry = 0; entry < pPvt->entries[i]; entry++)
        {
            if (c > bufsize - 10) { status = asynError; break; }
            c += snprintf( (pPvt->tableDefCmd + c), bufsize - c, "%d ", pPvt->count_table[i][entry] );
        }
        c += snprintf( (pPvt->tableDefCmd + c), bufsize - c, "\n" );
    }
    if (status != asynSuccess) 
        asynPrint( pasynUser, ASYN_TRACE_ERROR, 
                   "%s: ERROR: buffer full! c=%d bufsize=%d tablei=%d\n",
                   c, bufsize, i);
    else asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: define tables:\n%s\n", functionName, pPvt->tableDefCmd );
    return status;
}

asynStatus ct_writeTables( compensationtable_t *pPvt, asynUser *pasynUser )
{
    asynStatus status = asynSuccess;
    char * cmd = pPvt->pmacSendBuf;
    char * response = pPvt->pmacRecvBuf;
    int addr;
	int i;
    const char* functionName = "ct_writeTables";
   
    pasynManager->getAddr(pasynUser, &addr);
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s addr=%d\n", functionName, pPvt->portName, addr);

    /* First disable the compensation buffers */
    snprintf( cmd, PMAC_COMM_BYTES, "I51=0" );
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;
    
    /* Delete the existing buffers in order to re-write the new ones */
    status = ct_deleteBuffers( pPvt, pasynUser );
    if (status != asynSuccess) return status;
    
    /* Write the new comp tables for all axes */
    cmd = pPvt->tableDefCmd;
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;
    
	/* redefine lookahead buffers */
    for (i = 8; i >= 1; i--) snprintf( cmd+(i-1), PMAC_COMM_BYTES, "&%d DEFINE LOOKAHEAD 50,10\r", i );
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );

    /* Re-enable the tables in the PMAC if the user wants it */
    snprintf( cmd, PMAC_COMM_BYTES, "I51=%d", pPvt->i51 );
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;

    return status;
}

asynStatus ct_readSettings( compensationtable_t *pPvt, asynUser *pasynUser )
{
    asynStatus status = asynSuccess;
    int nitems, addr;
    char * cmd = pPvt->pmacSendBuf;
    char * response = pPvt->pmacRecvBuf;
    const char* functionName = "ct_readSettings";

    pasynManager->getAddr(pasynUser, &addr);
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s addr=%d\n", functionName, pPvt->portName, addr);
    
    snprintf( cmd, PMAC_COMM_BYTES , "I51 I30");
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;
    
    nitems = sscanf( response, "%d\r%d", &pPvt->i51, &pPvt->i30 );
    if (nitems != 2) return asynError;
    
    return status;
}

asynStatus ct_deleteBuffers( compensationtable_t *pPvt, asynUser *pasynUser )
{
    asynStatus status = asynSuccess;
    int addr, i;
    char * cmd = pPvt->pmacSendBuf;
    char * response = pPvt->pmacRecvBuf;
    const char* functionName = "ct_deleteBuffers";
    
    pasynManager->getAddr(pasynUser, &addr);
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "%s: port=%s addr=%d\n", functionName, pPvt->portName, addr);

    /* To begin with I don't want to delete all the buffers that are not supported by this module */
/*
    snprintf( cmd, PMAC_COMM_BYTES , "DEL GAT\rDEL TBUF");
    
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;
    
    for (i = 1; i <= MAX_NTABLES; i++) snprintf( cmd+(i-1), PMAC_COMM_BYTES, "#%d DEL BLCOMP\r", i );
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;
    
    for (i = 1; i <= MAX_NTABLES; i++) snprintf( cmd+(i-1), PMAC_COMM_BYTES, "#%d DEL TCOMP\r", i );
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;

    for (i = 1; i <= MAX_NTABLES; i++) snprintf( cmd+(i-1), PMAC_COMM_BYTES, "#%d DEL COMP\r", i );
    status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    if (status != asynSuccess) return status;
*/    
	snprintf( cmd, PMAC_COMM_BYTES , "DEL ALL");
	status = ctWriteRead( pPvt, pasynUser, cmd, PMAC_COMM_BYTES, response );
    
    return status;
}

/**************************************************************************************
 * 
 * Register the config function with the EPICS IOC shell
 * 
 **************************************************************************************/

 
static const iocshArg ctConfigArg0 = {"Port name", iocshArgString};
static const iocshArg ctConfigArg1 = {"Asyn octet port name", iocshArgString};
static const iocshArg ctConfigArg2 = {"Number of tables", iocshArgInt};
static const iocshArg * const ctConfigArgs[] =  {&ctConfigArg0,
                                                 &ctConfigArg1,
                                                 &ctConfigArg2};
static const iocshFuncDef config_ct = {"compTabConfig", 3, ctConfigArgs};
static void config_ctCallFunc(const iocshArgBuf *args)
{
    ctConfig(args[0].sval, args[1].sval, args[2].ival);
}

static void ctRegister(void)
{

    iocshRegister(&config_ct, config_ctCallFunc);
}

epicsExportRegistrar(ctRegister);

