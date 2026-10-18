#include "codexion_processes.h"

static void	coder_starts_compiling(t_coder *coder, t_sim *data)
{
	long	now;

	now = get_current_time_in_ms();
	coder->last_compile_start = now;
	safe_write_long(&coder->deadline, now + data->time_to_burnout,
		data->deadline_sem);
	display_state(data, coder->id, "is compiling");
}

static bool	coder_compiles(t_coder *coder, t_sim *data)
{
	int	first;
	int	next;

	get_dongle_indexes(coder, data->number_of_coders, &first, &next);
	if (dongle_acquire(data, first, coder) == false)
		return (false);
	if (dongle_acquire(data, next, coder) == false)
	{
		dongle_release(data, first);
		return (false);
	}
	coder_starts_compiling(coder, data);
	ft_usleep(data->time_to_compile, data);
	dongle_release(data, first);
	dongle_release(data, next);
	coder->number_of_compiles++;
	return (true);
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

t_status	coder_lifecycle(t_coder *coder, t_sim *data)
{
	if (data->number_of_coders == 1)
	{
		dongle_acquire(data, 0, coder);
		while (true)
			usleep(1000);
	}
	while (true)
	{
		if (coder_compiles(coder, data) == false)
			return (DEAD);
		if (coder->number_of_compiles == data->number_of_compiles_required)
			return (FULL);
		coder_debugs(coder, data);
		coder_refactors(coder, data);
	}
	return (DEAD);
}
