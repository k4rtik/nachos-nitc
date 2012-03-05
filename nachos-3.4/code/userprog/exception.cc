// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "exception.h"

char buf[BUF_SIZE];

void updatePC()
{
	// Note that we have to maintain three PC registers, 
	// namely : PCReg, NextPCReg, PrevPCReg. 
	// (See machine/machine.cc, machine/machine.h) for more details.
	int pc, nextpc, prevpc;

	// Read PCs
	prevpc = machine->ReadRegister(PrevPCReg);
	pc = machine->ReadRegister(PCReg);
	nextpc = machine->ReadRegister(NextPCReg);

	// Update PCs
	prevpc = pc;
	pc = nextpc;
	nextpc = nextpc + 4;	// PC incremented by 4 in MIPS

	// Write back PCs
	machine->WriteRegister(PrevPCReg, prevpc);
	machine->WriteRegister(PCReg, pc);
	machine->WriteRegister(NextPCReg, nextpc);
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch(which)
    {
        case SyscallException:
            switch(type)
            {
                case SC_Halt:
                         DEBUG('a', "DEBUG: Shutdown, initiated by user program.\n");
                         interrupt->Halt();
                         break;

                case SC_Print:
                {
                        DEBUG('a', "DEBUG: Print syscall invoked.\n");
			int vaddr = machine->ReadRegister(4);
			// This address (pointer to the string to be printed) is 
			// the address that points to the user address space.
			// Simply trying printf("%s", (char*)addr) will not work
			// as we are now in kernel space.
                
			// Get the string from user space.
                
			int size = 0;
                
			buf[BUF_SIZE - 1] = '\0';               // For safety.
                
			do {
				// Invoke ReadMem to read the contents from user space
                
				machine->ReadMem(vaddr,    // Location to be read
					sizeof(char),      // Size of data to be read
					(int*)(buf+size)   // where the read contents 
					);                 // are stored
                
				// Compute next address
				vaddr+=sizeof(char);    size++;
                
			} while( size < (BUF_SIZE - 1) && buf[size-1] != '\0');
                
			size--;
			DEBUG('a', "DEBUG: Size of string = %d\n", size);
                
			printf("%s", buf);
			bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
			updatePC();
			break; // SC_Print
		}

                case SC_Read:
                {
                        DEBUG('a', "DEBUG: Read syscall invoked.\n");
                        int baddr = machine->ReadRegister(4); //address of buffer
                        int bsize =  machine->ReadRegister(5);
                        int fd =   machine->ReadRegister(6);
                        
			int size, i = 0;
                
			buf[BUF_SIZE - 1] = '\0';               // For safety.

                        size = read(fd, buf, bsize);
			DEBUG('a', "DEBUG: Read from file = %s\n", buf);
			bsize = size;
                        while ( i < (BUF_SIZE-1) && bsize--) {
				// Invoke WriteMem to write the contents to user space
                
				machine->WriteMem(baddr,   // Location to write
					sizeof(char),      // Size of data to be written
					(int) *(buf+i)       // where the write contents 
					);                 // are stored
                
				// Compute next address
				baddr+=sizeof(char);    i++;
			}
                        
			machine->WriteRegister(2, size);
			bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
                        updatePC();
                        break; // SC_Read
                }

                case SC_Write:
                {
                        DEBUG('a', "DEBUG: Write syscall invoked.\n");
                        int baddr = machine->ReadRegister(4); //address of buffer
                        int bsize =  machine->ReadRegister(5);
                        int fd =   machine->ReadRegister(6);
                        
			int size = 0;
                
			buf[BUF_SIZE - 1] = '\0';               // For safety.
                
			do {
				// Invoke ReadMem to read the contents from user space
                
				machine->ReadMem(baddr,    // Location to be read
					sizeof(char),      // Size of data to be read
					(int*)(buf+size)   // where the read contents 
					);                 // are stored
                
				// Compute next address
				baddr+=sizeof(char);    size++;
                
			} while( size < (BUF_SIZE - 1) && buf[size-1] != '\0');
                
			size--;
			DEBUG('a', "DEBUG: Size of write buffer = %d\n", size);

                        write(fd, buf, bsize);
                        
			bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
                        updatePC();
                        break; // SC_Write
                }

                case SC_Open:
                {
                        DEBUG('a', "DEBUG: Open syscall invoked.\n");
			int faddr = machine->ReadRegister(4); //address of filename string
                
			int size = 0;
                
			buf[BUF_SIZE - 1] = '\0';               // For safety.
                
			do {
				// Invoke ReadMem to read the contents from user space
                
				machine->ReadMem(faddr,    // Location to be read
					sizeof(char),      // Size of data to be read
					(int*)(buf+size)   // where the read contents 
					);                 // are stored
                
				// Compute next address
				faddr+=sizeof(char);    size++;
                
			} while( size < (BUF_SIZE - 1) && buf[size-1] != '\0');
                
			size--;
			DEBUG('a', "DEBUG: Size of filename string = %d\n", size);
                
			int fd = open(buf, O_RDWR);
			
			DEBUG('a', "DEBUG: fd = %d.\n", fd);
			machine->WriteRegister(2, fd);
			bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.

			updatePC();
                        break; // SC_Open
                }

                case SC_Close:
                {
                        DEBUG('a', "DEBUG: Close syscall invoked.\n");
			int fd = machine->ReadRegister(4);
                
                        close(fd);

			updatePC();
                        break; // SC_Open
                }

                default:
                        printf("Unknown/Unimplemented system call %d!\n", type);
                        ASSERT(FALSE); // Should never happen
                        break;
            } // End switch(type)
	break; // End case SyscallException

        default:
                printf("Unexpected user mode exception %d %d\n", which, type);
                ASSERT(FALSE);
		break;
    }
}
