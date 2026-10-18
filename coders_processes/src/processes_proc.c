#include "codexion_processes.h"

static void	coder_process(t_sim *data, t_coder *coder)
{
	t_status	status;

	sem_wait(data->start_sem);
	coder->last_compile_start = get_current_time_in_ms();
	safe_write_long(&coder->deadline, coder->last_compile_start
		+ data->time_to_burnout, data->deadline_sem);
	if (pthread_create(&coder->monitor_tid, NULL, monitor_coder, coder) != 0)
		_exit(DEAD);
	status = coder_lifecycle(coder, data);
	safe_write_bool(&coder->full, true, data->deadline_sem);
	pthread_join(coder->monitor_tid, NULL);
	_exit(status);
}

static void	start_processes(t_sim *data)
{
	int	i;

	data->shared->start_time = get_current_time_in_ms();
	i = -1;
	while (++i < data->number_of_coders)
		sem_post(data->start_sem);
}

void	create_processes(t_sim *data)
{
	pid_t	pid;
	int		i;

	i = -1;
	while (++i < data->number_of_coders)
	{
		pid = fork();
		if (pid == -1)
		{
			kill_children(data);
			error("Failed to fork a coder process\n", 0, 1, data);
		}
		if (pid == 0)
			coder_process(data, &data->coders[i]);
		data->shared->pids[i] = pid;
	}
	start_processes(data);
}

void	wait_for_children(t_sim *data)
{
	int	status;
	int	i;

	i = -1;
	while (++i < data->number_of_coders)
	{
		if (waitpid(data->shared->pids[i], &status, 0) == -1)
			continue ;
		data->shared->pids[i] = 0;
	}
}

void	kill_children(t_sim *data)
{
	pid_t	self;
	int		i;

	self = getpid();
	i = -1;
	while (++i < data->number_of_coders)
	{
		if (data->shared->pids[i] > 0 && data->shared->pids[i] != self)
			kill(data->shared->pids[i], SIGKILL);
	}
}
