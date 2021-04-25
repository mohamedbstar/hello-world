#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMD_MAX_LEN 4096
#define CMD_MAX_ARGS 10

enum parse_res{PARSE_SUCESS, PARSE_FAILURE, PARSE_UNKNOWN};
enum execute_res{EXECUTE_OVERFLOW, EXECUTE_FAILURE};
int num_operations = 1;
char* operations[] = {"mls"}; 

typedef struct cmd cmd_t;

struct cmd{
	char* buffer;
	int buffer_length;
	char* command;
	char* args;
};

void print_prompt(){
	print_cur_dir();
	printf("$ ");
}

void print_cur_dir(){
	char* path = (char*) malloc(sizeof(char) * 4096);
	if (!path)
	{
		perror("malloc");
		exit(1);
	}

	if(getcwd(path, 4096) != NULL){
		printf("%s",path);
	}else{
		printf("%s\n","Error");
		perror("getcwd");
	}
}

cmd_t* new_cmd(){
	cmd_t* cmd = (cmd_t*) malloc(sizeof(cmd_t));
	cmd->buffer = (char*) malloc(sizeof(char) * CMD_MAX_LEN);
	if (cmd->buffer == NULL)
	{
		perror("malloc");
		exit(1);
	}

	cmd->buffer_length = 0;
	cmd->command = (char*) malloc(sizeof(char) * 1024);
	if (cmd->command == NULL)
	{
		perror("malloc");
		exit(1);
	}

	cmd->args = (char*) malloc(sizeof(char) * 3072);
	if (cmd->args == NULL)
	{
		perror("malloc");
		exit(1);
	}

	return cmd;
}

int search_operations(char* operation){
	/*change it to find the operation in the current directory*/
	for (int i = 0; i < num_operations; ++i)
	{
		if (!strcmp(operations[i], operation))
		{
			return i;
		}
	}

	return -1;
}

enum parse_res parse_command(cmd_t* cmd){
	int status = 0;
	int counter = 0;
	int command_counter = 0;
	int args_counter = 0;
	if (!strcmp(cmd->buffer, " ") /*| !strcmp(cmd.buffer, ' ')*/ )
	{
		return PARSE_FAILURE;
	}
	while(counter < cmd->buffer_length){
		if (cmd->buffer[counter] == ' '&&printf("%s\t","&&" )&& status == 0)
		{
			status++;
		}else{
			if (status == 0)
			{
				/*append to command*/
				cmd->command[command_counter++] = cmd->buffer[counter];
			}else{
				/*append to args*/
				cmd->args[args_counter++] = cmd->buffer[counter];
			}
		}
		cmd->command[command_counter] = '\0';
		cmd->args[args_counter] = '\0';
		counter++;
	}
	printf("%s\n","before search_operations" );
	if (search_operations(cmd->command) == -1)
	{
		return PARSE_UNKNOWN;
	}
	return PARSE_SUCESS;
}

int parse_args(cmd_t cmd, char*** args_to_ret){
	char* cmd_args = cmd.args;
	int status = 0;
	int args_counter = 0;
	int sole_counters[CMD_MAX_ARGS];
	for (int i = 0; i < CMD_MAX_ARGS; ++i)
	{
		sole_counters[i] = 0;
	}

	while(args_counter < strlen(cmd_args)){
		if (cmd_args[args_counter] == ' ' )
		{
			if ( status < CMD_MAX_ARGS - 1)
			{
				status++;
			}else{
				if (strlen(cmd_args) == args_counter+1)
				{
					return status;
				}
				return -1;
			}
		}else{
			(&args_to_ret)[status][sole_counters[status]++] = cmd_args[args_counter]; 
		}
		args_counter++;
	}

	return status;
}

enum execute_res execute(cmd_t cmd){
	pid_t child_process = fork();
	if (child_process == -1)
	{
		perror(fork);
		exit(1);
	}else if(child_process > 0){
		/*parent_code*/
		/*do waiting stuff*/
		int status = 0;
		int wait_res = wait(&status);
		if (WIFEXITED(status))
		{
			printf("%s%d\n","child exited with status: ",WEXITSTATUS(status));
		}else if (WIFSIGNALED(status))
		{
			printf("%s%d\n","Terminated with signal: ",WTERMSIG(status));
		}
	}else{
		/*child code*/
		/*first: parse args to put program arguments*/
		char** args_to_ret = (char**) malloc(sizeof(char*) * CMD_MAX_ARGS);
		if (!args_to_ret)
		{
			perror("malloc");
			exit(1);
		}
		for (int i = 0; i < CMD_MAX_ARGS; ++i)
		{
			args_to_ret[i] = (char*) malloc(sizeof(char) * 307);
			if (!args_to_ret[i])
			{
				perror("malloc");
				exit(1);
			}
		}
		int parsed_args_res = parse_args(cmd, &args_to_ret);
		if (parsed_args_res == -1)
		{
			return EXECUTE_OVERFLOW;
		}
		char** execv_args = (char**) malloc(sizeof(char*) * parsed_args_res);
		if (!execv_args)
		{
			perror("malloc");
			exit(1);
		}

		for (int i = 0; i < parsed_args_res+2; ++i)
		{
			execv_args[i] = (char*) malloc(sizeof(char) * 307);
			if (i == 0)
			{
				strcpy(execv_args[i], cmd.command);
			}else if(i < parsed_args_res - 1){
				strcpy(execv_args[i], args_to_ret[i-1]);
			}else{
				execv_args[i] = NULL;
			}
		}
		/*second: do the exec*/
		int exec_res = execv(cmd.command, execv_args);
		if (exec_res == -1)
		{
			perror("execv");
			return EXECUTE_FAILURE;
		}
	}
}

int main(int argc, char const *argv[])
{
	cmd_t* cmd = new_cmd();
	enum parse_res result;
	enum execute_res exec_res;
	int num_read;
	int buff_size = 4069;
	while(1){
		print_prompt();
		num_read = getline(&cmd->buffer, &cmd->buffer_length,stdin);
		if (num_read < 0)
		{
			perror("read");
			exit(1);
		}
		cmd->buffer[num_read] = '\0';
		cmd->buffer_length = strlen(cmd->buffer)-1;
		printf("%s%d\n","strlen(buffer): ", (int)strlen(cmd->buffer) );
		printf("%s%d\n","buffer_length: ",cmd->buffer_length);
		printf("You Entered: %s\n",cmd->buffer );
		printf("Executing...\n");
		result = parse_command(cmd);
		printf("%s\n","after parse command");
		if (result == PARSE_FAILURE)
		{
			fprintf(stderr, "%s\n","Empty Command" );
		}else if(result == PARSE_UNKNOWN){
			fprintf(stderr, "%s\n","Unknown operation..." );
		}else{
			exec_res = execute(*cmd);
			if (exec_res == EXECUTE_OVERFLOW)
			{
				fprintf(stderr, "%s\n","Too many arguments" );
			}else if(exec_res == EXECUTE_FAILURE){
				fprintf(stderr, "%s\n","operation not complete!");
			}
		}
	}
	return 0;
}