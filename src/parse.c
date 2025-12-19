#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

int create_db_header(struct dbheader_t **headerOut) {
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

int output_file(int fd, struct dbheader_t *dbheader,
				struct employee_t *employees) {
	int realcount = dbheader->count;
	dbheader->magic = htonl(dbheader->magic);
	dbheader->filesize = htonl(sizeof(struct dbheader_t) +
							   sizeof(struct employee_t) * realcount);
	dbheader->count = htons(dbheader->count);
	dbheader->version = htons(dbheader->version);

	lseek(fd, 0, SEEK_SET);
	write(fd, dbheader, sizeof(struct dbheader_t));

	for (int i = 0; i < realcount; i++) {
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbheader,
				   struct employee_t **employeesOut) {
	int count = dbheader->count;
	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if (employees == NULL) {
		printf("Malloc failed for creating employees\n");
		return STATUS_ERROR;
	}

	read(fd, employees, count * sizeof(struct employee_t));

	for (int i = 0; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *dbheader, struct employee_t **employees,
				 char *addstring) {

	char *name = strtok(addstring, ",");
	if (name == NULL) {
		printf("Failed to parse employee details - fail at name\n");
		return STATUS_ERROR;
	}
	char *address = strtok(NULL, ",");
	if (address == NULL) {
		printf("Failed to parse employee details - fail at address\n");
		return STATUS_ERROR;
	}
	char *hours = strtok(NULL, ",");
	if (hours == NULL) {
		printf("Failed to parse employee details - fail at hours\n");
		return STATUS_ERROR;
	}
	char *endptr = NULL;
	long actual_hours = strtol(hours, &endptr, 10);

	if (endptr == hours) {
		printf("Failed to parse employee details - fail at hours\n");
		return STATUS_ERROR;
	} else if (*endptr != '\0') {
		printf("Failed to parse employee details - fail at hours\n");
		return STATUS_ERROR;
	}

	struct employee_t *e = *employees;
	dbheader->count++;
	e = realloc(e, sizeof(struct employee_t) * dbheader->count);

	if (e == NULL) {
		printf("Failed to realloc employees");
		return STATUS_ERROR;
	}

	strncpy(e[dbheader->count - 1].name, name,
			sizeof(e[dbheader->count - 1].name) - 1);
	strncpy(e[dbheader->count - 1].address, address,
			sizeof(e[dbheader->count - 1].address) - 1);
	e[dbheader->count - 1].hours = actual_hours;

	*employees = e;

	return STATUS_SUCCESS;
}

int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
	if (dbhdr == NULL) {
		printf("Really?\n");
		return -1;
	}
	printf("\n=======================\n");
	printf("Database contains %d employees\n", dbhdr->count);
	printf("\n");
	printf("---\n");
	for (int i = 0; i < dbhdr->count; i++) {
		printf("Employee: %d\n", i);
		printf("\tName: %s\n", employees[i].name);
		printf("\tAddress: %s\n", employees[i].address);
		printf("\tHours: %d\n", employees[i].hours);
		printf("---\n");
	}
	printf("\n=======================\n");
	return STATUS_SUCCESS;
}
