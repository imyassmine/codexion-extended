#include "codexion_processes.h"

static void	close_sem(sem_t *sem, t_sim *data, const char *kind, int idx)
{
	char	name[SEM_NAME_LEN];

	if (!sem)
		return ;
	make_sem_name(name, data, kind, idx);
	sem_close(sem);
	sem_unlink(name);
}

void	semaphores_clean(t_sim *data)
{
	int	i;

	i = -1;
	if (data->dongle_sems)
	{
		while (++i < data->number_of_coders)
			close_sem(data->dongle_sems[i], data, "mtx", i);
	}
	i = -1;
	if (data->grant_sems)
	{
		while (++i < data->number_of_coders)
			close_sem(data->grant_sems[i], data, "grant", i);
	}
	close_sem(data->log_sem, data, "log", 0);
	close_sem(data->fifo_sem, data, "fifo", 0);
	close_sem(data->start_sem, data, "start", 0);
	close_sem(data->deadline_sem, data, "deadline", 0);
}
