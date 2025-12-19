#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
	printf("Usage: %s -n -f <database file>\n", argv[0]);
	printf("\t -f  - (required) path to database file\n");
	printf("\t -n  - create new database file\n");
	printf("\t -a  - add employee\n");
	printf("\t -l  - list employees\n");
	return;
}

int main(int argc, char *argv[]) {
	int c;
	bool newfile = false;
	bool list = false;
	char *filepath = NULL;
	char *addstring = NULL;
	int dbfd = -1;
	struct dbheader_t *dbheader = NULL;
	struct employee_t *employees = NULL;

	while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
		switch (c) {
		case 'n':
			newfile = true;
			break;
		case 'f':
			filepath = optarg;
			break;
		case 'a':
			addstring = optarg;
			break;
		case 'l':
			list = true;
			break;
		case '?':
			printf("Unknown option -%c\n", c);
			print_usage(argv);
			break;
		default:
			return -1;
		}
	}

	if (filepath == NULL) {
		printf("Filepath is a required argument\n");
		print_usage(argv);
		return 0;
	}

	if (newfile) {
		dbfd = create_db_file(filepath);
		if (dbfd == -1) {
			printf("Unable to create database file\n");
			return -1;
		}

		if (create_db_header(&dbheader) == STATUS_ERROR) {
			printf("Failed to create database header\n");
			return -1;
		}
	} else {
		dbfd = open_db_file(filepath);
		if (dbfd == STATUS_ERROR) {
			printf("Failed to open db file\n");
			return -1;
		}
		if (validate_db_header(dbfd, &dbheader) == STATUS_ERROR) {
			printf("Database file validation failed\n");
			return -1;
		}
	}

	if (read_employees(dbfd, dbheader, &employees) != STATUS_SUCCESS) {
		printf("Failed to read employees\n");
		return -1;
	}

	if (addstring) {
		if (add_employee(dbheader, &employees, addstring) != STATUS_SUCCESS) {
			printf("Failed to add employee\n");
			return -1;
		};
	}

	if (list) {
		list_employees(dbheader, employees);
	}

	output_file(dbfd, dbheader, employees);

	return 0;
}
