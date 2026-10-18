#include "codexion.h"

static void	coder_compiles(t_coder *coder, t_sim *data)
{
	int	first;
	int	next;

	get_dongle_indexes(coder, data->number_of_coders, &first, &next);
	if (dongle_acquire(data, first, coder) == false)
		return ;
	if (dongle_acquire(data, next, coder) == false)
	{
		dongle_release(data, first);
		return ;
	}
	coder->last_compile_start = get_current_time_in_ms();
	safe_write_long(&coder->deadline,
		coder->last_compile_start + data->time_to_burnout, &data->deadline_mtx);
	display_state(data, coder->id, "is compiling");
	ft_usleep(data->time_to_compile, data);
	dongle_release(data, first);
	dongle_release(data, next);
	coder->number_of_compiles++;
}

static void	coder_debugs(t_coder *coder, t_sim *data)
{
	display_state(data, coder->id, "is debugging");
	ft_usleep(data->time_to_debug, data);
}

static void	coder_refactors(t_coder *coder, t_sim *data)
{
	display_state(data, coder->id, "is refactoring");
	ft_usleep(data->time_to_refactor, data);
}

void	*coder_cycle(void *arg)
{
	t_coder	*coder;
	t_sim	*data;

	coder = (t_coder *)arg;
	data = coder->data;
	if (data->number_of_coders == 1)
	{
		dongle_acquire(data, 0, coder);
		while (safe_read_bool(&data->one_burnout, &data->burnout_mtx) == false
			&& safe_read_bool(&data->all_coders_compiled, &data->compiled_mtx)
			== false)
			usleep(500);
		return (dongle_release(data, 0), NULL);
	}
	while (safe_read_bool(&data->all_coders_compiled, &data->compiled_mtx)
		== false && safe_read_bool(&data->one_burnout, &data->burnout_mtx)
		== false)
	{
		coder_compiles(coder, data);
		if (coder->number_of_compiles == data->number_of_compiles_required)
			safe_increment_long(&data->done_count, &data->done_count_mtx);
		coder_debugs(coder, data);
		coder_refactors(coder, data);
	}
	return (NULL);
}
