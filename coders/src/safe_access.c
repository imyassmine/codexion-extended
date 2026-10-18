#include "codexion.h"

void	safe_write_bool(bool *var, bool value, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	*var = value;
	pthread_mutex_unlock(mutex);
}

bool	safe_read_bool(bool *var, pthread_mutex_t *mutex)
{
	bool	read_var;

	pthread_mutex_lock(mutex);
	read_var = *var;
	pthread_mutex_unlock(mutex);
	return (read_var);
}

void	safe_write_long(long *var, long value, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	*var = value;
	pthread_mutex_unlock(mutex);
}

long	safe_read_long(long *var, pthread_mutex_t *mutex)
{
	long	read_var;

	pthread_mutex_lock(mutex);
	read_var = *var;
	pthread_mutex_unlock(mutex);
	return (read_var);
}

long	safe_increment_long(long *var, pthread_mutex_t *mutex)
{
	long	read_var;

	pthread_mutex_lock(mutex);
	*var += 1;
	read_var = *var;
	pthread_mutex_unlock(mutex);
	return (read_var);
}
