#include "codexion_processes.h"

static void	*map_shared(size_t size, t_sim *data)
{
	void	*ptr;

	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		error("Failed to allocate shared memory\n", 0, 1, data);
	return (ptr);
}

static void	check_shared_sizes(t_sim *data, size_t n)
{
	if (n > SIZE_MAX / n || n * n > SIZE_MAX / sizeof(t_heap_node))
		error("Too many coders for the shared memory\n", 0, 1, data);
}

static void	init_dongles(t_sim *data)
{
	t_dongle	*dongles;
	size_t		n;
	int			i;

	dongles = data->shared->dongles;
	n = (size_t)data->number_of_coders;
	i = -1;
	while (++i < data->number_of_coders)
	{
		dongles[i].in_use = false;
		dongles[i].owner = -1;
		dongles[i].size = 0;
		dongles[i].capacity = n;
		dongles[i].nodes = data->heap_nodes + (size_t)i * n;
	}
}

void	init_shared(t_sim *data)
{
	size_t	n;

	n = (size_t)data->number_of_coders;
	check_shared_sizes(data, n);
	data->shared_size = sizeof(t_shared);
	data->dongles_size = sizeof(t_dongle) * n;
	data->nodes_size = sizeof(t_heap_node) * n * n;
	data->pids_size = sizeof(pid_t) * n;
	data->shared = map_shared(data->shared_size, data);
	data->heap_nodes = map_shared(data->nodes_size, data);
	data->shared->dongles = map_shared(data->dongles_size, data);
	data->shared->pids = map_shared(data->pids_size, data);
	init_dongles(data);
}
