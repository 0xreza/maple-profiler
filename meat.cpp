/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdio.h>
#include "pin.H"

//My Additions
// #include <vector>
// #include <tuple>
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

#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */

//My Additions
// typedef tuple<unsigned int, unsigned int, int, int> page_tuple; //page_number, threadid, rcount, wcount
// vector <page_tuple> page_v; // tuple of vector
PIN_LOCK lock;


FILE * trace;

int rcount = 0;
int wcount = 0;
int start_logging = 0;
stringstream buffer;

long int *sz;

int output_file;
char buf[50];
void log(char op,unsigned long page){
    sprintf(buf,"%c,%lu,%lu\n", op, page, (unsigned long)time(NULL)); 
    write(output_file, buf, strlen(buf));
}

// This routine is executed every time a thread is created.
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&lock, threadid+1);
    cout << "thread begin: " << threadid << endl;
    //pid_t pid = getpid();
    //pid_t tid = syscall(SYS_gettid);
    cout << "Thread " << threadid << syscall(SYS_gettid) << endl;

    //fprintf(out, "thread begin %d\n",threadid);
    //fflush(out);
    PIN_ReleaseLock(&lock);
}

// This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    PIN_GetLock(&lock, threadid+1);
    cout << "thread end: " << threadid << endl;
    //fprintf(out, "thread end %d code %d\n",threadid, code);
    //fflush(out);
    PIN_ReleaseLock(&lock);
}

string GetBinaryStringFromHexString (string sHex)
{
    string sReturn = "";
    for (unsigned int i = 0; i < sHex.length (); ++i)
    {
        switch (sHex [i])
        {
            case '0': sReturn.append ("0000"); break;
            case '1': sReturn.append ("0001"); break;
            case '2': sReturn.append ("0010"); break;
            case '3': sReturn.append ("0011"); break;
            case '4': sReturn.append ("0100"); break;
            case '5': sReturn.append ("0101"); break;
            case '6': sReturn.append ("0110"); break;
            case '7': sReturn.append ("0111"); break;
            case '8': sReturn.append ("1000"); break;
            case '9': sReturn.append ("1001"); break;
            case 'a': sReturn.append ("1010"); break;
            case 'b': sReturn.append ("1011"); break;
            case 'c': sReturn.append ("1100"); break;
            case 'd': sReturn.append ("1101"); break;
            case 'e': sReturn.append ("1110"); break;
            case 'f': sReturn.append ("1111"); break;
            }
    }
    return sReturn;
}

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr, THREADID threadid)
{
    //buffer.clear();//clear any bits set
   // stringstream buffer;

    // start logging the accesses when thread 1 begins
    if (threadid == 1) {
        start_logging = 1;
    }

    if (start_logging == 1) {

        PIN_GetLock(&lock, threadid+1);
        buffer << addr;

        string s = buffer.str();
        s.insert(2, "0000");
        s = s.substr(2, (s.length()-1));
        //cout << "s: " << s << endl;

        string addr_binary = GetBinaryStringFromHexString(s);

        //tracing at the page level, page = 4096 bytes
        string page_binary = bitset<64>(4096).to_string<char,std::string::traits_type,std::string::allocator_type>();

        unsigned long page_decimal = bitset<64>(page_binary).to_ulong();
        unsigned long addr_decimal = bitset<64>(addr_binary).to_ulong();
        //buffer << setw(16) << setfill('0') << addr;

       // cout << "page_binary: " << page_binary << endl;
       // cout << "addr_binary: " << addr_binary << endl;
       // cout << "page_decimal: " << page_decimal << " addr_decimal: " << addr_decimal << endl;

        //cout << "~page_binary: " << ~(page_decimal-1) << endl;
        unsigned int virtual_page = (addr_decimal & ~(page_decimal-1));
        //cout << "Shifting Right: " << (addr_decimal >> 12) << endl;
        //cout << "virtual_page: " << virtual_page << endl;
        //int page_number_divided = address / sz ;
       // cout << "page_number_divided: " << page_number_divided << endl;
       // cout << "buffer before: " << buffer.str() << " addr before: " << addr << endl;

        //buffer << hex << setw(16) << setfill('0') << addr;
        //buffer << buffer.str();
       // string firstEightBits = buffer.str().substr(0,3);
      //  unsigned int page_number = stoi(firstEightBits, 0, 16);

       // cout << "buffer: " << buffer.str() << " firstEightBits: " << hex << firstEightBits << " page_number: " << page_number << " addr: " << addr << endl;
       // cout << "size: " << page_v.size() << endl;
       // if (page_v.size() <= 0){
        //    rcount = 1;
        //    page_v.push_back(make_tuple(virtual_page, PIN_ThreadId(), rcount, wcount));
            //page_v.push_back(make_tuple(page_number, 1, count));
        //}

       // int page_exist = 0;

        // check if the page exist and just increment the counter
        // for (unsigned int i = 0; i < page_v.size(); i++)
        // {
        //     //cout << "size: " << page_v.size() << endl;
        //     if (get<0>(page_v.at(i)) == virtual_page && get<1>(page_v.at(i)) == threadid)
        //     {
        //         get<2> (page_v.at(i)) = get<2> (page_v.at(i)) + 1;
        //         page_exist = 1;
        //         break;
        //     } 
        // }
        log('r',virtual_page);

        // page doesn't exist, add
        // if (page_exist == 0){
        //     rcount = 1;
        //     page_v.push_back(make_tuple(virtual_page, threadid, rcount, wcount));
        // }

        // page_exist = 0;
        buffer.str("");
        PIN_ReleaseLock(&lock);
    }
    //fprintf(trace,"%p: R %p\n", ip, addr);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, THREADID threadid)
{
   // buffer.clear();//clear any bits set
   // buffer.str(std::string());
    //stringstream buffer;
    // start logging accesses when thread 1 begins
    if (threadid == 1) {
        start_logging = 1;
    }

    if (start_logging == 1) {

        PIN_GetLock(&lock, threadid+1);
        buffer << addr;

        string s = buffer.str();
        s.insert(2, "0000");
        s = s.substr(2, (s.length()-1));
       // cout << "s: " << s << endl;

        string addr_binary = GetBinaryStringFromHexString(s);


        string page_binary = bitset<64>(4096).to_string<char,std::string::traits_type,std::string::allocator_type>();

        unsigned long page_decimal = bitset<64>(page_binary).to_ulong();
        unsigned long addr_decimal = bitset<64>(addr_binary).to_ulong();
        //bitset<64> b(n);
       //string addr_binary = bitset<64>(n).to_string();

       // cout << "page_binary: " << page_binary << endl;
        //cout << "addr_binary: " << addr_binary << endl;
       // cout << "page_decimal: " << page_decimal << " addr_decimal: " << addr_decimal << endl;

        //cout << "~page_binary: " << ~(page_decimal-1) << endl;
        unsigned int virtual_page = (addr_decimal & ~(page_decimal-1));
        //cout << "Shifting Right: " << (addr_decimal >> 12) << endl;
        //cout << "virtual_page: " << virtual_page << endl;
       // cout << "addr_binary: " << b.to_string() << endl;
       // buffer << setw(16) << setfill('0') << addr;


        //int page_number_divided = address / sz ;
        //cout << "page_number_divided: " << page_number_divided << endl;

       // buffer << buffer.str();
       // cout << "buffer before: " << buffer.str() << " addr before: " << addr << endl;
       // buffer << hex << setw(16) << setfill('0') << addr;
        //string firstEightBits = buffer.str().substr(0,3);
        //unsigned int page_number = stoi(firstEightBits, 0, 16);

       // cout << "buffer: " << buffer.str() << " firstEightBits: " << hex << firstEightBits << " page_number: " << page_number << " addr: " << addr << endl;
        //  cout << "size: " << page_v.size() << endl;
       // if (page_v.size() <= 0){
        //    wcount = 1;
        //    page_v.push_back(make_tuple(virtual_page, PIN_ThreadId(), rcount, wcount));
            //page_v.push_back(make_tuple(page_number, 1, count));
        //}

        // int page_exist = 0;

        // // check if the page exist and just increment the counter
        // for (unsigned int i = 0; i < page_v.size(); i++)
        // {
        //     //cout << "size: " << page_v.size() << endl;
        //     if (get<0>(page_v.at(i)) == virtual_page && get<1>(page_v.at(i)) == PIN_ThreadId())
        //     {
        //         get<3> (page_v.at(i)) = get<3> (page_v.at(i)) + 1;
        //         page_exist = 1;
        //         break;
        //     } 
        // }

        // // page doesn't exist, add
        // if (page_exist == 0){
        //     wcount = 1;
        //     page_v.push_back(make_tuple(virtual_page, PIN_ThreadId(), rcount, wcount));
        // }

        // page_exist = 0;

        log('w',virtual_page);

        buffer.str("");
        PIN_ReleaseLock(&lock);
    }
    //fprintf(trace,"%p: W %p\n", ip, addr);
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
    // ofstream myfile("memtracker.log");
    // if (myfile.is_open()){
    //     for (unsigned int i = 0; i < page_v.size(); i++){
    //         // print only accesses whose read || write threshold exceeds a number i.e. 100
    //         if (get<2>(page_v[i]) >= 100 || get<3>(page_v[i]) >= 100)
    //         {
    //             myfile << get<0>(page_v[i]) << ",";
    //             myfile << get<1>(page_v[i]) << ",";
    //             myfile << get<2>(page_v[i]) << ",";
    //             myfile << get<3>(page_v[i]) << "\n";
    //         }
    //     }
    // }
    // else cout << "Unable to open file";
    //fprintf(myfile, "#eof\n");
    //fclose(myfile);

    close(output_file);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    char file_name[50];
    sprintf(file_name, "trace_%lu.csv", (unsigned long)time(NULL));
    output_file = open(file_name, O_CREAT | O_RDWR);
    // Get the page_size()
    //sz = sysconf(_SC_PAGESIZE);

    //cout << "My Page Size is: " << sz << endl;

    // Initialize the pin lock
    PIN_InitLock(&lock);

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    //trace = fopen("pinatrace.out", "w");

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