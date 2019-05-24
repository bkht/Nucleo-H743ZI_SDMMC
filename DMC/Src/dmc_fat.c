#include "dmc_fat.h"

FRESULT scan_files(char* path, uint8_t printLegend)
{
	FRESULT res;
	DIR dir;
//    UINT i;
	FILINFO fno;
//    char path2[128];

	if (printLegend)
	{
		printf("Directory: %s\n", path);
		//      -a---   00-00-1980 00:00           15   hello.txt
		printf("Attr.   Last modified      Size         Name\n");
		printf("-----   ----------------   ----------   ----------------\n");
	}
	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR)
			{ /* It is a directory */
//                i = strlen(path);
//                sprintf(&path[i], "/%s", fno.fname);
//                res = scan_files(path);                    /* Enter the directory */
//                if (res != FR_OK) break;
				printf("%c%c%c%c%c   ", (fno.fattrib & AM_DIR) ? 'd' : '-',
						(fno.fattrib & AM_ARC) ? 'a' : '-',
						(fno.fattrib & AM_RDO) ? 'r' : '-',
						(fno.fattrib & AM_HID) ? 'h' : '-',
						(fno.fattrib & AM_SYS) ? 's' : '-');
				printf("%02u-%02u-%u %02u:%02u   ", fno.fdate & 31,
						(fno.fdate >> 5) & 15, (fno.fdate >> 9) + 1980,
						fno.ftime >> 11, (fno.ftime >> 5) & 63);
//                if (strlen(path > 2))
//                {
//                	printf("%10lu   %s/%s\n", fno.fsize, path, fno.fname);
//                }
//                else
//                {
				printf("             %s\n", fno.fname);
//                }
//        		printf("%s\n", path);
//                path[i] = 0;
				scan_files(fno.fname, 0);
			}
			else
			{
				/* It is a file. */
				printf("%c%c%c%c%c   ", (fno.fattrib & AM_DIR) ? 'd' : '-',
						(fno.fattrib & AM_ARC) ? 'a' : '-',
						(fno.fattrib & AM_RDO) ? 'r' : '-',
						(fno.fattrib & AM_HID) ? 'h' : '-',
						(fno.fattrib & AM_SYS) ? 's' : '-');
				printf("%02u-%02u-%u %02u:%02u   ", fno.fdate & 31,
						(fno.fdate >> 5) & 15, (fno.fdate >> 9) + 1980,
						fno.ftime >> 11, (fno.ftime >> 5) & 63);
				if (strlen(path) <= 1)
				{
					printf("%10llu   %s\n", fno.fsize, fno.fname);
				}
				else
				{
					printf("%10llu   %s/%s\n", fno.fsize, path, fno.fname);
				}
			}
		}
		f_closedir(&dir);
	}

	return res;
}

void ShowFatFsError(FRESULT res)
{
	printf("FAILED: ");
	if (res == FR_DISK_ERR)
		printf("1: A hard error occurred in the low level disk I/O layer\n");
	else if (res == FR_INT_ERR)
		printf("2: Assertion failed\n");
	else if (res == FR_NOT_READY)
		printf("3: The physical drive cannot work\n");
	else if (res == FR_NO_FILE)
		printf("4: Could not find the file\n");
	else if (res == FR_NO_PATH)
		printf("5: Could not find the path\n");
	else if (res == FR_INVALID_NAME)
		printf("6: The path name format is invalid\n");
	else if (res == FR_DENIED)
		printf("7: Access denied due to prohibited access or directory full\n");
	else if (res == FR_EXIST)
		printf("8: Access denied due to prohibited access\n");
	else if (res == FR_INVALID_OBJECT)
		printf("9: The file/directory object is invalid\n");
	else if (res == FR_WRITE_PROTECTED)
		printf("10: The physical drive is write protected\n");
	else if (res == FR_INVALID_DRIVE)
		printf("11: The logical drive number is invalid\n");
	else if (res == FR_NOT_ENABLED)
		printf("12: The volume has no work area\n");
	else if (res == FR_NO_FILESYSTEM)
		printf("13: There is no valid FAT volume\n");
	else if (res == FR_MKFS_ABORTED)
		printf("14: The f_mkfs() aborted due to any problem\n");
	else if (res == FR_TIMEOUT)
		printf("15: Could not get a grant to access the volume within defined period\n");
	else if (res == FR_LOCKED)
		printf("16: The operation is rejected according to the file sharing policy\n");
	else if (res == FR_NOT_ENOUGH_CORE)
		printf("17: LFN working buffer could not be allocated\n");
	else if (res == FR_TOO_MANY_OPEN_FILES)
		printf("18: Number of open files > _FS_LOCK\n");
	else if (res == FR_INVALID_PARAMETER)
		printf("19: FR_INVALID_PARAMETER\n");
	else if (res == FR_DISK_ERR)
		printf("1: FR_DISK_ERR\n");
	else
		printf("%d (unknown error)", res);
}

void ShowDiskStatus(DSTATUS stat)
{
	printf("FAILED: ");
	if (stat == STA_NOINIT)
		printf("0x01: Drive not initialized\n");
	else if (stat == STA_NODISK)
		printf("0x02: No medium in the drive\n");
	else if (stat == STA_PROTECT)
		printf("0x04: Write protected\n");
	else
		printf("%d (Unknown error)\n", stat);

}

FRESULT set_timestamp(char *obj, /* Pointer to the file name */
int year, int month, int mday, int hour, int min, int sec)
{
	FILINFO fno;

	fno.fdate = (WORD) (((year - 1980) * 512U) | month * 32U | mday);
	fno.ftime = (WORD) (hour * 2048U | min * 32U | sec / 2U);

	return f_utime(obj, &fno);
}
