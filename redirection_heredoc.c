/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_heredoc.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: taejkim <taejkim@student.42seoul.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/17 11:51:31 by kanlee            #+#    #+#             */
/*   Updated: 2021/12/26 06:41:22 by kanlee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/readline.h>
#include "minishell.h"
#include "libft/libft.h"

static void	sigint_in_heredoc_handler(int sig)
{
	if (sig == SIGINT)
		close(STDIN_FILENO);
}

/* We're in second readline.
 * When user press ctrl-c, we should stop readline and return to prompt.
 * To do this, replace signal handler temporarily and restore it when
 *   heredoc input is completed.
 * New sighandler will close STDIN to break current readline.
 * if input == NULL && STDIN is not terminal, it means we met SIGINT.
 * in this case, restore STDIN and return 1.
 * In general situation, write input to tmpfile and return 0.
*/
static int	write_to_tmpfile(int fd, char *limit)
{
	char	*input;
	int		stdin_bak;

	stdin_bak = dup(STDIN_FILENO);
	signal(SIGINT, sigint_in_heredoc_handler);
	while (1)
	{
		input = readline(">");
		if (input == NULL || ft_strequ(input, limit))
		{
			if (input)
				free(input);
			break ;
		}
		ft_putendl_fd(input, fd);
		free(input);
	}
	signal(SIGINT, sig_handler);
	if (isatty(STDIN_FILENO))
		return (0);
	dup2(stdin_bak, STDIN_FILENO);
	ft_putendl_fd("", STDOUT_FILENO);
	return (1);
}

static int	open_available(char **tmpfilename)
{
	int		fd;
	int		i;
	char	*idxstr;

	i = 0;
	while (i < 65535)
	{
		idxstr = ft_itoa(i);
		*tmpfilename = ft_strjoin("/tmp/tmp_for_heredoc_", idxstr);
		free(idxstr);
		fd = open(*tmpfilename, O_WRONLY | O_CREAT | O_EXCL, 0644);
		if (fd > 0)
			return (fd);
		free(*tmpfilename);
		i++;
	}
	return (-1);
}

/* open tmp file for write
 *   print heredoc prompt
 *   read input from stdin, write to tmpfile
 * while input==limitstr
 * print warning when reached EOF before Limitstr found.
 * close tmpfile
 * open it again read-only
 * unlink() --- tmpfile will be deleted after it's closed
 * change RD_TARGET content to readable fd.
 * return 1 if open failed or interrupted.
*/
int	read_heredoc(t_cmd *node, char *limit)
{
	int		fd;
	char	*tmpfilename;
	int		ret;

	fd = open_available(&tmpfilename);
	if (fd < 0)
	{
		printf("Fatal: can't create tmpfile for heredoc\n");
		return (1);
	}
	ret = write_to_tmpfile(fd, limit);
	close(fd);
	if (ret != 0)
	{
		unlink(tmpfilename);
		free(tmpfilename);
		return (ret);
	}
	fd = open(tmpfilename, O_RDONLY);
	unlink(tmpfilename);
	free(tmpfilename);
	free(node->next->token);
	node->next->token = ft_itoa(fd);
	return (0);
}
