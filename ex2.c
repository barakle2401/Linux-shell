/*
 * Barak Levy
 * ID 203280185
 * Ex2 
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <bits/fcntl-linux.h>
static int cmd_length=0;
static int num_of_cmd=0;
void printPromt();
void catchChild(int signum)
{
    signal(SIGINT,catchChild);
    signal(SIGCHLD,catchChild);
    while(waitpid(-1,NULL,WNOHANG)>0);  
}
void free_mem(char **argv,int how_many_blocks)//function that free the dynamic memory 
{   
     for(int i=0;i<how_many_blocks;i++)
          {
              free(argv[i]);
          }
     
}
void run(char ** arg,int len,bool its_amp)//excute function using fork sc 
                    //the child process run the cmd by execvp system call
{ 
    pid_t p;
    int status;
    if(strcmp(*arg,"cd")==0)//case cd command inserted 
    {
        chdir(*(arg+1));
        return;
    }
    cmd_length+=(int)strlen(*arg);//update the cmd length before fork 
    num_of_cmd++;
    if(( p = fork())<0)//fork sc child process run the command 
    {
      printf("ERR \n");
        return;
    }
    if(p == 0)
    {
        signal(SIGINT,catchChild);
       if(( execvp(*arg,arg))<0)//child process,run the command by execvp if an error occurred the error will show up 
       {
           printf("%s: command not found\n",*arg);
           free_mem(arg,len);
           exit(0);   
       }
    }
    else
    {
        if(its_amp)//case & command parent back, to get a new command
        {        
            return;
        }
           
        wait(&status);//parent wait for the child to finish his process
    }
}

char * split(char * str)//split the cmd line to an dynamic memory  
{ 
    char * other = (char *) malloc(strlen(str));
    assert(other!=NULL);
    strcpy(other,str);
    return other;
}

void printPromt()//function that print the promt to the cmd screen 
{
   
    struct passwd *pwd;
    char *userName;
    char cwd[256];
    pwd = getpwuid(0);
    userName = pwd->pw_name;//getting the user name from the struct fields
    getcwd(cwd,sizeof(cwd));
    printf("%s@%s>",userName,cwd);//print the promt line

}
void pipe_run(char **args,int index_of_pipe,int len,bool its_both)
{
    int status;
    int fd [2];
    pid_t leftpid,rightpid;
    pipe(fd);
    int right_len = len-index_of_pipe+1;
    char  file_name[510];
    char  redirection_c[2];
    int file;
    bool first_case=false,second_case=false,third_case=false;//flags for the redc signs
    if(its_both)//case redc and pipe 
    {   
        strcpy(file_name,args[len-1]);// get the file name 
        strcpy(redirection_c,args[len-2]);//get the i/o sign
        right_len = right_len -2;// -2 because > and the file name
        len = len-2;
         
        if(strcmp(redirection_c,">")==0)
        {
          
            first_case = true;
            file = open(file_name,O_WRONLY|O_CREAT|O_TRUNC,0600);
        }
        else if(strcmp(redirection_c,">>")==0)
        {
            second_case =true;
            file = open(file_name,O_WRONLY|O_CREAT|O_APPEND,0600);
        }
        else if(strcmp(redirection_c,"2>")==0)
        {
            third_case= true;
            file = open(file_name,O_WRONLY|O_CREAT|O_TRUNC,0600);
        }       
    }
    char ** left = (char **) malloc((index_of_pipe)*sizeof(char *));//left side of the pipe
    char ** right = (char **) malloc((right_len)*sizeof(char *));//right side of the pipe 
    assert(left!=NULL);
    assert(right!=NULL);
    int i,l,r;
    for(i=0,l=0,r=0;i<len;i++)//split the command for two halves 
    {
        if(i<index_of_pipe)
        {
            left[l++] = split(*(args+i));     
        }
        else
        {     
            right[r++] = split(*(args+i)); 
        }
    }
    left[l] = NULL;
    right[r] = NULL;
    cmd_length+=strlen(left[0]);
    num_of_cmd++;
    if((leftpid=fork())==0)//first child run the left
    {
        num_of_cmd++;
        cmd_length+=strlen(right[0]);
        dup2(fd[1],STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        if(execvp(*left,left)<0)
        {
            perror("command not found\n");
            exit(1);
        }   
    }
    if((rightpid=fork())==0)//second child run the right 
    {
        if(first_case||second_case)
        {
            dup2(file,STDOUT_FILENO);
        }
        if(third_case)
        {
            dup2(file,STDERR_FILENO);
        }
        dup2(fd[0],STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        if(execvp(*right,right)<0)
        {
             perror("command not found\n");
             exit(1);
        }
    }
    else
    {
        close(fd[0]);
        close(fd[1]);
        wait(&status);
        wait(&status);
        if(its_both)
             close(file);
    }
   free_mem(left,index_of_pipe);
   free_mem(right,right_len);
   free(left);
   free(right);
}
void redirection_run(char *line)
{
    bool first_case=false,second_case=false,third_case=false,fourth_case=false;
    char line_cpy[510];
    strcpy(line_cpy,line);
    char *str = strtok(line," \n");
    int len = 0;
    pid_t p;
    while(str!=NULL)
    {
        if(strcmp(str,">")==0)
        {
            
            first_case = true;
            str = strtok(NULL," \n");        
        }
        else if(strcmp(str,">>")==0)
        {
            second_case = true;
            str = strtok(NULL," \n");
        }
        else if(strcmp(str,"2>")==0)
        {
            third_case = true;
            str = strtok(NULL," \n");
        }
        else if(strcmp(str,"<")==0)
        {
            fourth_case = true;
            str = strtok(NULL," \n");
        }
        else
        {
            len++;
            str = strtok(NULL," \n");
        }
      
       
    }
    
    char **args = (char**)malloc(len*sizeof(char*));
    int i=0;
    char * file_name;
    str = strtok(line_cpy," \n");
    while(str!=NULL)
    {  
        if((strcmp(str,">")==0)||(strcmp(str,"<")==0)||(strcmp(str,">>")==0)||strcmp(str,"2>")==0)
        {//skip case redc
            str = strtok(NULL," \n");
            continue;
        }
        if(i==(len-1))//get the file name
        {
        
            file_name = split(str);
            str = strtok(NULL," \n");
            break;
        }
        else
        {
           args[i++] = split(str);
           str = strtok(NULL," \n");
        }  
    }
    args[i] = NULL;
    cmd_length+=strlen(args[0]);
    num_of_cmd++;
    
    if((p=fork())==0)
    {
        signal(SIGCHLD,catchChild);
        int fd ;
        if(first_case)// >
        {
              fd =open(file_name,O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
              dup2(fd,STDOUT_FILENO);
           
           
        }
        else if(second_case)// >>
        {
             fd =open(file_name,O_WRONLY|O_CREAT|O_APPEND,S_IRWXU);
             dup2(fd,STDOUT_FILENO); 
        }
        else if(third_case)//2>
        {
            fd = open(file_name,O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
            dup2(fd,STDERR_FILENO);         
        }
        else if(fourth_case)//<
        {
             fd = open(file_name,O_RDONLY,S_IRWXU);
             if(fd<0){
                 perror("no such file or directory\n");
                 exit(1);
             }
                 
             dup2(fd,STDIN_FILENO);
            
        }
      
        if(execvp(*args,args)<0)
        {
            perror("command not found\n");
            exit(1);
        }
        close(fd);
    }
    else
    {
       wait(NULL);
       free_mem(args,len);
       free(args);
       free(file_name);
    }
    
}
//***************************MAIN***************************************
void main(int argc, char** arg) {
    while(true)
    {      signal(SIGINT,catchChild);
           signal(SIGCHLD,catchChild);
           int pipe_index = 0;
           bool its_pipe = false,its_amp=false,its_redirection=false,its_both=false;
           char line[510];//for the cmd line 
           char line_cpy[510];//for the cmd copy
           int len = 0;
           printPromt();//print the promt line to the cmd
           if(fgets(line,510,stdin)==NULL) //get the command from the promt line,continue if an error occurred 
               continue;
           if(strcmp(line,"done\n")==0)//exit the program 
           {
               printf("Num of cmd:%d\n",num_of_cmd);
               printf("Cmd length:%d\nBye!\n",cmd_length);
               exit(0);
           }  
           strcpy(line_cpy,line);//copy of the line before strtok 
           
           char *str = strtok(line," \n");
           while(str!=NULL)
           {
               if(strcmp(str,"|")==0)
               {
                   pipe_index = len;
                   its_pipe = true;
               }  
               else if(strcmp(str,"&")==0)
               {
                   its_amp = true;
                   len--;
               }
               else if((strcmp(str,">")==0)||(strcmp(str,"<")==0)||(strcmp(str,">>")==0)||strcmp(str,"2>")==0)
               {
                   its_redirection = true;
                   if(its_pipe)//case redirection and pipe pattern
                   {
                        its_redirection = false;
                        its_both = true;
                   }                
               }
               str = strtok(NULL," \n");
               len++;
           }
           if(its_redirection)//case i/o commands
           {
               redirection_run(line_cpy);
               continue;
           }
           char **arg = (char**)malloc(len*(sizeof(char*)+1));//create args array for the execvp  
           len=0; 
           str = strtok(line_cpy," &|\n");
            
           while(str!=NULL)
           {  
               arg[len++] = split(str);//storing each string into dynamic memory using split function   
               str = strtok(NULL," &|\n");
           }
           if(its_pipe)
           {
               
              pipe_run(arg,pipe_index,len,its_both);
              free_mem(arg,len);
              free(arg);
              continue;
           }
          arg[len] = NULL;
          if((*arg)==NULL)   //case that no command entered 
          {
              free(arg);
              continue;
              
          }
          
          run(arg,len,its_amp);
          free_mem(arg,len);//free memory 
          free(arg);
         
    }
}

