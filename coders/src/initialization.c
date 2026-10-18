#include "codexion.h"

static void	init_coders(t_coder **coders, t_sim *data)
{
	size_t	i;

	*coders = malloc(sizeof(t_coder) * data->number_of_coders);
	if (!*coders)
		error("Failed to allocate memory for coders\n", 0, 1, data);
	i = 0;
	while (i < data->number_of_coders)
	{
		(*coders)[i].id = (int)(i + 1);
		(*coders)[i].data = data;
		(*coders)[i].number_of_compiles = 0;
		(*coders)[i].last_compile_start = data->start_time;
		(*coders)[i].deadline = data->start_time + data->time_to_burnout;
		i++;
	}
}

static void	init_dongles(t_sim *data)
{
	size_t	i;

	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!data->dongles)
		error("Failed to allocate memory for dongles\n", 0, 1, data);
	i = -1;
	while (++i < data->number_of_coders)
	{
		if (pthread_mutex_init(&data->dongles[i].mtx, NULL)
			|| pthread_cond_init(&data->dongles[i].cond, NULL))
			error("Failed to init dongle %ld\n", (long)(i + 1), 1, data);
		data->dongles[i].in_use = false;
		data->dongles[i].ready_time = data->start_time;
		heap_init(&data->dongles[i].heap, data->number_of_coders, data);
	}
}

static void	init_mutexes(t_sim *data)
{
	pthread_mutex_t	*mutexes[6];
	size_t			i;

	mutexes[0] = &data->deadline_mtx;
	mutexes[1] = &data->log_mtx;
	mutexes[2] = &data->compiled_mtx;
	mutexes[3] = &data->done_count_mtx;
	mutexes[4] = &data->burnout_mtx;
	mutexes[5] = &data->fifo_mtx;
	i = -1;
	while (++i < 6)
	{
		if (pthread_mutex_init(mutexes[i], NULL))
			error("Failed to initialize a mutex\n", 0, 1, data);
	}
}

void	init_data(char **argv, t_sim *data)
{
	memset(data, 0, sizeof(t_sim));
	data->number_of_coders = (size_t)atol(argv[1]);
	data->time_to_burnout = atol(argv[2]);
	data->time_to_compile = atol(argv[3]);
	data->time_to_debug = atol(argv[4]);
	data->time_to_refactor = atol(argv[5]);
	data->number_of_compiles_required = atol(argv[6]);
	data->dongle_cooldown = atol(argv[7]);
	data->scheduler = argv[8];
	init_mutexes(data);
	data->start_time = get_current_time_in_ms();
	init_coders(&data->coders, data);
	init_dongles(data);
}
