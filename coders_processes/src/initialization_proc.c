#include "codexion_processes.h"

void	init_coders(t_sim *data)
{
	t_coder	*coders;
	int		i;

	coders = malloc(sizeof(t_coder) * data->number_of_coders);
	if (!coders)
		error("Failed to allocate memory for coders\n", 0, 1, data);
	data->coders = coders;
	i = -1;
	while (++i < data->number_of_coders)
	{
		coders[i].id = i + 1;
		coders[i].data = data;
		coders[i].number_of_compiles = 0;
		coders[i].last_compile_start = 0;
		coders[i].deadline = 0;
		coders[i].full = false;
	}
}

void	init_data(char **argv, t_sim *data)
{
	memset(data, 0, sizeof(t_sim));
	data->number_of_coders = (int)atol(argv[1]);
	data->time_to_burnout = atol(argv[2]);
	data->time_to_compile = atol(argv[3]);
	data->time_to_debug = atol(argv[4]);
	data->time_to_refactor = atol(argv[5]);
	data->number_of_compiles_required = atol(argv[6]);
	data->dongle_cooldown = atol(argv[7]);
	data->scheduler = argv[8];
	data->sem_prefix = (long)getpid();
	init_shared(data);
	semaphores_init(data);
	init_coders(data);
}
