/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ttshivhu <marvin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/08 12:32:47 by ttshivhu          #+#    #+#             */
/*   Updated: 2018/07/09 15:51:04 by ttshivhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_H
# define MALLOC_H

# include <stdio.h>
# include <unistd.h>
# include <sys/mman.h>
# include <sys/time.h>
# include <sys/resource.h>

# define MAX_ALLOCS 100
# define MAX_EXTEND 10
# define TINY_PAGES_PER_BLOCK 1
# define SMALL_PAGES_PER_BLOCK 6

typedef struct		s_block
{
	size_t			size;
	struct s_block	*next;
	struct s_area	*area;
	int				free;
	char				data[1];
}					t_block;

typedef struct		s_area
{
	size_t			max_allocs;
	size_t			cur_free;
	size_t			max_block_size;
	void			*area;
	t_block			*blocks;
	struct s_area	*next;
}					t_area;

typedef struct		s_malloc
{
	t_area			*tiny;
	t_area			*small;
	t_area			*large;
}					t_malloc;

t_malloc				*g_m = NULL;


t_block	*get_block(t_area **area, size_t size);

extern void	*malloc(size_t size);
extern void	free(void *ptr);
#endif
