#include "codexion_processes.h"

static bool	node_is_smaller(t_heap_node *a, t_heap_node *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->coder_id < b->coder_id);
}

static size_t	smallest_child(t_dongle *dongle, size_t i)
{
	size_t	left;
	size_t	right;
	size_t	smallest;

	left = i * 2 + 1;
	right = i * 2 + 2;
	smallest = i;
	if (left < dongle->size && node_is_smaller(&dongle->nodes[left],
			&dongle->nodes[smallest]) == true)
		smallest = left;
	if (right < dongle->size && node_is_smaller(&dongle->nodes[right],
			&dongle->nodes[smallest]) == true)
		smallest = right;
	return (smallest);
}

void	heap_push(t_dongle *dongle, long key, int coder_id)
{
	size_t		i;
	size_t		parent;
	t_heap_node	tmp;

	dongle->nodes[dongle->size].key = key;
	dongle->nodes[dongle->size].coder_id = coder_id;
	i = dongle->size;
	dongle->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (node_is_smaller(&dongle->nodes[i],
				&dongle->nodes[parent]) == false)
			break ;
		tmp = dongle->nodes[parent];
		dongle->nodes[parent] = dongle->nodes[i];
		dongle->nodes[i] = tmp;
		i = parent;
	}
}

void	heap_pop(t_dongle *dongle)
{
	size_t		i;
	size_t		smallest;
	t_heap_node	tmp;

	if (dongle->size == 0)
		return ;
	dongle->size--;
	dongle->nodes[0] = dongle->nodes[dongle->size];
	i = 0;
	while (true)
	{
		smallest = smallest_child(dongle, i);
		if (smallest == i)
			break ;
		tmp = dongle->nodes[i];
		dongle->nodes[i] = dongle->nodes[smallest];
		dongle->nodes[smallest] = tmp;
		i = smallest;
	}
}
