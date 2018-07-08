#include "malloc.h"

static void	*ft_mmap(size_t size)
{
	void	*ptr;

	ptr = mmap(0, size, PROT_READ | PROT_WRITE,
		   MAP_ANON | MAP_PRIVATE, -1, 0);
	return (ptr);
}

static void	create_blocks(t_area **b, int space_size, size_t i)
{
	t_block	*cur;
	t_block	*next;
	t_area	*area;

	area = *b;
	cur = (t_block*)area->area;
	cur->size = (size_t)space_size;
	cur->next = NULL;
	cur->area = area;
	cur->free = 1;
	area->blocks = cur;
	while (++i < area->max_allocs)
	{
		if (i != area->max_allocs)
			next = (t_block*)((char*)cur + (size_t)space_size + sizeof(t_block));
		next->size = (size_t)space_size;
		next->next = NULL;
		cur->next = next;
		cur->area = area;
		cur->free = 1;
		cur = cur->next;
	}
}

static void	area_gen(t_area *area, int space_size)
{
	size_t	s;
	size_t	i;

	i = 0;
	s = area->max_block_size * area->max_allocs;
	area->area = ft_mmap(s);
	create_blocks(&area, space_size, i);
}

void	create_areas(void)
{
	size_t pagesz;

	pagesz = getpagesize();
	g_m->tiny = (t_area*)ft_mmap(sizeof(t_area));
	if (g_m->tiny)
	{
		g_m->tiny->max_allocs = (size_t)ZONE_MAX_ALLOCS;
		g_m->tiny->cur_free = (size_t)ZONE_MAX_ALLOCS;
		g_m->tiny->max_block_size = (size_t)TINY_PAGES_PER_BLOCK * pagesz;
		g_m->tiny->next = NULL;
		area_gen(g_m->tiny, g_m->tiny->max_block_size - sizeof(t_block));
	}
}

t_block	*get_block(t_area **area, size_t size)
{
	t_area	*current_arrea;
	t_block	*current_block;

	current_arrea = *area;
	while (current_arrea)
	{
		if (current_arrea->cur_free > 0)
		{
			current_block = current_arrea->blocks;
			while (current_block)
			{
				if (size <= current_block->size && current_block->free == 1)
					return (current_block);
				current_block = current_block->next;
			}
		}
		current_arrea = current_arrea->next;
	}
	return (NULL);
}

void	*use_block(size_t size, t_block *block)
{
	t_block	*temp;
	int		leftover;
	void	*ptr;

	if (block)
	{
		leftover = (int)block->size - (int)size;
		block->free = 0;
		ptr = (void*)block + sizeof(t_block);
		if (((leftover - (int)sizeof(t_block)) > 0))
		{
			block->area->max_allocs++;
			block->size = size;
			temp = block->next;
			block->next = (t_block*)(block->data + size);
			block->next->size = leftover - sizeof(t_block);
			block->next->next = temp;
			block->next->area = block->area;
			block->next->free = 1;
		}
		else
			block->area->cur_free--;
		return (ptr);
	}
	return (NULL);
}

void	*ft_malloc(size_t size)
{
	void			*ptr;

	if (!(g_m))
	{
		g_m = (t_malloc *)ft_mmap(sizeof(t_malloc));
		create_areas();
	}
	if (g_m->tiny->blocks && size > 0)
	{
		if (size <= (g_m->tiny->max_block_size - sizeof(t_area)))
			ptr = use_block(size, get_block(&g_m->tiny, size));
		if (ptr)
			return (ptr);
	}
	return (NULL);
}

void	print_blocks(t_block *b)
{
	int i = 1;
	printf("Created blocks\n");
	while (b)
	{
		printf("%d.  size: %d free: %d addr: %p\n", i, b->size, b->free, b);
		i++;
		b = b->next;
	}
}

int	main(void)
{
	char	*s = ft_malloc(1000);
	int	i;

	i = 0;
	printf("%p\n", &s);
//	while(i < 234)
//	{
		//s[i] = 'a';
		char *tmp = ft_malloc(1000);
		tmp = "test";
		char *tmp2 = ft_malloc(1000);
		tmp2 = "test2";
		printf("%s : %s", tmp2, tmp);
//		i++;
//	}
	//s[i] ='\0';
	printf("%s\n", s);
	printf("%p\n", &s);
	i = -1;
	printf("block : %d\n", sizeof(t_block));
	while (++i < 250)
		ft_malloc(3250);
	char	*p = ft_malloc(4);
	if(p)
		printf("Allocated\n");
	else
		printf("Not allocated\n");
	print_blocks(g_m->tiny->blocks);
	return (0);
}
