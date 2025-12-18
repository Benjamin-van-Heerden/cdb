#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

int create_db_header(int fd, struct dbheader_t **headerOut) {
	struct dbheader_t *header = malloc(sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed to create db header");
		return STATUS_ERROR;
	}

	header->version = 0x1;
	header->count = 0;
	header->magic = HEADER_MAGIC;
	header->filesize = sizeof(struct dbheader_t);

	*headerOut = header;

	output_file(fd, header);

	return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
	struct dbheader_t *header = malloc(sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed to create db header");
		return STATUS_ERROR;
	}

	if (read(fd, header, sizeof(struct dbheader_t)) !=
		sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);

	if (header->version != 1) {
		printf("Improper header version\n");
		return STATUS_ERROR;
	}
	if (header->magic != HEADER_MAGIC) {
		printf("Improper header magic\n");
		return STATUS_ERROR;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);

	if (header->filesize != dbstat.st_size) {
		printf("Corrupted database\n");
		return STATUS_ERROR;
	}

	*headerOut = header;

	return STATUS_SUCCESS;
}

void output_file(int fd, struct dbheader_t *dbheader) {
	dbheader->magic = htonl(dbheader->magic);
	dbheader->filesize = htonl(dbheader->filesize);
	dbheader->count = htons(dbheader->count);
	dbheader->version = htons(dbheader->version);

	lseek(fd, 0, SEEK_SET);
	write(fd, dbheader, sizeof(struct dbheader_t));
}
