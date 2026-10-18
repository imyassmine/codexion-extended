#include "codexion_processes.h"

static struct timespec	ms_to_timespec(long ms)
{
	struct timespec	ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000L;
	return (ts);
}

void	wait_on_grant(t_sim *data, t_coder *coder, long until)
{
	struct timespec	ts;
	long			target;

	target = get_current_time_in_ms() + SEM_WAIT_STEP;
	if (until > 0 && until < target)
		target = until;
	ts = ms_to_timespec(target);
	sem_timedwait(data->grant_sems[coder->id - 1], &ts);
}

void	dongle_release(t_sim *data, int idx)
{
	t_dongle	*dongle;
	int			next;

	sem_wait(data->dongle_sems[idx]);
	dongle = &data->shared->dongles[idx];
	dongle->in_use = false;
	dongle->owner = -1;
	dongle->ready_time = get_current_time_in_ms() + data->dongle_cooldown;
	if (dongle->size > 0)
	{
		next = dongle->nodes[0].coder_id;
		heap_pop(dongle);
		dongle->in_use = true;
		dongle->owner = next;
		sem_post(data->grant_sems[next - 1]);
	}
	sem_post(data->dongle_sems[idx]);
}

void	get_dongle_indexes(t_coder *coder, int n, int *first, int *next)
{
	if (coder->id % 2)
	{
		*first = coder->id % n;
		*next = coder->id - 1;
	}
	else
	{
		*first = coder->id - 1;
		*next = coder->id % n;
	}
}
