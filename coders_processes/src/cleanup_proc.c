#include "codexion_processes.h"

void	clean_up(t_sim *data)
{
	semaphores_clean(data);
	if (data->shared && data->shared->dongles)
		munmap(data->shared->dongles, data->dongles_size);
	if (data->shared && data->shared->pids)
		munmap(data->shared->pids, data->pids_size);
	if (data->heap_nodes)
		munmap(data->heap_nodes, data->nodes_size);
	if (data->shared)
		munmap(data->shared, data->shared_size);
	free(data->coders);
	free(data->dongle_sems);
	free(data->grant_sems);
}

void	error(const char *error_msg, long x, int status, t_sim *data)
{
	if (x)
		fprintf(stderr, error_msg, x);
	else
		fprintf(stderr, "%s", error_msg);
	if (data)
		clean_up(data);
	exit(status);
}
