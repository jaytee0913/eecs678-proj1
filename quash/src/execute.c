/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>

#include "quash.h"

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

int *fd;
int idx;
int idx_pos;

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple
  // IMPLEMENT_ME();

  // Change this to true if necessary
  *should_free = true;

  return getcwd(NULL, 1024);
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  // IMPLEMENT_ME();

  // TODO: Remove warning silencers
  // (void) env_var; // Silence unused variable warning
  return getenv(env_var);
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  // IMPLEMENT_ME();

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  // TODO: Remove warning silencers
  // (void) exec; // Silence unused variable warning
  // (void) args; // Silence unused variable warning

  // TODO: Implement run generic
  // IMPLEMENT_ME();

  if(execvp(exec, args) < 0)
    perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  // TODO: Remove warning silencers
  // (void) str; // Silence unused variable warning

  // TODO: Implement echo
  // IMPLEMENT_ME();
  for(; *str != NULL; ++str)
    printf("%s ",*str);
  printf("\n");

  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  // TODO: Remove warning silencers
  // (void) env_var; // Silence unused variable warning
  // (void) val;     // Silence unused variable warning

  // TODO: Implement export.
  // HINT: This should be quite simple.
  // IMPLEMENT_ME();
  setenv(env_var, val, 1);
}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;

  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // Change directory
  chdir(dir);

  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  // IMPLEMENT_ME();
  const char* old_dir = getenv("PWD");
  if(setenv("PWD", dir, 1) < 0)
    perror("ERROR: Failed to update PWD");
  if(setenv("OLD_PWD", old_dir, 1) < 0)
    perror("ERROR: Failed to update OLD_PWD"); 
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  (void) signal; // Silence unused variable warning
  (void) job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  IMPLEMENT_ME();
}


// Prints the current working directory to stdout
void run_pwd() {
  // TODO: Print the current working directory
  // IMPLEMENT_ME();

  // Flush the buffer before returning
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
    printf("%s\n", cwd);
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  // TODO: Print background jobs
  IMPLEMENT_ME();

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder) {
  // Read the flags field from the parser

  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  // TODO: Remove warning silencers
  // (void) p_in;  // Silence unused variable warning
  // (void) p_out; // Silence unused variable warning
  // (void) r_in;  // Silence unused variable warning
  // (void) r_out; // Silence unused variable warning
  // (void) r_app; // Silence unused variable warning

  // TODO: Setup pipes, redirects, and new process
  // IMPLEMENT_ME();
  int f_open;
  pid_t pid;
  pid = fork();
  if(pid == 0) {
      
    /* Pipe code */
    /* Get input from prev pipe, output to pipe */
    if(p_in && p_out){
      int pipe_end = idx_pos * 2;
      dup2(fd[pipe_end - 2], STDIN_FILENO);//pipe_end - 2 is read end of prev pipe
      close(fd[pipe_end - 1]);

      dup2(fd[pipe_end + 1], STDOUT_FILENO);//pipe_end + 1 is write end of current pipe
      close(fd[pipe_end]);

      /* Close unused pipes */
      for(int i = 0; i < pipe_end - 2; ++i)
        close(fd[i]);
      for(int i = pipe_end + 2; i < idx * 2; ++i)
        close(fd[i]);
    }
    /* Get input from prev pipe */
    else if(p_in) {
      dup2(fd[idx * 2 - 2], STDIN_FILENO);
      close(fd[idx * 2 - 1]);

      /* Close unused pipes */
      for(int i = 0; i < idx * 2 - 2; ++i)
        close(fd[i]);
    }
    /* Output to pipe */
    else if(p_out) {
      dup2(fd[1], STDOUT_FILENO);
      close(fd[0]);

      /* Close unused pipes */
      for(int i = 2; i < idx * 2; ++i)
        close(fd[i]);
    }

    /* Redirection code */
    if( r_in ) {
      f_open = open(holder.redirect_in, O_RDONLY);
      dup2( f_open, STDIN_FILENO );
      close( f_open );
    }
    else if(r_out) {
      if(r_app) {
        f_open = open(holder.redirect_out, O_APPEND | O_WRONLY | O_CREAT, 0777);
        dup2(f_open, STDOUT_FILENO);
        close(f_open);
      } 
      else {
        f_open = open(holder.redirect_out, O_WRONLY | O_CREAT, 0777);
        dup2(f_open, STDOUT_FILENO);
        close(f_open);
      }
    }
    child_run_command(holder.cmd); // This should be done in the child branch of a fork
   
    exit(0);
  }
  else {
    push_back_pid_queue (&p_queue, pid);
    parent_run_command(holder.cmd); // This should be done in the parent branch of
                                    // a fork
  }
}

// Run a list of commands
void run_script(CommandHolder* holders) {
  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;
  
  /* Determine the number of commands and thus the number of pipes, idx will denote the pipe we are at */
  idx = 0;
  for (idx; (type = get_command_holder_type(holders[idx])) != EOC; ++idx){}

  /* Number of pipes will be one less than the number of commands */
  --idx;
  if(idx < 1)
    idx = 1;

  /* Allocate ports for the pipes, need 2 ends for each pipe */
  fd = (int *)malloc(idx * 2 * sizeof(int));
  /* idx_pos is a global which will let create_process() keep track of the pipe we are at */
  idx_pos = 0;

  /* Create pipes */
  for (int i = 0; i < idx; ++i) {
    pipe(&fd[i*2]);
  }

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i){
    create_process(holders[i]);
    ++idx_pos;
  }

  /* Close pipes */
  for(int i = 0; i < idx; ++i){
    close(fd[i*2]);
    close(fd[i*2+1]);
  }

  /* Reuse the pipes' ports for future commands */
  free(fd);
  fd = NULL;

  int status;
  int fg_pid;
  job_struct js;

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    // IMPLEMENT_ME();
    while(is_empty_pid_queue (&p_queue) == false){
      fg_pid = pop_back_pid_queue(&p_queue);  
      waitpid(fg_pid, &status, 0);
    }
  }
  else {
    // A background job.
    // TODO: Push the new job to the job queue
    // IMPLEMENT_ME();
    
    int bg_pid;
    bg_pid = pop_front_pid_queue (&p_queue);
    js.job_id = 0;//bg_jobs_count;
    js.process_queue = &p_queue;

    push_back_job_queue (&j_queue, js);
    // TODO: Once jobs are implemented, uncomment and fill the following line
    // print_job_bg_start(job_id, pid, cmd);
    print_job_bg_start(js.job_id, bg_pid, "TODO");
  }
}
