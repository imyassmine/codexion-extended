#include "codexion.h"

void	create_threads(t_sim *data)
{
	size_t	i;

	i = -1;
	while (++i < data->number_of_coders)
	{
		if (pthread_create(&data->coders[i].tid, NULL,
				coder_cycle, &data->coders[i]))
			error("Failed to create coder %ld's thread\n",
				(long)(i + 1), 1, data);
	}
	if (pthread_create(&data->monitor_tid, NULL, monitor_coder, data))
		error("Failed to create monitor thread\n", 0, 1, data);
}

void	wait_for_threads(t_sim *data)
{
	size_t	i;

	i = -1;
	while (++i < data->number_of_coders)
	{
		if (pthread_join(data->coders[i].tid, NULL))
			error("Failed to join coder %ld's thread\n",
				(long)(i + 1), 1, data);
	}
	if (pthread_join(data->monitor_tid, NULL))
		error("Failed to join monitor thread\n", 0, 1, data);
}
