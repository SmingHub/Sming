#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <spiffs.h>
#include <sys/stat.h>

#define LOG_PAGE_SIZE       256
#define SPI_FLASH_SEC_SIZE 4096

#define ROM_ERASE 0xFF

#define DEFAULT_FOLDER   "files"
#define DEFAULT_ROM_NAME "spiff_rom.bin"
#define DEFAULT_ROM_SIZE 0x30000

static spiffs fs;
static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];

#define S_DBG(fmt, ...)
//#define S_DBG printf

static FILE *rom = 0;

void hexdump_mem(u8_t *b, u32_t len) {
	int i;
	for (i = 0; i < len; i++) {
		S_DBG("%02x", *b++);
		if ((i % 16) == 15) S_DBG("\n");
		else if ((i % 16) == 7) S_DBG(" ");
	}
	if ((i % 16) != 0) S_DBG("\n");
}

static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst) {

	int res;

	if (fseek(rom, addr, SEEK_SET)) {
		printf("Unable to seek to %d.\n", addr);
		return SPIFFS_ERR_END_OF_OBJECT;
	}

	res = fread(dst, 1, size, rom);
	if (res != size) {
		printf("Unable to read - tried to get %d bytes only got %d.\n", size, res);
		return SPIFFS_ERR_NOT_READABLE;
	}

	S_DBG("Read %d bytes from offset %d.\n", size, addr);
	//hexdump_mem(dst, size);
	return SPIFFS_OK;
}

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src) {

	int ret = SPIFFS_OK;
	u8_t *buff = 0;

	buff = malloc(size);
	if (!buff) {
		printf("Unable to malloc %d bytes.\n", size);
		ret = SPIFFS_ERR_INTERNAL;
	} else {
		ret = my_spiffs_read(addr, size, buff);
		if (ret == SPIFFS_OK) {
			int i;
			for(i = 0; i < size; i++) buff[i] &= src[i];
			//hexdump_mem(buff, size);
			if (fseek(rom, addr, SEEK_SET)) {
				printf("Unable to seek to %d.\n", addr);
				ret = SPIFFS_ERR_END_OF_OBJECT;
			} else {
				if (fwrite(src, 1, size, rom) != size) {
					printf("Unable to write.\n");
					ret = SPIFFS_ERR_NOT_WRITABLE;
				} else {
					fflush(rom);
					S_DBG("Wrote %d bytes to offset %d.\n", size, addr);
				}
			}
		}
	}

	if (buff) free (buff);
	return ret;
}

static s32_t my_spiffs_erase(u32_t addr, u32_t size) {

	int i;

	if (fseek(rom, addr, SEEK_SET)) {
		printf("Unable to seek to %d.\n", addr);
		return SPIFFS_ERR_END_OF_OBJECT;
	}

	for (i = 0; i < size; i++) {
		if (fputc(ROM_ERASE, rom) == EOF) {
			printf("Unable to write.\n");
			return SPIFFS_ERR_NOT_WRITABLE;
		}
	}

	fflush(rom);
	S_DBG("Erased %d bytes at offset %d.\n", size, addr);

	return SPIFFS_OK;
}

int my_spiffs_mount(u32_t msize) {

	spiffs_config cfg;

	cfg.phys_size = msize;
	cfg.phys_addr = 0;

	cfg.phys_erase_block = SPI_FLASH_SEC_SIZE;
	cfg.log_block_size = SPI_FLASH_SEC_SIZE * 2;
	cfg.log_page_size = LOG_PAGE_SIZE;

	cfg.hal_read_f = my_spiffs_read;
	cfg.hal_write_f = my_spiffs_write;
	cfg.hal_erase_f = my_spiffs_erase;

	int res = SPIFFS_mount(&fs,
			&cfg,
			spiffs_work_buf,
			spiffs_fds,
			sizeof(spiffs_fds),
			spiffs_cache_buf,
			sizeof(spiffs_cache_buf),
			0);
	S_DBG("Mount result: %d.\n", res);

	return res;
}

void my_spiffs_unmount() {
	SPIFFS_unmount(&fs);
}

int my_spiffs_format() {
	int res  = SPIFFS_format(&fs);
	S_DBG("Format result: %d.\n", res);
	return res;
}

int write_to_spiffs(char *fname, u8_t *data, int size) {

	int ret = 0;

	spiffs_file fd = SPIFFS_open(&fs, fname, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	if (fd < 0) {
		printf("Unable to open spiffs file '%s', error %d.\n", fname, fd);
	} else {
		S_DBG("Opened spiffs file '%s'.\n", fname);
		if (SPIFFS_write(&fs, fd, (u8_t *)data, size) < SPIFFS_OK) {
			printf("Unable to write to spiffs file '%s', errno %d.\n", fname, SPIFFS_errno(&fs));
		} else {
			ret = 1;
		}
	}

	if (fd >= 0) {
		SPIFFS_close(&fs, fd);
		S_DBG("Closed spiffs file '%s'.\n", fname);
	}
	return ret;
}

int add_file(const char* fdir, char* fname) {

	int ret = 0;
	u8_t *buff = 0;
	FILE *fp = 0;
	char *path = 0;

	path = malloc(1024);
	if (!path) {
		printf("Unable to malloc %d bytes.\n", 1024);
	} else {
		struct stat st;
		sprintf(path, "%s/%s", fdir, fname);
		if (stat(path, &st) || !S_ISREG(st.st_mode)) {
			S_DBG("Skipping non-file '%s'.\n", fname);
		} else {
			fp = fopen(path, "rb");
			if (!fp) {
				S_DBG("Unable to open '%s'.\n", fname);
			} else {
				int size = (int)st.st_size;
				buff = malloc(size);
				if (!buff) {
					printf("Unable to malloc %d bytes.\n", size);
				} else {
					if (fread(buff, 1, size, fp) != size) {
						printf("Unable to read file '%s'.\n", fname);
					} else {
						S_DBG("%d bytes read from '%s'.\n", size, fname);
						if (write_to_spiffs(fname, buff, size)) {
							printf("Added '%s' to spiffs (%d bytes).\n", fname, size);
							ret = 1;
						}
					}
				}
			}
		}
	}

	if (buff) free(buff);
	if (path) free(path);
	if (fp) fclose(fp);

	return ret;
}

int get_rom_size (const char *str) {

	long val;

	// accept decimal or hex, but not octal
	if ((strlen(str) > 2) && (str[0] == '0') &&
		(((str[1] == 'x')) || ((str[1] == 'X')))) {
		val = strtol(str, NULL, 16);
	} else {
		val = strtol(str, NULL, 10);
	}

	return (int)val;
}

int main(int argc, char **argv) {

	const char *folder;
	const char *romfile;
	int romsize;
	int ret = EXIT_SUCCESS;

	if (argc == 1) {
		romsize = DEFAULT_ROM_SIZE;
		folder = DEFAULT_FOLDER;
		romfile = DEFAULT_ROM_NAME;
		printf("Usage: %s maxFsSizeinByte spiffsBaseDir [outfile.bin]\n"
			"There is no specific size or files directory.\n"
			"Starting in compatibility mode.\n"
			"Default fs size is 0x%x (%d) bytes and directory is '%s'.\n",
			argv[0], romsize, romsize, DEFAULT_FOLDER);
	} else if (argc == 3) {
		romsize = get_rom_size(argv[1]);
		folder = argv[2];
		romfile = DEFAULT_ROM_NAME;
	} else if (argc == 4) {
		romsize = get_rom_size(argv[1]);
		folder = argv[2];
		romfile = argv[3];
	} else {
		printf ("Usage: %s <FsSizeInBytes> <FilesDir> [OutFile.bin]\n", argv[0]);
		printf ("  FsSizeInBytes can be in hex (starting with 0x) or decimal.\n");
		printf ("  To create an empty filesystem pass 'dummy.dir' as FilesDir.\n");
		exit(EXIT_FAILURE);
	}

	printf("Creating rom '%s' of size 0x%x (%d) bytes.\n", romfile, romsize, romsize);
	rom = fopen(romfile, "wb+");

	if (!rom) {
		printf("Unable to open file '%s' for writing.\n", romfile);
	} else {

		fseek(rom, romsize - 1, SEEK_SET);
		fputc(ROM_ERASE, rom);

		// we have to do this before calling format
		if (!my_spiffs_mount(romsize)) {
			my_spiffs_unmount();
		}

		int res;
		if ((res =  my_spiffs_format()) != SPIFFS_OK) {
			printf("Failed to format spiffs, error %d.\n", res);
			ret = EXIT_FAILURE;
		} else if ((res = my_spiffs_mount(romsize)) != SPIFFS_OK) {
			printf("Failed to mount spiffs, error %d.\n", res);
			ret = EXIT_FAILURE;
		} else {
			DIR *dir;
			struct dirent *ent;
			if (!strcmp(folder, "dummy.dir")) {
				printf("Creating empty filesystem.\n");
			} else if ((dir = opendir(folder)) != NULL) {
				printf("Adding files in directory '%s'.\n", folder);
				while ((ent = readdir(dir)) != NULL) {
					add_file(folder, ent->d_name);
				}
				closedir(dir);
			} else {
				printf("Unable to open directory '%s'.\n", folder);
				ret = EXIT_FAILURE;
			}
			my_spiffs_unmount();
		}
	}

	if (rom) fclose(rom);
	if (ret == EXIT_FAILURE) unlink(romfile);
	exit(ret);
}
