
//
//  maple.cpp
//  MAPLE: Memory Access ProfiLEr
//
//  Created by Reza Karimi on 07/20/2018.
//  Copyright © 2018 Reza Karimi. All rights reserved.
//
#include <stdio.h>
#include "pin.H"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <bitset>
#include <fcntl.h>
#include <unistd.h>

#define PAGE_SIZE 4096
#define OUTOUT_HEADERS "page,time,op\n"
PIN_LOCK lock;

FILE *trace;

int rcount = 0;
int wcount = 0;
int start_logging = 0;
stringstream buffer;

long int *sz;

unsigned long previous_access;
unsigned long epoch;

int output_file;
char buf[50];
void log(char op, unsigned long page)
{
    sprintf(buf, "%lu,%lu,%c\n", page, (unsigned long)time(0)-epoch, op);
    write(output_file, buf, strlen(buf));
}

// This routine is executed every time a thread is created.
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&lock, threadid + 1);
    cout << "thread begin: " << threadid << endl;
    //pid_t pid = getpid();
    //pid_t tid = syscall(SYS_gettid);
    cout << "Thread " << threadid << syscall(SYS_gettid) << endl;
    PIN_ReleaseLock(&lock);
}

// This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    PIN_GetLock(&lock, threadid + 1);
    cout << "thread end: " << threadid << endl;
    PIN_ReleaseLock(&lock);
}

// Print a memory read record
VOID RecordMemRead(VOID *ip, VOID *addr, THREADID threadid)
{
    PIN_GetLock(&lock, threadid + 1);
    buffer << addr;
    string s = buffer.str();
    s = s.substr(2, (s.length() - 1));
    unsigned long virtual_page = strtol(s.c_str(), NULL, 16) / PAGE_SIZE;
    if (previous_access != virtual_page){
        log('r', virtual_page);
        previous_access = virtual_page;
    } 
    buffer.str("");
    PIN_ReleaseLock(&lock);
}

// Print a memory write record
VOID RecordMemWrite(VOID *ip, VOID *addr, THREADID threadid)
{

    PIN_GetLock(&lock, threadid + 1);
    buffer << addr;
    string s = buffer.str();
    s = s.substr(2, (s.length() - 1));
    unsigned long virtual_page = strtol(s.c_str(), NULL, 16) / PAGE_SIZE;
    if (previous_access != virtual_page){
        log('w', virtual_page);
        previous_access = virtual_page;
    } 
    buffer.str("");
    PIN_ReleaseLock(&lock);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{

    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, IARG_THREAD_ID,
                IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    close(output_file);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of memory addresses\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    char file_name[50];
    sprintf(file_name, "trace_%lu.csv", (unsigned long)time(0));
    output_file = open(file_name, O_CREAT | O_RDWR);
    previous_access = -1;
    epoch = (unsigned long)time(NULL);
    write(output_file, OUTOUT_HEADERS, strlen(OUTOUT_HEADERS));
    // Initialze the pin lock
    PIN_InitLock(&lock);

    // Initialize pin
    if (PIN_Init(argc, argv))
        return Usage();

    INS_AddInstrumentFunction(Instruction, 0);

    // Register Analysis routines to be called when a thread begins/ends
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}