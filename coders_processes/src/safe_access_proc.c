#include "codexion_processes.h"

bool	safe_read_bool(bool *var, sem_t *sem)
{
	bool	read_var;

	sem_wait(sem);
	read_var = *var;
	sem_post(sem);
	return (read_var);
}

void	safe_write_bool(bool *var, bool value, sem_t *sem)
{
	sem_wait(sem);
	*var = value;
	sem_post(sem);
}

long	safe_read_long(long *var, sem_t *sem)
{
	long	read_var;

	sem_wait(sem);
	read_var = *var;
	sem_post(sem);
	return (read_var);
}

void	safe_write_long(long *var, long value, sem_t *sem)
{
	sem_wait(sem);
	*var = value;
	sem_post(sem);
}

long	increment_long(long *var, sem_t *sem)
{
	long	read_var;

	sem_wait(sem);
	*var += 1;
	read_var = *var;
	sem_post(sem);
	return (read_var);
}
