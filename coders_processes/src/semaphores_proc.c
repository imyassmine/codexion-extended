#include "codexion_processes.h"

void	make_sem_name(char *name, t_sim *data, const char *kind, int idx)
{
	snprintf(name, SEM_NAME_LEN, "/codexion_b_%ld_%s_%d", data->sem_prefix,
		kind, idx);
}

static sem_t	*open_sem(t_sim *data, const char *kind, int idx,
		unsigned int value)
{
	char	name[SEM_NAME_LEN];
	sem_t	*sem;

	make_sem_name(name, data, kind, idx);
	sem_unlink(name);
	sem = sem_open(name, O_CREAT | O_EXCL, 0644, value);
	if (sem == SEM_FAILED)
		error("Failed to open a semaphore\n", 0, 1, data);
	return (sem);
}

static void	init_sem_arrays(t_sim *data)
{
	int	i;

	data->dongle_sems = calloc(data->number_of_coders, sizeof(sem_t *));
	data->grant_sems = calloc(data->number_of_coders, sizeof(sem_t *));
	if (!data->dongle_sems || !data->grant_sems)
		error("Failed to allocate semaphore arrays\n", 0, 1, data);
	i = -1;
	while (++i < data->number_of_coders)
	{
		data->dongle_sems[i] = open_sem(data, "mtx", i, 1);
		data->grant_sems[i] = open_sem(data, "grant", i, 0);
	}
}

void	semaphores_init(t_sim *data)
{
	data->log_sem = open_sem(data, "log", 0, 1);
	data->fifo_sem = open_sem(data, "fifo", 0, 1);
	data->start_sem = open_sem(data, "start", 0, 0);
	data->deadline_sem = open_sem(data, "deadline", 0, 1);
	init_sem_arrays(data);
}
