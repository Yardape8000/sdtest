#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string.h>
//#include <time.h>
//#include <sys/time.h>
#include <malloc.h>
//#include <inttypes.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"

#define TEST_FILE_PATH	"sd:/wiiu/apps/sdtest/data/" 
#define PRINT_TEXT2(x, y, ...) { snprintf(msg, 80, __VA_ARGS__); OSScreenPutFontEx(0, x, y, msg); OSScreenPutFontEx(1, x, y, msg); }

u64 fix_OSTicks(u64 ticks)
{
	ticks = ((ticks & 0x00000000FFFFFFFF) << 32) | ((ticks & 0xFFFFFFFF00000000) >> 32);
	return ticks;
}

u32 pressed_buttons(void)
{
    int vpadError = -1;
    VPADData vpad;
	VPADRead(0, &vpad, 1, &vpadError);
	return (vpadError == 0 ) ? (vpad.btns_d | vpad.btns_h) : 0;
}

double calc_read_time(u64 end_time, u64 start_time, int rollover)
{
	start_time = (start_time & 0xFFFFFFFF00000000) >> 32;
	end_time = (end_time & 0xFFFFFFFF00000000) >> 32 | ((u64)rollover << 32);
	return (double)((end_time - start_time) * 100 / SECS_TO_TICKS(1)) / 100;
}

double calc_read_rate(double read_time, int factor)
{
	double read_rate = 0;
	if (read_time > 0)
		read_rate = factor / read_time;
	return read_rate;
}

void update_screen(double data[], int handle_buffer)
{
    char msg[80];
	if (handle_buffer)
	{
		OSScreenClearBufferEx(0, 0);
		OSScreenClearBufferEx(1, 0);
	}
	PRINT_TEXT2(0, 4, "1k byte = %.2f s, %.2f MBps", data[0], calc_read_rate(data[0], 1));
	PRINT_TEXT2(0, 5, "10k byte = %.2f s, %.2f MBps", data[1], calc_read_rate(data[1], 10));
	PRINT_TEXT2(0, 6, "100k byte = %.2f s, %.2f MBps", data[2], calc_read_rate(data[2], 100));
	PRINT_TEXT2(0, 7, "1M byte = %.2f s, %.2f MBps", data[3], calc_read_rate(data[3], 1000));
	PRINT_TEXT2(0, 8, "1k byte in 1M = %.2f s, %.2f MBps", data[4], calc_read_rate(data[4], 1));
	PRINT_TEXT2(0, 9, "10k byte in 10M = %.2f s, %.2f MBps", data[5], calc_read_rate(data[5], 10));
	PRINT_TEXT2(0, 10, "100k byte in 100M = %.2f s, %.2f MBps", data[6], calc_read_rate(data[6], 100));
	PRINT_TEXT2(0, 11, "1M byte in 1G = %.2f s, %.2f MBps", data[7], calc_read_rate(data[7], 1000));
	PRINT_TEXT2(0, 12, "1k byte in 10M = %.2f s, %.2f MBps", data[8], calc_read_rate(data[8], 10));
	if (handle_buffer)
	{
		PRINT_TEXT2(0, 14, "Hold X to Exit");
		OSScreenFlipBuffersEx(0);
		OSScreenFlipBuffersEx(1);
	}
}

int test1(unsigned char* fileBuffer, double read_time[], int addr, int file_size, int samples, int loops, char *file_name)
{
	int iFd;
	int failed = 0;
	u64 start_time, end_time, last_time;
	char test_file[255];
	int test_num = 0;
	read_time[addr] = 0;
	int rollover = 0;
	
	start_time = OSGetTick();
	end_time = start_time;
	last_time = start_time;
	for (int i = 0; i < loops; i++)
	{
		test_num = rand() % samples + 1;
		snprintf(test_file, 255, "sd:/wiiu/apps/sdtest/data/%s%d.x", file_name, test_num);
		iFd = open(test_file, O_RDONLY);  
		if (iFd >= 0)  
		{  
			read(iFd, fileBuffer, file_size);  
			close(iFd);  
		}
		else
		{
			failed = 1;
			break;
		}
		end_time = OSGetTick();
		if (end_time < last_time)
			rollover ++;
		last_time = end_time;
	}
	if (!failed)
	{
		//end_time = OSGetTick();
		read_time[addr] = calc_read_time(end_time, start_time, rollover);
		update_screen(read_time, 1);
	}
	return failed;
}

int test2(unsigned char* fileBuffer, double read_time[], int addr, int file_size, int samples, int loops, char *file_name)
{
	int iFd;
	int failed = 0;
	u64 start_time, end_time, last_time;
	char test_file[255];
	int test_num = 0;
	int rollover = 0;
	
	snprintf(test_file, 255, "sd:/wiiu/apps/sdtest/data/%s.x", file_name);
	start_time = OSGetTick();
	end_time = start_time;
	last_time = start_time;
	iFd = open(test_file, O_RDONLY);  
	if (iFd >= 0)
	{
		for (int i = 0; i < loops; i++)
		{
			test_num = rand() % samples;
			lseek(iFd, test_num * file_size, SEEK_SET);
			read(iFd, fileBuffer, file_size);  

			end_time = OSGetTick();
			if (end_time < last_time)
				rollover ++;
			last_time = end_time;
		}
	}
	else
	{
		failed = 1;
	}
	close(iFd);  
	if (!failed)
	{
		end_time = OSGetTick();
		if (end_time < last_time)
			rollover ++;
		read_time[addr] = calc_read_time(end_time, start_time, rollover);
		update_screen(read_time, 1);
	}
	return failed;
}


int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
    InitOSFunctionPointers();
    InitSocketFunctionPointers();

    log_init("192.168.178.3");
    log_print("Starting launcher\n");

    InitFSFunctionPointers();
    InitVPadFunctionPointers();

    log_print("Function exports loaded\n");

    //!*******************************************************************
    //!                    Initialize heap memory                        *
    //!*******************************************************************
    log_print("Initialize memory management\n");
    //! We don't need bucket and MEM1 memory so no need to initialize
    memoryInitialize();
    char msg[80];

    //!*******************************************************************
    //!                        Initialize FS                             *
    //!*******************************************************************
    log_printf("Mount SD partition\n");
    mount_sd_fat("sd");

    VPADInit();

    // Prepare screen
    int screen_buf0_size = 0;
    int screen_buf1_size = 0;

    // Init screen and screen buffers
    OSScreenInit();
    screen_buf0_size = OSScreenGetBufferSizeEx(0);
    screen_buf1_size = OSScreenGetBufferSizeEx(1);

    unsigned char *screenBuffer = MEM1_alloc(screen_buf0_size + screen_buf1_size, 0x40);

    OSScreenSetBufferEx(0, screenBuffer);
    OSScreenSetBufferEx(1, (screenBuffer + screen_buf0_size));

    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);

    // Clear screens
    OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);

	// setup filebuffer
	int mem_ok = 0;
    unsigned char *fileBuffer = MEM1_alloc(10000020, 0x40);
	if (fileBuffer != NULL) mem_ok = 1;

	PRINT_TEXT2(0, 0, "SD Test");
	PRINT_TEXT2(0, 1, "Press HOME-Button to exit.");

    if (mem_ok)
	{
		PRINT_TEXT2(0, 2, "Press A to start.");
	}
	else
	{
		PRINT_TEXT2(0, 2, "Memory allocation failed");
	}
		

    // Flush the cache
    DCFlushRange(screenBuffer, screen_buf0_size);
    DCFlushRange((screenBuffer + screen_buf0_size), screen_buf1_size);

    // Flip buffers
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

	srand (time(NULL));

	int failed = 0;
	double read_time[10] = {0};
	u32 pressedBtns;
	
    while(mem_ok)
    {
        pressedBtns = pressed_buttons();
		if(pressedBtns & VPAD_BUTTON_HOME)
        {
			mem_ok = 0;
			break;
		}

        if(pressedBtns & VPAD_BUTTON_A)
        {
			if (test1(fileBuffer, read_time, 0, 1000, 100, 1000, "A")) break;		// 100 x 1k files, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			if (test1(fileBuffer, read_time, 1, 10000, 100, 1000, "B")) break;		// 100 x 10k files, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			if (test1(fileBuffer, read_time, 2, 100000, 100, 1000, "C")) break;		// 100 x 100k files, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			if (test1(fileBuffer, read_time, 3, 1000000, 100, 1000, "D")) break;	// 100 x 1M files, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;

			if (test2(fileBuffer, read_time, 4, 1000, 100, 1000, "D1")) break;		// 100 x 1k blocks in 1M file, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			if (test2(fileBuffer, read_time, 5, 10000, 100, 1000, "E")) break;		// 100 x 10k blocks in 10M file, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			if (test2(fileBuffer, read_time, 6, 100000, 100, 1000, "F")) break;		// 100 x 100k blocks in 100M file, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			if (test2(fileBuffer, read_time, 7, 1000000, 100, 1000, "G")) break;	// 100 x 1M blocks in 1G file, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;

			if (test2(fileBuffer, read_time, 8, 1000, 1000, 10000, "E")) break;		// 1000 x 1k blocks in 10M file, 1000 reads
			if(pressed_buttons() & VPAD_BUTTON_X) break;
			
			break;
		}
		usleep(20000);
    }
	
	if (mem_ok)
	{
		DCFlushRange(screenBuffer, screen_buf0_size);
		DCFlushRange((screenBuffer + screen_buf0_size), screen_buf1_size);
		OSScreenClearBufferEx(0, 0);
		OSScreenClearBufferEx(1, 0);
		OSScreenFlipBuffersEx(0);
		OSScreenFlipBuffersEx(1);
		OSScreenClearBufferEx(0, 0);
		OSScreenClearBufferEx(1, 0);
		if (failed)
		{
			PRINT_TEXT2(0, 8, "Memory read failed.");
		}
		else
		{
				update_screen(read_time, 0);
		}
		PRINT_TEXT2(0, 14, "Home or X to Exit");
		OSScreenFlipBuffersEx(0);
		OSScreenFlipBuffersEx(1);
	}

	FILE *dataFile = fopen("sd:/wiiu/apps/sdtest/testdata.txt", "w");
	if (!failed && dataFile != NULL)
	{
		fprintf(dataFile, "1k byte = %.2f s, %.2f MBps\r\n", read_time[0], calc_read_rate(read_time[0], 1));
		fprintf(dataFile, "10k byte = %.2f s, %.2f MBps\r\n", read_time[1], calc_read_rate(read_time[1], 10));
		fprintf(dataFile, "100k byte = %.2f s, %.2f MBps\r\n", read_time[2], calc_read_rate(read_time[2], 100));
		fprintf(dataFile, "1M byte = %.2f s, %.2f MBps\r\n", read_time[3], calc_read_rate(read_time[3], 1000));
		fprintf(dataFile, "1k byte in 1M = %.2f s, %.2f MBps\r\n", read_time[4], calc_read_rate(read_time[4], 1));
		fprintf(dataFile, "10k byte in 10M = %.2f s, %.2f MBps\r\n", read_time[5], calc_read_rate(read_time[5], 10));
		fprintf(dataFile, "100k byte in 100M = %.2f s, %.2f MBps\r\n", read_time[6], calc_read_rate(read_time[6], 100));
		fprintf(dataFile, "1M byte in 1G = %.2f s, %.2f MBps\r\n", read_time[7], calc_read_rate(read_time[7], 1000));
		fprintf(dataFile, "1k byte in 10M = %.2f s, %.2f MBps\r\n", read_time[8], calc_read_rate(read_time[8], 10));
	}
	fclose(dataFile);
	
    while(mem_ok)
    {

        if(pressed_buttons() & (VPAD_BUTTON_HOME | VPAD_BUTTON_X)) break;
		usleep(50000);
    }

	MEM1_free(fileBuffer);
	MEM1_free(screenBuffer);
	screenBuffer = NULL;

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************

    log_printf("Unmount SD\n");
    unmount_sd_fat("sd");
    log_printf("Release memory\n");
    memoryRelease();
    log_deinit();

    return EXIT_SUCCESS;
}

