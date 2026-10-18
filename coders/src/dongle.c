#include "codexion.h"

static long	get_priority_key(t_sim *data, t_coder *coder)
{
	if (strcmp(data->scheduler, "fifo") == 0)
		return (safe_increment_long(&data->fifo_counter, &data->fifo_mtx));
	return (safe_read_long(&coder->deadline, &data->deadline_mtx));
}

static struct timespec	ms_to_timespec(long ms)
{
	struct timespec	ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	return (ts);
}

bool	dongle_acquire(t_sim *data, int i, t_coder *coder)
{
	struct timespec	ts;

	pthread_mutex_lock(&data->dongles[i].mtx);
	heap_push(&data->dongles[i].heap, get_priority_key(data, coder), coder);
	pthread_cond_broadcast(&data->dongles[i].cond);
	while (true)
	{
		if (safe_read_bool(&data->one_burnout, &data->burnout_mtx) == true
			|| safe_read_bool(&data->all_coders_compiled, &data->compiled_mtx)
			== true)
			return (pthread_mutex_unlock(&data->dongles[i].mtx), false);
		if (data->dongles[i].in_use == false
			&& data->dongles[i].heap.nodes[0].coder == coder
			&& get_current_time_in_ms() >= data->dongles[i].ready_time)
		{
			data->dongles[i].in_use = true;
			heap_pop(&data->dongles[i].heap);
			break ;
		}
		ts = ms_to_timespec(data->dongles[i].ready_time);
		pthread_cond_timedwait(&data->dongles[i].cond,
			&data->dongles[i].mtx, &ts);
	}
	pthread_mutex_unlock(&data->dongles[i].mtx);
	return (display_state(data, coder->id, "has taken a dongle"), true);
}

void	dongle_release(t_sim *data, int i)
{
	pthread_mutex_lock(&data->dongles[i].mtx);
	data->dongles[i].in_use = false;
	data->dongles[i].ready_time = get_current_time_in_ms()
		+ data->dongle_cooldown;
	pthread_cond_broadcast(&data->dongles[i].cond);
	pthread_mutex_unlock(&data->dongles[i].mtx);
}

void	get_dongle_indexes(t_coder *coder, size_t n, int *first, int *next)
{
	if (coder->id % 2)
	{
		*first = coder->id % (int)n;
		*next = coder->id - 1;
	}
	else
	{
		*first = coder->id - 1;
		*next = coder->id % (int)n;
	}
}
