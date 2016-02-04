/*
 ============================================================================
 Name        : lab.c
 Author      : Myron Marintsev
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#define HEADER_SIZE	(sizeof(struct header_t))

#pragma pack(1)
struct header_t {
	int32_t version;
	int64_t avail;
};

#define ERRNO 1

#define VERSION (0x00020000)

#define BYTE( x, n ) (((x)>>((n)*8))&(0xff))
#define SBYTE( x, n ) (((x)&(0xff))<<((n)*8))
#define BIG_ENDIAN32( x ) SBYTE(BYTE(x,0),3) | SBYTE(BYTE(x,1),2) | SBYTE(BYTE(x,2),1) | SBYTE(BYTE(x,3),0)

void wtf(int code, char * message, ...) {
	va_list ap;
	va_start(ap, message);
	if (code == ERRNO)
		print_errno();
	fprintf( stderr, "[WTF]: ");
	vfprintf( stderr, message, ap);
	fprintf( stderr, "\n");

	va_end(ap);
	exit(code);
}

// #define ERRNOS 2
int errno_nos[] = { EPERM, ENOENT, 0 };
char * errno_names[] = { "EPERM", "ENOENT", NULL };

char * find_errno_name() {
	int val = errno;
	int i;
	char * name;
	for (i = 0; errno_nos[i]; i++) {
		if (val == errno_nos[i]) {
			name = errno_names[i];
			break;
		}

	}
	if (name)
		return errno_names[i];
	return "?";
}

void print_errno() {
	fprintf( stderr, "errno=%d (%s)\n", errno, find_errno_name());
}

int main(void) {
	assert(sizeof(struct header_t) == 8 + 4);

	char * filename = "database.bin";

	FILE * f = fopen(filename, "r+");
	if (f == NULL) {
		if ( errno == ENOENT) {
			f = fopen(filename, "w+");
			if (f == NULL)
				wtf( ERRNO, "Не могу создать файл.");
		} else
			wtf( ERRNO, "Не могу открыть файл.");
	}

	int code;

	struct header_t header;
	long length;
	if (!!fseek(f, 0, SEEK_END))
		wtf( ERRNO, "Не могу перейти к концу файла.");
	length = ftell(f);

	if (length < HEADER_SIZE) {
		if (!!ftruncate(fileno(f), HEADER_SIZE))
			wtf( ERRNO,
					"Не могу сделать размер файла достаточным для заголовка.");
		header.version = BIG_ENDIAN32(VERSION);

		// указатель на следующий байт после заголовка
		// там находится длина свободного блока и указатель на следующий блок
		header.avail = HEADER_SIZE;
		if (1 != (code = fwrite(&header, HEADER_SIZE, 1, f)))
			wtf( ERRNO, "Не могу впервые записать заголовок.");
	} else {
		if (!!fseek(f, 0, SEEK_SET))
			wtf( ERRNO, "Не могу перейти в начало файла.");
		if (1 != (code = fread(&header, HEADER_SIZE, 1, f)))
			wtf( ERRNO,
					"Не могу прочитать заголовок (HEADER_SIZE=%d, code=%d).",
					HEADER_SIZE, code);

		if (!(header.version == BIG_ENDIAN32(VERSION)))
			wtf(3, "Версия не соответствует.");


	}

	if (0 != fclose(f))
		wtf(2, "Не могу закрыть файл.");
	return EXIT_SUCCESS;
}
