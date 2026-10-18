#include "codexion.h"

long	get_current_time_in_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	display_state_raw(t_sim *data, int id, const char *state)
{
	pthread_mutex_lock(&data->log_mtx);
	printf("%ld %d %s\n", get_current_time_in_ms() - data->start_time,
		id, state);
	pthread_mutex_unlock(&data->log_mtx);
}

void	display_state(t_sim *data, int id, const char *state)
{
	if (safe_read_bool(&data->all_coders_compiled, &data->compiled_mtx) == true
		|| safe_read_bool(&data->one_burnout, &data->burnout_mtx) == true)
		return ;
	display_state_raw(data, id, state);
}

void	ft_usleep(long time_ms, t_sim *data)
{
	long	start;

	start = get_current_time_in_ms();
	while (get_current_time_in_ms() - start < time_ms)
	{
		if (safe_read_bool(&data->one_burnout, &data->burnout_mtx) == true
			|| safe_read_bool(&data->all_coders_compiled, &data->compiled_mtx)
			== true)
			return ;
		usleep(500);
	}
}
