#include "codexion.h"

void	heap_init(t_heap *h, size_t capacity, t_sim *data)
{
	h->nodes = malloc(sizeof(t_heap_node) * capacity);
	if (!h->nodes)
		error("Failed to allocate memory for a dongle heap\n", 0, 1, data);
	h->size = 0;
	h->capacity = capacity;
}

static bool	node_is_smaller(t_heap_node *a, t_heap_node *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->coder->id < b->coder->id);
}

static size_t	smallest_child(t_heap *h, size_t i)
{
	size_t	left;
	size_t	right;
	size_t	smallest;

	left = i * 2 + 1;
	right = i * 2 + 2;
	smallest = i;
	if (left < h->size && node_is_smaller(&h->nodes[left],
			&h->nodes[smallest]) == true)
		smallest = left;
	if (right < h->size && node_is_smaller(&h->nodes[right],
			&h->nodes[smallest]) == true)
		smallest = right;
	return (smallest);
}

void	heap_push(t_heap *h, long key, t_coder *coder)
{
	size_t		i;
	size_t		parent;
	t_heap_node	tmp;

	h->nodes[h->size].key = key;
	h->nodes[h->size].coder = coder;
	i = h->size;
	h->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (node_is_smaller(&h->nodes[i], &h->nodes[parent]) == false)
			break ;
		tmp = h->nodes[parent];
		h->nodes[parent] = h->nodes[i];
		h->nodes[i] = tmp;
		i = parent;
	}
}

void	heap_pop(t_heap *h)
{
	size_t		i;
	size_t		smallest;
	t_heap_node	tmp;

	if (h->size == 0)
		return ;
	h->size--;
	h->nodes[0] = h->nodes[h->size];
	i = 0;
	while (true)
	{
		smallest = smallest_child(h, i);
		if (smallest == i)
			break ;
		tmp = h->nodes[i];
		h->nodes[i] = h->nodes[smallest];
		h->nodes[smallest] = tmp;
		i = smallest;
	}
}
