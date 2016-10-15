# shell-prog
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <signal.h>
    
    #define MAX_BUFFER 1024     // max line buffear
    #define MAX_ARGS 64         // max # args
    #define SEPARATORS " \t\n"  // token sparators
    void env(char **e);    //function declarations
    void syserr(char * msg);
    void setMyShellEnv();
    
    int i=0;
    char *command[MAX_ARGS];
    extern char **environ; //env variables
    extern int errno;      // system error number
    pid_t pid;             // process ID
    int status;             // status for fork/exec process
    int in, out, input, output, append; // I/O redirection parameters
    char *inputFile, *outputFile; // I/O input and output files
    FILE *fp;             //pointer to file for ouput file 
    
    // get the environment variables
    
    void env(char **e){
      char **env = e;
      // IO redirection
      if (output == 1){
        fp = fopen(outputFile, "w");
      }
      else if (append == 1){
        fp = fopen(outputFile, "a");
      }
      
      //if ouput or append then fprintf
      if (output == 1 || append == 1){
        while(*env){
          fprintf(fp,"%s\n", *env++);
        }
        fclose(fp);
      }
      
      else{
        while(*env){
          printf("%s\n", *env++);
        }
      }  
    }
    
    //set the shell environment variable to the <homepath>/myshell
    void setMyShellEnv(){
      char home_path[1024];
      getcwd(home_path, 1024);
      strcat(home_path, "/myshell");
      setenv("shell", home_path, 1);
    }
    
    //function is called when error messeges arise
    void syserr(char * msg)
    {
      fprintf(stderr, "%s: %s\n", strerror(errno), msg);
      abort();
    }
    
    //the main function
    int main(int argc, char ** argv){
      char buf[MAX_BUFFER];
       char * args[MAX_ARGS];
      char ** arg;
      char * prompt = "wp671028 >";
      int dont_wait =0;
      int status;
      char commandstrin[50];
      setMyShellEnv(); // get the shell environment
      if(argc > 1) {
        freopen(argv[1], "r", stdin);
      }
    
     while(!feof(stdin)){
        fputs(prompt, stdout);
        if(fgets(buf, MAX_BUFFER, stdin)){
          arg = args;
          *arg++ = strtok(buf,SEPARATORS);
    
          while ((*arg++ = strtok(NULL,SEPARATORS)));
    
          if (args[0]) {
            // if there was an input redirection (<) 
            if (input == 1){
              if(!access(inputFile, R_OK)){ //check access
                freopen(inputFile, "r", stdin); // replace the stdin with the file
              }
            }
    
    //for clr command
            if (!strcmp(args[0],"clr")) { // clear
              pid = getpid(); // get process id
              switch(pid = fork()){ 
                case -1:
                  syserr("fork"); //error
                case 0:
                  setenv("parent", getenv("shell"), 1);  //set parent
                  execvp("clear", args); //execute in the child thread
                  syserr("execvp"); //error
                default:
                  if(!dont_wait)  //determine if wait is needed for background execution
                    waitpid(pid, &status, WUNTRACED); 
              }
              continue;        
            }
    //for dir command 
            if (!strcmp(args[0],"dir")) {
                
              pid = getpid();
           
              switch(pid = fork()){
                case -1:
                  syserr("fork");
                case 0:
                  setenv("parent", getenv("shell"), 1); //set parent
                  //set the i/o redirection
                  if(output == 1)
                    freopen(outputFile, "w", stdout); //replace the stdout with the outputfile
                  else if(append == 1)
                    freopen(outputFile, "a+", stdout); //replace the stdout with the outputfile (append)
                  // if there is a argument after the "dir" command ls for that directory
                  if (args[1]) {                    
                    if(execl("/bin/ls", "ls", "-al", args[1], NULL)){ //execute in the child thread
                    syserr("execl");              
                    exit(1); // exit so that the next part isn't run
                    }
                  }
                  // otherwise just ls for the current directory
                  if (execl("/bin/ls", "ls", "-al", NULL) < 0) {   
                    syserr("execl");
                    exit(1);
                  }
                  syserr("execl");
                default:
                  if(!dont_wait) //determine wait for background execution
                    waitpid(pid, &status, WUNTRACED);
                }
              continue;
            }
    
            //get the environment variables of the shell
            if (!strcmp(args[0],"environ")) {

              env(environ); //call helper
              continue;
            }
    
            //for run command type run along with argument beside it 
    if(!strcmp(args[0],"run"))
    {
   if(args[1]){
strcpy(commandstrin,args[1]);
}

       system(commandstrin);
     
              continue;
            }
    
            if (!strcmp(args[0],"quit")) { 
              break; //break the loop so the program returns and ends
            }
    
            else{
              pid = getpid();   
              switch (pid = fork ()) { 
                case -1:
                  syserr("fork"); 
                case 0:
                  setenv("parent", getenv("shell"), 1); //set parent
                  //i/o redirection
                  if(output == 1)
                    freopen(outputFile, "w", stdout);
                  else if(append == 1)
                    freopen(outputFile, "a+", stdout); 
    
                  execvp (args[0], args); //execute in child thread
                  syserr("exec");
                default:                
                  if (!dont_wait) //determine background execution wait (&)
                    waitpid(pid, &status, WUNTRACED);
            }
             
             continue;
            }
          }
    
        }
    
      }
      return 0;
    }


