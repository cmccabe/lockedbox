// vim: ts=8:sw=8:tw=79:noet
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
     
void usage(void)
{
	fputs("lockedbox: locks block files\n", stderr);
	fputs("-h: this help message\n", stderr);
	fputs("-l <lock-file>: the lock file containing \
lists of files to lock\n", stderr);
}

static void chomp(char *line)
{
	while (1) {
		size_t len = strlen(line);
		if (len == 0) {
			return;
		}
		if (line[len - 1] == '\n') {
			line[len - 1] = '\0';
		} else {
			return;
		}
	}
}

static int lock_files(const char *file_name)
{
	char line[PATH_MAX], *s;
	size_t line_len = sizeof(line);
	int num_failed = 0, err, nline = 0, fd = -1;
	FILE *fp = fopen(file_name, "r");
	struct stat st_buf;
	void *map;

	if (!fp) {
		err = errno;
		fprintf(stderr, "error opening %s: %s\n", file_name, strerror(errno));
		exit(1);
	}
	while (1) {
		if (fd >= 0) {
			close(fd);
			fd = -1;
		}
		s = fgets(line, line_len, fp);
		if (!s) {
			break;
		}
		chomp(line);
		fd = open(line, O_RDONLY);
		if (fd < 0) {
			err = errno;
			fprintf(stderr, "failed to open %s: %s\n", line, strerror(err)); 
			num_failed++;
			continue;
		}
		if (fstat(fd, &st_buf) < 0) {
			err = errno;
			fprintf(stderr, "failed to stat %s: %s\n", line, strerror(err)); 
			num_failed++;
			continue;
		}
		map = mmap(NULL, st_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (map == MAP_FAILED) {
			err = errno;
			fprintf(stderr, "mmap failed for %s: %s\n", line, strerror(err)); 
			num_failed++;
			continue;
		}
		if (mlock(map, st_buf.st_size) < 0) {
			err = errno;
			fprintf(stderr, "mlock failed for %s: %s\n", line, strerror(err)); 
			num_failed++;
			continue;
		}
		fprintf(stderr, "locked %s\n", line);
		// deliberately leak mmap
	}
	fclose(fp);
	return nline;
}

int main (int argc, char **argv)
{
	int c, ret;
	const char *lock_list_file = NULL;

	while ((c = getopt (argc, argv, "hl:")) != -1) {
		switch (c) {
		case 'h':
			usage();
			exit(0);
			break;
		case 'l':
			lock_list_file = optarg;
			break;
		}
	}
	if (!lock_list_file) {
		fputs("You must specify a lock list file with -l\n", stderr);
		exit(1);
	}
	ret = lock_files(lock_list_file);
	if (ret == 0) {
		fprintf(stderr, "locked all requested block files.\n");
	} else {
		fprintf(stderr, "FAILED to lock %d requested block files.\n", ret);
	}
	while (1) {
		sleep(1000000);
	}
	return 0;
}
