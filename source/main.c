// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

// Main program entrypoint
int main(int argc, char* argv[])
{
	// This example uses a text console, as a simple way to output text to the screen.
	// If you want to write a software-rendered graphics application,
	//   take a look at the graphics/simplegfx example, which uses the libnx Framebuffer API instead.
	// If on the other hand you want to write an OpenGL based application,
	//   take a look at the graphics/opengl set of examples, which uses EGL instead.
	consoleInit(NULL);

	// Configure our supported input layout: a single player with standard controller styles
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);

	// Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
	PadState pad;
	padInitializeDefault(&pad);

	// Other initialization goes here. As a demonstration, we print hello world.
	printf("Searching for \"usb\" sysmodule...\n");
	consoleUpdate(NULL);

	u64 pid = 0;
	Result rc = pmdmntInitialize();
	if (R_SUCCEEDED(rc)) {
		rc = pmdmntGetProcessId(&pid, 0x0100000000000006);
		pmdmntExit();
	}

	if (R_FAILED(rc)) {
		printf(CONSOLE_RED "\"usb\" not found...\n");
		goto loop;
	}
	else printf("\"usb\" found, PID: %ld\n", pid);
	consoleUpdate(NULL);


	Handle debug;
	rc = svcDebugActiveProcess(&debug, pid);
	if (R_FAILED(rc)) {
		printf(CONSOLE_RED "Couldn't debug \"usb\" process, err: 0x%x\n", rc);
		goto loop;
	}
	MemoryInfo mem_info = {0};
	u32 pageinfo = 0;
	u64 addr = 0;
	rc = svcQueryDebugProcessMemory(&mem_info, &pageinfo, debug, addr);
	if (R_FAILED(rc)) {
		printf(CONSOLE_RED "Couldn't retrieve process memory info, err: 0x%x\n", rc);
		svcCloseHandle(debug);
		goto loop;
	}
	addr = mem_info.addr;
	memset(&mem_info, 0, sizeof(mem_info));

	while(true) {
		rc = svcQueryDebugProcessMemory(&mem_info, &pageinfo, debug, addr);
		if (mem_info.addr < addr) {
			printf(CONSOLE_RED "Reached end of mappings, no valid mapping was found.\n");
			svcCloseHandle(debug);
			break;
		}
		if (R_FAILED(rc)) {
			printf(CONSOLE_RED "Couldn't retrieve process memory info, err: 0x%x\n", rc);
			svcCloseHandle(debug);
			break;
		}
		if (mem_info.type == MemType_Io && mem_info.size == 0x1000) {
			char stack[0x10] = "";
			char compare[0x10] = "";
			rc = svcReadDebugProcessMemory((void*)&stack[0], debug, mem_info.addr, 0x10);
			if (R_FAILED(rc)) {
				printf(CONSOLE_RED "Couldn't retrieve IO data, err: 0x%x\n", rc);
				svcCloseHandle(debug);
				break;				
			}
			if (!memcmp(stack, compare, 0x10)) {
				printf("Found IO region...\n");
				char dump[0x400] = "";
				rc = svcReadDebugProcessMemory((void*)dump, debug, mem_info.addr + 0x800, 0x400);
				svcCloseHandle(debug);
				if (R_FAILED(rc)) {
					printf(CONSOLE_RED "Couldn't retrieve FUSE data, err: 0x%x\n", rc);
					break;			
				}
				FILE* file = fopen("sdmc:/fuse_dump.bin", "wb");
				if (!file) {
					printf(CONSOLE_RED "Couldn't open for writing \"fuse_dump.bin\"...\n");
					break;				
				}
				fwrite((void*)dump, 0x400, 1, file);
				fclose(file);
				printf(CONSOLE_GREEN "File written successfully to root of sdcard as \"fuse_dump.bin\".\n");
				break;
			}
		}
		addr = mem_info.addr + mem_info.size;
	}
	


loop:
	// Main loop
	printf(CONSOLE_RESET "Press + to exit.\n");
	consoleUpdate(NULL);
	while (appletMainLoop())
	{
		// Scan the gamepad. This should be done once for each frame
		padUpdate(&pad);

		// padGetButtonsDown returns the set of buttons that have been
		// newly pressed in this frame compared to the previous one
		u64 kDown = padGetButtonsDown(&pad);

		if (kDown & HidNpadButton_Plus)
			break; // break in order to return to hbmenu

		// Your code goes here

		// Update the console, sending a new frame to the display
		consoleUpdate(NULL);
	}

	// Deinitialize and clean up resources used by the console (important!)
	consoleExit(NULL);
	return 0;
}
