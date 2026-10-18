#include "codexion_processes.h"

long	get_current_time_in_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

bool	is_stopped(t_sim *data)
{
	bool	stopped;

	sem_wait(data->log_sem);
	stopped = data->shared->one_burnout;
	sem_post(data->log_sem);
	return (stopped);
}

void	display_state(t_sim *data, int id, const char *state)
{
	long	timestamp;

	sem_wait(data->log_sem);
	if (data->shared->one_burnout == false)
	{
		timestamp = get_current_time_in_ms() - data->shared->start_time;
		printf("%ld %d %s\n", timestamp, id, state);
	}
	sem_post(data->log_sem);
}

void	ft_usleep(long time_ms, t_sim *data)
{
	long	start;

	start = get_current_time_in_ms();
	while (get_current_time_in_ms() - start < time_ms)
	{
		if (is_stopped(data) == true)
			return ;
		usleep(500);
	}
}
