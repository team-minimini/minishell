/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: taejkim <taejkim@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/11 16:59:43 by kanlee            #+#    #+#             */
/*   Updated: 2021/12/26 08:35:42 by kanlee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "minishell.h"
#include "libft/libft.h"

int		g_exit_status = 0;

#ifndef DEBUG

void	sig_handler(int signo)
{
	int	wstatus;

	if (signo == SIGINT)
	{
		if (waitpid(-1, &wstatus, WNOHANG) == -1 && errno == ECHILD)
		{
			g_exit_status = 1;
			printf("\n");
			rl_on_new_line();
			rl_replace_line("", 0);
			rl_redisplay();
		}	
	}
}

int	get_line(char **line)
{
	char		*line_trimmed;
	extern int	rl_catch_signals;

	rl_catch_signals = 0;
	if (*line)
	{
		free(*line);
		*line = NULL;
	}
	*line = readline("minimini-shell $ ");
	if (*line == NULL)
	{
		ft_putendl_fd("exit", STDERR_FILENO);
		exit(g_exit_status);
	}
	line_trimmed = ft_strtrim(*line, " ");
	free(*line);
	*line = line_trimmed;
	if (**line == '\0')
		return (-1);
	add_history(*line);
	return (0);
}

#endif

static void	inc_shlvl(t_env *env)
{
	int		oldlvl;
	char	*newlvl;

	oldlvl = ft_atoi(get_value(env, "SHLVL"));
	newlvl = ft_itoa(oldlvl + 1);
	modify_env("SHLVL", newlvl, 1, env);
	free(newlvl);
}

void	init(int ac, char **av, t_env *env)
{
	(void)ac;
	(void)av;
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	inc_shlvl(env);
}

int	main(int ac, char **av, char **envp)
{
	t_env	*env;
	char	*line;
	t_cmd	*cmd;
	int		err_flag;

	env = make_env(envp);
	init(ac, av, env);
	line = 0;
	cmd = 0;
	while (1)
	{
		if (get_line(&line) < 0)
			continue ;
		err_flag = 0;
		parse(&cmd, line, &err_flag);
		if (!err_flag)
			err_flag = check_cmd(cmd);
		err_print(err_flag);
		if (err_flag)
			continue ;
		parse_env(cmd, env);
		g_exit_status = exec_line(cmd, &env, g_exit_status);
	}
	return (0);
}
