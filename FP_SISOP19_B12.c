#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

typedef struct skedul
{
    char min[4];
    char hour[4];
    char date[4];
    char mon[4];
    char day[4];
    char command[1000];
    char path[1000];
} schedule;

int stopthread=0;

void* ruuun(void *waktu)
{
    char mins[4], hours[4], dates[4], mons[4], days[4], loopmin[4], command[1000];

    while(stopthread!=1)
    {
        time_t sekarang = time(NULL);
        time_t now = time(NULL);
        struct tm tema = *localtime(&sekarang);
        struct tm cekloop;
        
        schedule* wakz = (schedule*)waktu;
        memset(mins,'\0',sizeof(mins));
        memset(hours,'\0',sizeof(hours));
        memset(dates,'\0',sizeof(dates));
        memset(mons,'\0',sizeof(mons));
        memset(days,'\0',sizeof(days));

        sprintf(mins, "%d", tema.tm_min);
        sprintf(hours, "%d", tema.tm_hour);
        sprintf(dates, "%d", tema.tm_mday);
        sprintf(mons, "%d", tema.tm_mon);
        sprintf(days, "%d", tema.tm_wday);
        
        sleep(1);

        if(strcmp(wakz->min,"*") == 0 || atoi(mins) == atoi(wakz->min))
        {
            if(strcmp(wakz->hour,"*") == 0  || atoi(hours) == atoi(wakz->hour))
            {
                if(strcmp(wakz->date,"*") == 0 || atoi(dates) == atoi(wakz->date))
                {
                    if(strcmp(wakz->mon,"*") == 0 || (atoi(mons)+1) == atoi(wakz->mon))
                    {
                        if(strcmp(wakz->day,"*") == 0 || atoi(days) == atoi(wakz->day))
                        {                            
                            // PAKE SYSTEM
                            // memset(command,'\0',sizeof(command));
                            // sprintf(command, "%s %s", wakz->command, wakz->path);
                            // printf("%s\n", command);
                            // system(command);

                            //fork-exec
                            pid_t child_id;
                            child_id = fork();
                            if (child_id == 0) {
                                // this is child

                                char *argv[] = {wakz->command, wakz->path, NULL};
                                execv(wakz->command, argv);
                            }

                            now = time(NULL);   //ngambil waktu saat ini
                            cekloop = *localtime(&now);
                            memset(loopmin,'\0',sizeof(loopmin));
                            sprintf(loopmin, "%d", cekloop.tm_min);
                            while(strcmp(mins,loopmin) == 0 && stopthread!=1)
                            {
                                // muter-muter di sini sampe ganti menit
                                now = time(NULL);
                                memset(loopmin,'\0',sizeof(loopmin));
                                cekloop = *localtime(&now);
                                sprintf(loopmin, "%d", cekloop.tm_min);
                                sleep(1);
                            }                            
                        }
                    }
                }       
            }
        }
    }
    return 0;
}

int main() {
  pid_t pid, sid;

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();

  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/home/Penunggu/sisop/FP")) < 0) {
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  int oldMtime = 0, nowMtime, pertama = 1;
  pthread_t tid[100];
  int t=0;
  
  while(1) {
    // main program here
    struct stat filecron;

    int err = stat("crontab.data", &filecron);
    nowMtime = filecron.st_mtime;
        
    if(nowMtime > oldMtime)
    {
        int i;
        if(t!=0)
        {
          stopthread=1;
          for(i=0; i<t; i++) {pthread_join(tid[i], NULL);}
          t=0;
        }
        //printf("File modified\n");

        FILE *kertas = fopen("crontab.data", "r");
        char perintah[1000][1000], mins[5], hours[5], com[1000], path[1000];
        int c, j=0, cas=1, k=0;
        memset(perintah,'\0',sizeof(perintah));
        memset(mins,'\0',sizeof(mins));
        memset(hours,'\0',sizeof(hours));
        memset(com,'\0',sizeof(com));
        memset(path,'\0',sizeof(path));
        i=0;
        while((c = fgetc (kertas)) != EOF)
        {
          if(c == '\n')
          {
            i++;
            j=0;
          }
          else
          {
            perintah[i][j] = c;
            j++;   
          }
        }
        j=0; i=0; stopthread=0;
            
        while(perintah[i][j]!='\0')
        {
          schedule *wakt = (schedule*)malloc(sizeof(schedule));
          memset(wakt,'\0',sizeof(wakt));

          while(perintah[i][j]!='\0')
          {
            if(perintah[i][j] == ' ')
            {
              cas++;
              k=0;
            }
            else
            {
              if(cas==1)
              {
                wakt->min[k]=perintah[i][j];
                k++;
              }
              else if(cas==2)
              {
                wakt->hour[k]=perintah[i][j];
                k++;
              }
              else if(cas==3)
              {
                wakt->date[k]=perintah[i][j];
                k++;
              }
              else if(cas==4)
              {
                wakt->mon[k]=perintah[i][j];
                k++;
              }
              else if(cas==5)
              {
                wakt->day[k]=perintah[i][j];
                k++;
              }
              else if(cas==6)
              {
                wakt->command[k]=perintah[i][j];
                k++;
              }
              else if(cas==7)
              {
                wakt->path[k]=perintah[i][j];
                k++;
              }
            }
            j++;
          }
          j=0; cas=1;
                
          int minss;
          if(mins[0]!='*')
          {minss=atoi(mins);}
          else
          {minss=mins[0];}

          pthread_create(&(tid[t]), NULL, ruuun, (void*)wakt);
          t++;

          memset(mins,'\0',sizeof(mins));
          memset(hours,'\0',sizeof(hours));
          memset(com,'\0',sizeof(com));
          memset(path,'\0',sizeof(path));
                
          j=0; i++; k=0;
        }
        j=0; i=0;
        fclose(kertas);
    }
    // else
    // {
    //   printf("File not modified\n");
    // }
        
    oldMtime = nowMtime;

    sleep(5);
  }
  
  exit(EXIT_SUCCESS);
}