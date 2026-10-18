#include "codexion.h"

static bool	is_coder_burnedout(t_sim *data, t_coder *coder)
{
	long	deadline;

	deadline = safe_read_long(&coder->deadline, &data->deadline_mtx);
	if (get_current_time_in_ms() >= deadline)
	{
		safe_write_bool(&data->one_burnout, true, &data->burnout_mtx);
		display_state_raw(data, coder->id, "burned out");
		return (true);
	}
	return (false);
}

static bool	all_coders_compiled(t_sim *data)
{
	long	done;

	done = safe_read_long(&data->done_count, &data->done_count_mtx);
	if (done == (long)data->number_of_coders)
	{
		safe_write_bool(&data->all_coders_compiled, true, &data->compiled_mtx);
		return (true);
	}
	return (false);
}

void	*monitor_coder(void *arg)
{
	t_sim	*data;
	size_t	i;

	data = (t_sim *)arg;
	while (safe_read_bool(&data->one_burnout, &data->burnout_mtx) == false
		&& safe_read_bool(&data->all_coders_compiled, &data->compiled_mtx)
		== false)
	{
		i = -1;
		while (++i < data->number_of_coders)
		{
			if (is_coder_burnedout(data, &data->coders[i]) == true)
				return (NULL);
		}
		if (all_coders_compiled(data) == true)
			return (NULL);
		usleep(500);
	}
	return (NULL);
}
