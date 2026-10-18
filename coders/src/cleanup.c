#include "codexion.h"

void	clean_up(t_sim *data)
{
	size_t	i;

	i = -1;
	while (data->dongles && ++i < data->number_of_coders)
	{
		pthread_mutex_destroy(&data->dongles[i].mtx);
		pthread_cond_destroy(&data->dongles[i].cond);
		free(data->dongles[i].heap.nodes);
	}
	free(data->dongles);
	free(data->coders);
	pthread_mutex_destroy(&data->deadline_mtx);
	pthread_mutex_destroy(&data->log_mtx);
	pthread_mutex_destroy(&data->compiled_mtx);
	pthread_mutex_destroy(&data->done_count_mtx);
	pthread_mutex_destroy(&data->burnout_mtx);
	pthread_mutex_destroy(&data->fifo_mtx);
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
