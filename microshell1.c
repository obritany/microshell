#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int			g_exit_code = 0;

int			ft_strlen(char *s)
{
	int		i = 0;
	while (s[i] != '\0')
		i++;
	return (i);
}

void		fd_connect(int	*prev, int *next)
{
	if (prev[0] != -1)
	{
		dup2(prev[0], 0);
		close(prev[0]);
		close(prev[1]);
	}
	if (next[1] != -1)
	{
		dup2(next[1], 1);
		close(next[1]);
		close(next[0]);
	}
}

int			ft_cd(char **cmd)
{
	int	ac = 0;
	int	r;
	while (cmd[ac])
		ac++;
	if (ac != 2)
	{
		write(2, "error: cd: bad argument\n", 25);
		return (1);
	}
	if ((r = chdir(cmd[1])) < 0)
	{
		write(2, "error: cd: cannot change directory to ", 38);
		write(2, cmd[1], ft_strlen(cmd[1]));
		write(2, "\n", 1);
		return (r);
	}
	return (r);
}

int			get_next_cmd(char ***cmd, char *end_tok, int start_pos, char **av)
{
	int		i = 0;
	av += start_pos;
	while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
		i++;
	*end_tok = av[i] ? *av[i] : '\0' ;
	av[i] = 0;
	*cmd = av;
	return (i);
}

int			main(int ac, char **av, char **env)
{
	(void)ac;
	char	**cmd;
	char	end_tok = ';';
	int		cmd_pos = 1;
	int		prev_pipe[2];
	int		next_pipe[2];
	int		res;
	pid_t	pid;

	next_pipe[0] = -1;
	next_pipe[1] = -1;
	while (end_tok != '\0' || (end_tok == ';' && av[cmd_pos] != 0))
	{
		cmd_pos = cmd_pos + get_next_cmd(&cmd, &end_tok, cmd_pos, av) + 1;
		if (!cmd[0])
			continue ;
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
		if (!(strcmp(cmd[0], "cd")))
			g_exit_code = ft_cd(cmd);
		else
		{
			pid = fork();
			if (pid == 0)
			{
				fd_connect(prev_pipe, next_pipe);
				res	= execve(cmd[0], cmd, env);
				write(2, "error: cd: cannot execute ", 22);
				write(2, cmd[0], ft_strlen(cmd[0]));
				write(2, "\n", 1);
				exit(res);
			}
			if (pid < 0)
			{
				write(2, "error: fatal\n", 13);
				exit(1);
			}
			waitpid(-1, &res, 0);
			if (WIFEXITED(res))
				g_exit_code = WEXITSTATUS(res);
		}
		close(prev_pipe[0]);
	}
	return (g_exit_code);
}