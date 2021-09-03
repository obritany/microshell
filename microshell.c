#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

int ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

void ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

void connect(int *prev_pipe, int *next_pipe)
{
	if (prev_pipe[0] != -1)
	{
		dup2(prev_pipe[0], 0);
		close(prev_pipe[0]);
		close(prev_pipe[1]);
	}
	if (next_pipe[1] != -1)
	{
		dup2(next_pipe[1], 1);
		close(next_pipe[1]);
		close(next_pipe[0]);
	}
}

int cd(char **cmd)
{
	int argc = 0;

	while (cmd[argc])
		argc++;

	if (argc != 2)
	{
		ft_putstr_fd("error: cd: bad argument\n", 2);
		return (1);
	}

	if (chdir(cmd[1]))
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(cmd[1], 2);
		ft_putstr_fd("\n", 2);
		return (-1);
	}

	return (0);
}

int get_next_cmd(char *argv[], int arg_pos, char *end_tok, char **cmd[])
{
	int i = 0;

	argv += arg_pos;
	while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
		i++;
	*end_tok = argv[i] ? *argv[i] : '\0';
	argv[i] = 0;
	*cmd = argv;
	return (i);
}

int main(int argc, char *argv[], char *env[])
{
	(void)argc;
	int rslt = 0;
	int prev_pipe[2];
	int next_pipe[2];
	char **cmd;
	int arg_pos = 1;
	char end_tok = ';';
	pid_t pid;

	next_pipe[0] = -1;
	next_pipe[1] = -1;

	while (end_tok != '\0')
	{
		arg_pos += get_next_cmd(argv, arg_pos, &end_tok, &cmd) + 1;
		if (!cmd[0])
			continue;

		prev_pipe[0] = next_pipe[0];
		prev_pipe[1] = next_pipe[1];
		if (end_tok == '|')
			pipe(next_pipe);
		else
		{
			next_pipe[0] = -1;
			next_pipe[1] = -1;
		}
		close(prev_pipe[1]);

		if (!strcmp(cmd[0], "cd"))
			rslt = cd(cmd);
		else
		{
			pid = fork();
			if (pid < 0)
			{
				ft_putstr_fd("error: fatal\n", 2);
				return (1);
			}
			if (pid == 0)
			{
				connect(prev_pipe, next_pipe);
				rslt = execve(cmd[0], cmd, env);
				ft_putstr_fd("error: cannot execute ", 2);
				ft_putstr_fd(cmd[0], 2);
				ft_putstr_fd("\n", 2);
				exit(rslt);
			}
			waitpid(-1, &rslt, 0);
			if (WIFEXITED(rslt))
				rslt = WEXITSTATUS(rslt);
		}
		close(prev_pipe[0]);
	}
	return (rslt);
}