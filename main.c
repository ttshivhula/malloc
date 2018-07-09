#include "malloc.h"

static void	*ft_mmap(size_t size)
{
	void	*ptr;

	ptr = mmap(0, size, PROT_READ | PROT_WRITE,
		   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return (ptr);
}

static void	create_blocks(t_area **b, int space_size)
{
	t_block	*cur;
	t_block	*next;
	t_area	*area;
	size_t	i;

	i = 0;
	area = *b;
	cur = (t_block*)area->area;
	cur->size = (size_t)space_size;
	cur->next = NULL;
	cur->area = area;
	cur->free = 1;
	area->blocks = cur;
	while (++i <= area->max_allocs)
	{
		next = (t_block *)((char *)cur->data + (size_t)space_size);
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

	s = area->max_block_size * area->max_allocs;
	area->area = ft_mmap(s);
	create_blocks(&area, space_size);
}

static void	create_small_init_large(void)
{
	size_t pagesz;

	pagesz = getpagesize();
	g_m->small = (t_area *)ft_mmap(sizeof(t_area));
	if (g_m->small)
	{
		g_m->small->max_allocs = MAX_ALLOCS;
		g_m->small->cur_free = MAX_ALLOCS;
		g_m->small->max_block_size = SMALL_PAGES_PER_BLOCK * pagesz;
		g_m->tiny->next = NULL;
		area_gen(g_m->small, g_m->small->max_block_size - sizeof(t_block));
	}
	g_m->large = (t_area *)ft_mmap(sizeof(t_area));
	g_m->large->blocks = NULL;
}

void	create_areas(void)
{
	size_t pagesz;

	pagesz = getpagesize();
	g_m->tiny = (t_area *)ft_mmap(sizeof(t_area));
	if (g_m->tiny)
	{
		g_m->tiny->max_allocs = MAX_ALLOCS;
		g_m->tiny->cur_free = MAX_ALLOCS;
		g_m->tiny->max_block_size = TINY_PAGES_PER_BLOCK * pagesz;
		g_m->tiny->next = NULL;
		area_gen(g_m->tiny, g_m->tiny->max_block_size - sizeof(t_block));
	}
	create_small_init_large();
}

t_block	*extend_area(t_area **cur)
{
	t_area	*area;
	t_area	*next;

	area = *cur;
	next = (t_area *)ft_mmap(sizeof(t_area));
	area->next = next;
	if (area->next)
	{
		area->next->max_allocs = MAX_ALLOCS;
		area->next->cur_free = MAX_ALLOCS;
		area->next->max_block_size = area->max_block_size;
		area->next->next = NULL;
		area_gen(area->next, area->next->max_block_size - sizeof(t_block));
		return (area->next->blocks);
	}
	return (NULL);
}

t_block	*get_block(t_area **area, size_t size)
{
	t_area	*current_arrea;
	t_block	*current_block;
	int	pagesz;

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
	return (extend_area(area));
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
		if (size <= (g_m->tiny->max_block_size - sizeof(t_block)))
			ptr = use_block(size, get_block(&g_m->tiny, size));
		else if (size <= (g_m->small->max_block_size - sizeof(t_block)))
			ptr = use_block(size, get_block(&g_m->small, size));
		if (ptr)
			return (ptr);
	}
	return (NULL);
}

void	fix_fragmentation(t_block *block, int type)
{
	size_t	max_size;
	t_block	*tmp;

	max_size = (type) ? (g_m->small->max_block_size - sizeof(t_block)) :
			     (g_m->tiny->max_block_size - sizeof(t_block));
	if (block->next && (block->size + block->next->size < max_size) &&
	    block->next->free)
	{
		tmp = block->next->next;
		block->size += block->next->size + sizeof(t_block);
		block->next = tmp;
		if (type)
			g_m->small->cur_free--;
		else
			g_m->tiny->cur_free--;
	}
}

void	ft_free(void *ptr)
{
	t_block *b;
	size_t	size;

	if (ptr)
	{
		b = ptr - sizeof(t_block);
		size = b->size;
		b->free = 1;
		b->area->cur_free++;
		(size < (g_m->tiny->max_block_size - sizeof(t_block))) ?
			fix_fragmentation(b, 0) : fix_fragmentation(b, 1);
		if (size >= g_m->small->max_block_size - (sizeof(t_block)))
		{
			//ummap big block
		}
		ptr = NULL;
	}
}

void	print_blocks(t_block *b)
{
	int i = 1;
	int	free = (b) ? b->area->cur_free : 0;
	int	free_bytes = 0;
	printf("-- CREATED BLOCKS --\n");
	while (b)
	{
		printf("%d.  size: %d free: %d addr: %p\n", i, b->size, b->free, b);
		if (b->free)
			free_bytes += b->size;
		i++;
		b = b->next;
	}
	printf("free blocks: %d total bytes: %d\n", free, free_bytes);
}

int	main(void)
{
	char	*s = ft_malloc(0);
	int	i;
	char	*ptr;

	while (++i < 10)
	{
		ptr = ft_malloc(4000);
		ft_free(ptr);
	}
	//ft_malloc(3000);
	//ft_malloc(500);
	//ft_malloc(24500);
	print_blocks(g_m->tiny->blocks);
	//print_blocks(g_m->small->blocks);
	//print_blocks(g_m->large->blocks);
	return (0);
}
