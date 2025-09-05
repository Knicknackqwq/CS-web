#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

void logs_get(FILE* f)
{
	char a;
	while ((a = fgetc(f)) != EOF)
	{
		printf("%c",a);
	}
}