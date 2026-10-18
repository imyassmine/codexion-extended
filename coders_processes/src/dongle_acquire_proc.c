#include "codexion_processes.h"

static long	get_priority_key(t_sim *data, t_coder *coder)
{
	if (strcmp(data->scheduler, "fifo") == 0)
		return (increment_long(&data->shared->fifo_counter, data->fifo_sem));
	return (safe_read_long(&coder->deadline, data->deadline_sem));
}

static bool	try_take(t_sim *data, int idx, t_coder *coder)
{
	t_dongle	*dongle;
	long		now;

	dongle = &data->shared->dongles[idx];
	now = get_current_time_in_ms();
	if (dongle->in_use == true)
	{
		if (dongle->owner != coder->id || now < dongle->ready_time)
			return (false);
		return (true);
	}
	if (dongle->size == 0 || dongle->nodes[0].coder_id != coder->id)
		return (false);
	if (now < dongle->ready_time)
		return (false);
	dongle->in_use = true;
	dongle->owner = coder->id;
	heap_pop(dongle);
	return (true);
}

static long	get_wait_until(t_sim *data, int idx, t_coder *coder)
{
	t_dongle	*dongle;

	dongle = &data->shared->dongles[idx];
	if (dongle->in_use == true && dongle->owner == coder->id)
		return (dongle->ready_time);
	if (dongle->in_use == false && dongle->size > 0
		&& dongle->nodes[0].coder_id == coder->id)
		return (dongle->ready_time);
	return (0);
}

static t_action	dongle_step(t_sim *data, int idx, t_coder *coder,
		t_request *request)
{
	sem_wait(data->dongle_sems[idx]);
	if (is_stopped(data) == true)
		return (sem_post(data->dongle_sems[idx]), ACTION_STOPPED);
	if (request->enqueued == false)
	{
		heap_push(&data->shared->dongles[idx],
			get_priority_key(data, coder), coder->id);
		request->enqueued = true;
	}
	request->wait_until = get_wait_until(data, idx, coder);
	if (try_take(data, idx, coder) == true)
	{
		sem_post(data->dongle_sems[idx]);
		return (ACTION_TAKEN);
	}
	sem_post(data->dongle_sems[idx]);
	return (ACTION_WAIT);
}

bool	dongle_acquire(t_sim *data, int idx, t_coder *coder)
{
	t_request	request;
	t_action	action;

	request.enqueued = false;
	request.wait_until = 0;
	while (true)
	{
		action = dongle_step(data, idx, coder, &request);
		if (action == ACTION_TAKEN)
			return (display_state(data, coder->id,
					"has taken a dongle"), true);
		if (action == ACTION_STOPPED)
			return (false);
		wait_on_grant(data, coder, request.wait_until);
	}
}
