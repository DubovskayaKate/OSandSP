#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#define PATH_LEN 65535

//Написать программу подсчета количества слов в файлах заданного
//каталога его подкаталогов. Пользователь задаёт имя каталога. Главный
//процесс открывает каталоги и запускает для каждого файла каталога
//отдельный процесс подсчета количества слов. Каждый процесс выводит
//на экран свой pid, полный путь к файлу, общее число просмотренных байт
//и количество слов. Число одновременно работающих процессов не
//должно превышать N (вводится пользователем). Проверить работу
//программы для каталога /etc.


static char* prog_name;

int getSize(char* fname)
{
    FILE* f;
    f = fopen(fname, "r");

    if (f == NULL)
    {
        fprintf(stderr, "%d fopen() error: %s %s\n",getpid(), strerror(errno), fname);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    fclose(f);
    return size;
}


char* basename(char *filename)
{
    char *p = strrchr(filename, '/');
    return p ? p + 1 : filename;
}

int getWordCount(char* fname)
{
    int CHUNK = SHRT_MAX; //32767
    char buf[CHUNK];

    FILE *file;
    size_t nread;

    file = fopen(fname, "r");
    
    int tot_words = 0;  
    int in_space = 1;
    
    if (file) 
    {		
        while ((nread = fread(buf, 1, CHUNK, file)) > 0)
        {
            for(int i = 0; i < nread; i++)
            {      
                if (isspace(buf[i])) //Проверка на пробельный символ
                {
                    in_space = 1;
                } 
                else 
                {
                    tot_words += in_space;
                    in_space = 0;
                }
            }
        }
        if (ferror(file)) 
        {
            fprintf (stderr, "%d fread() error: %s %s\n", getpid(),strerror(errno), fname);
            exit(1);
        }

        fclose(file);
        return tot_words;
    } 
    else
    {
        
        fprintf (stderr, "%d fopen() error: %s %s\n",  getpid(), strerror(errno), fname);
        exit(1);    
    } 
}


int maxAmount = 0;
int currAmount = 0;
pid_t pid;

void walk_dir(const char *src_dir)
{
    DIR *curr = opendir(src_dir);

    if (!curr) 
    {
	fprintf(stderr, "%d opendir() error %s %s", getpid(), strerror(errno), src_dir);
        return;
    }

    struct dirent *info;

    char *full_path = (char*)malloc(PATH_LEN * sizeof (char));

    if (!full_path) 
    {
	fprintf(stderr, "%d malloc() error %s %s", getpid(), strerror(errno), src_dir);
        return;
    }

    if (!realpath(src_dir, full_path))
    {
	fprintf(stderr, "%d realpath() error %s %s", getpid(), strerror(errno), src_dir);
        free(full_path);
        closedir(curr);
        return;
    }

    strcat(full_path, "/");
    size_t base_dir_len = strlen(full_path);
    errno = 0;

    while ((info = readdir(curr))) 
    {

        full_path[base_dir_len] = 0;
        strcat(full_path, info->d_name);

        if (info->d_type == DT_DIR && strcmp(info->d_name, ".") && strcmp(info->d_name, ".."))
        {
            walk_dir(full_path);
        }

        if (info->d_type == DT_REG) 
        {	
	    //printf("%s\n",full_path);
            if (currAmount < maxAmount) 
            {
                currAmount++;
                pid = fork();
            }    

            if (pid == 0)
            {
                int amount = getWordCount(full_path);
                printf("%d %s %d %d\n", getpid(),full_path, getSize(full_path), amount);
                exit(0);
            }

            if (currAmount == maxAmount)
            {
                int status = 0;
                wait(&status);
                currAmount--;  
            }    
        }
    }

    if (errno)
        fprintf (stderr, "%d readdir() error: %s %s\n", getpid(),strerror(errno), full_path);

    free(full_path);
    closedir(curr);
}

int main(int argc, char *argv[])
{
    prog_name = basename(argv[0]);

    if (argc != 3) 
    {
         fprintf (stderr, "Program need 2 arguments");
        return 1;
    }

    char *src_dir = argv[1];
    maxAmount = atoi(argv[2]) - 1; 
    walk_dir(src_dir);


    for (int i = 0; i < currAmount; i++)
    {
        int status;
        wait(&status);
    }

    return 0;
}

