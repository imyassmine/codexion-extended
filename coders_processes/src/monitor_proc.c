#include "codexion_processes.h"

static void	report_burnout(t_sim *data, t_coder *coder)
{
	bool	first;

	first = false;
	sem_wait(data->log_sem);
	if (data->shared->one_burnout == false)
	{
		data->shared->one_burnout = true;
		printf("%ld %d burned out\n",
			get_current_time_in_ms() - data->shared->start_time, coder->id);
		first = true;
	}
	sem_post(data->log_sem);
	if (first == true)
		kill_children(data);
	_exit(DEAD);
}

void	*monitor_coder(void *arg)
{
	t_coder	*coder;
	t_sim	*data;
	long	deadline;

	coder = (t_coder *)arg;
	data = coder->data;
	while (true)
	{
		if (safe_read_bool(&coder->full, data->deadline_sem) == true)
			return (NULL);
		deadline = safe_read_long(&coder->deadline, data->deadline_sem);
		if (get_current_time_in_ms() >= deadline)
		{
			if (safe_read_bool(&coder->full, data->deadline_sem) == true)
				return (NULL);
			report_burnout(data, coder);
		}
		usleep(500);
	}
	return (NULL);
}
