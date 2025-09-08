#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <chrono>

#define LOG "logs.log"


int download(char* ip, char* filename,char *mode);
void logs_get(FILE* f);
int upload(char* server_ip, char* filename, char* mode);
