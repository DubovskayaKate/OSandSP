#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <libgen.h>

char * resolved_path;

//Подсчитать суммарный размер файлов в заданном каталоге (аргумент 1 командной
//строки) и для каждого его подкаталога отдельно. Вывести на консоль и в файл (аргумент 2
//командной строки) название подкаталога, количество файлов в нём, суммарный размер
//файлов, имя файла с наибольшим размером.

void call_realpath (char * argv0)
{
	resolved_path = basename(argv0);
}

int getSize(char* fname)
{
	struct stat st;
	if (lstat (fname, &st) == -1) {
		fprintf (stderr, "%s: lstat() error: %s %s\n", resolved_path, strerror(errno), fname);
		exit (1);
	}
	return st.st_size;
}

int isDir(char* fname)
{
	struct stat st;
	if (lstat (fname, &st) == -1) {
		fprintf (stderr, "%s: lstat() error: %s %s\n", resolved_path, strerror(errno), fname);
		exit (1);
	}
	return S_ISDIR(st.st_mode);
}

int isFile(char* fname)
{
	struct stat st;
	if (lstat (fname, &st) == -1) {
		fprintf (stderr, "%s: lstat() error: %s %s\n", resolved_path, strerror(errno), fname);
		exit (1);
	}
	return S_ISREG(st.st_mode);
}

int notDots(char* fname)
{
	return !(!strcmp(fname, ".") || !strcmp(fname, ".."));
}

char* getFullPath(char* fname, char* path)
{
	char* fullPath = malloc(strlen(fname) + strlen(path) + 2);
	if(path[strlen(path) - 1] == '/')
		sprintf(fullPath, "%s%s", path, fname);
	else
		sprintf(fullPath, "%s/%s", path, fname);
	return fullPath;
}

char* currDir;
FILE* outputFile;

void traversal(char* path, int flag)
{
	DIR* dir;
	dir = opendir(path);
	if (dir == NULL)
	{
		fprintf (stderr, "%s: opendir() error: %s %s \n", resolved_path, strerror(errno), path);
		exit(1);
	}	

	struct dirent* entry; // массив элементов типа char с именем d_name
	int files_amount = 0;
	int files_size = 0;
	int max = -1;
	char* maxFile = malloc(1);
	maxFile = "\0";
	errno = 0;
	while ((entry = readdir(dir)) != NULL)
	{
		
		char* fileName = entry->d_name;
		if (notDots(fileName))
		{
			char* fullPath = getFullPath(fileName, path);

			if (isDir(fullPath))
			{
				
				free(currDir);
				currDir = (char*) malloc(strlen(fullPath) + 1);
				strcpy(currDir, fullPath);
			
				traversal(fullPath, 0);

				free(currDir);
				currDir = (char*) malloc(strlen(path) + 1);
				strcpy(currDir, path);
			}
			else
			{	
				if(isFile(fullPath))
				{
					int currentSize = getSize(fullPath);
					files_amount++;
					files_size += currentSize;
					if (currentSize > max)
					{
						maxFile = fileName;
						max = currentSize;
					}
				}
			}
			free(fullPath);
		}
	}
	if(errno != 0)
		fprintf(stderr, "%s: readdir() error: %s %s\n", resolved_path, strerror(errno), path);

	printf("%s %d %d %s\n",currDir, files_amount, files_size, maxFile);
	fprintf(outputFile,"%s %d %d %s\n",currDir, files_amount, files_size, maxFile);

	closedir(dir);
}


int main(int args, char **argv)
{
	if (args < 3)
	{
		printf("Not enough arguments!\n");
		return 1;
	}
	currDir = malloc(1);

	call_realpath(argv[0]);            // ./lab  
	char* initPath = argv[1];          // Directory
	outputFile = fopen(argv[2], "w+"); // out file

	traversal(initPath, 0);

	fclose(outputFile);
}

