#ifndef _Debug_H
#define _Debug_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define DEBUG 1
// #define DEBUG 0

typedef struct {
	char* header;
	int sdebug;
} Debugger;

Debugger DebuggerCreate(char* header) {
	Debugger d;
	d.header = header;
	d.sdebug = 1;
	return d;
}

void DebuggerPrint(Debugger d, const char* format, ...) {
	if (!DEBUG) return;
	if (!d.sdebug) return;

	time_t t = time(NULL);
	char time_str[100];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&t));

	va_list args;
	va_start(args, format);

	printf("[%s]", time_str);
	printf("[%s]", d.header);
	vprintf(format, args);

	va_end(args);
}

void DebuggerPrintln(Debugger d, const char* format, ...) {
	DebuggerPrint(d, format);
	printf("\n");
}

void DebuggerEnable(Debugger d) {
	d.sdebug = 1;
}

void DebuggerDisable(Debugger d) {
	d.sdebug = 0;
}
#endif