#include "tftp.h"

int main(int arg, char* args[])
{
	if (arg == 1)				//未输入任何参数
	{
		printf("Wrong command!");
		exit(1);
	}

	else if (arg == 2)
	{
		if (!(strcmp(args[1], "-h")))
		{
			//-h命令，help
			FILE *f;
			f=fopen("help.txt","r");
			if (f == NULL)
			{
				printf("help.txt failed!");
				exit(-1);
			}
			else
			{
				char a;
				while ((a = fgetc(f)) != EOF)
					printf("%c", a);
			}
		}
		else if (!(strcmp(args[1], "-v")))
		{
			FILE* f;
			f = fopen(LOG, "r");
			logs_get(f);
			fclose(f);
		}
		else 
		{
			printf("Wrong command!");
			exit(1);
		}
	}
	else if (arg == 5)
	{
		if (!(strcmp(args[1], "-d")))
			//下载文件
		{
			char ip[10];
			char filename[20];
			char mode[10];
			FILE* f;
			strcpy(ip, args[2]);
			strcpy(filename, args[3]);
			strcpy(mode, args[4]);
			auto downloadStartTime = std::chrono::high_resolution_clock::now();
			auto downloadEndTime = std::chrono::high_resolution_clock::now();
			//download命令
			if (download(ip, filename,mode) == 1)
			{
				f = fopen(LOG, "a+");
				time_t a = time(NULL);
				struct tm* b = localtime(&a);
				char e[30];
				strcpy(e, asctime(b));
				int p;
				p = strlen(e) - 1;
				e[p] = 0;
				fprintf(f, "[%s] DOWNLOAD INFO Connection closed!\n", e);
				fclose(f);
				printf("Connection closed!\n");
				//记录日志
				std::chrono::duration<double> downloadTimeInSeconds = downloadEndTime - downloadStartTime;
				std::streampos fileSize = std::ifstream(args[3], std::ios::ate | std::ios::binary).tellg();
				double downloadThroughput = fileSize / 1024.0 / downloadTimeInSeconds.count();
				std::cout << "Download throughput: " << downloadThroughput << " KiB/s" << std::endl;
			}
			else
			{
				f = fopen(LOG, "a+");
				time_t a = time(NULL);
				struct tm* b = localtime(&a);
				char e[30];
				strcpy(e, asctime(b));
				int p;
				p = strlen(e) - 1;
				e[p] = 0;
				fprintf(f, "[%s] DOWNLOAD ERROR FAIL!\n", e);
				fclose(f);
				printf("Download Error!\n");
				//记录日志
			}
		}

		else if (!(strcmp(args[1], "-u")))
			//上传文件
		{
			FILE* f;
			auto downloadStartTime = std::chrono::high_resolution_clock::now();
			auto downloadEndTime = std::chrono::high_resolution_clock::now();

			if (upload(args[2], args[3], args[4]) == 0)
			{
				f = fopen(LOG, "a+");
				time_t a = time(NULL);
				struct tm* b = localtime(&a);
				char e[30];
				strcpy(e, asctime(b));
				int p;
				p = strlen(e) - 1;
				e[p] = 0;
				fprintf(f, "[%s] UPLAOD INFO %s %s Request is sent!\n", e, args[2],args[3]);
				fprintf(f, "[%s] UPLOAD INFO Connection closed!\n", e);
				fclose(f);
				printf("Connection closed!\n");
				//记录日志
				std::chrono::duration<double> downloadTimeInSeconds = downloadEndTime - downloadStartTime;
				std::streampos fileSize = std::ifstream(args[3], std::ios::ate | std::ios::binary).tellg();
				double downloadThroughput = fileSize / 1024.0 / downloadTimeInSeconds.count();
				std::cout << "Download throughput: " << downloadThroughput << " KiB/s" << std::endl;
			}
			else
			{
				f = fopen(LOG, "a+");
				time_t a = time(NULL);
				struct tm* b = localtime(&a);
				char e[30];
				strcpy(e, asctime(b));
				int p;
				p = strlen(e) - 1;
				e[p] = 0;
				fprintf(f, "[%s] DOWNLOAD ERROR FAIL!\n", e);
				fclose(f);
				printf("UPLOAD Error!\n");
				//记录日志
			}

		}
		
	}

	return 0;
}