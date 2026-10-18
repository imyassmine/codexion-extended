#ifndef CODEXION_H
# define CODEXION_H

/* ================================ INCLUDES ================================ */

# include <pthread.h>
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <limits.h>
# include <stdbool.h>
# include <sys/time.h>

/* =============================== STRUCTURES =============================== */

typedef struct s_sim	t_sim;
typedef struct s_coder	t_coder;

/* --- Heap node --- */
typedef struct s_heap_node
{
	long	key;
	t_coder	*coder;
}	t_heap_node;

/* --- Heap --- */
typedef struct s_heap
{
	t_heap_node	*nodes;
	size_t		size;
	size_t		capacity;
}	t_heap;

/* --- Dongle --- */
typedef struct s_dongle
{
	pthread_mutex_t	mtx;
	pthread_cond_t	cond;
	bool			in_use;
	long			ready_time;
	t_heap			heap;
}	t_dongle;

/* --- Coder --- */
struct s_coder
{
	int			id;
	long		number_of_compiles;
	long		last_compile_start;
	long		deadline;
	pthread_t	tid;
	t_sim		*data;
};

/* --- Simulation data --- */
struct s_sim
{
	size_t			number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	long			number_of_compiles_required;
	long			dongle_cooldown;
	const char		*scheduler;
	pthread_t		monitor_tid;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_mutex_t	deadline_mtx;
	pthread_mutex_t	log_mtx;
	pthread_mutex_t	compiled_mtx;
	pthread_mutex_t	done_count_mtx;
	pthread_mutex_t	burnout_mtx;
	pthread_mutex_t	fifo_mtx;
	bool			all_coders_compiled;
	bool			one_burnout;
	long			done_count;
	long			fifo_counter;
	long			start_time;
};

/* ========================== FUNCTION PROTOTYPES =========================== */

/* --- Parsing --- */
void		parse_arguments(int argc, char **argv);
void		init_data(char **argv, t_sim *data);

/* --- Heap management --- */
void		heap_init(t_heap *h, size_t capacity, t_sim *data);
void		heap_push(t_heap *h, long key, t_coder *coder);
void		heap_pop(t_heap *h);

/* --- Coder thread --- */
bool		dongle_acquire(t_sim *data, int idx, t_coder *coder);
void		dongle_release(t_sim *data, int idx);
void		get_dongle_indexes(t_coder *coder, size_t number_of_coders,
				int *first, int *next);
void		display_state(t_sim *data, int id, const char *state);
void		display_state_raw(t_sim *data, int id, const char *state);
void		*coder_cycle(void *arg);

/* --- Monitor thread --- */
void		*monitor_coder(void *arg);

/* --- Thread management --- */
void		create_threads(t_sim *data);
void		wait_for_threads(t_sim *data);

/* --- Safe variable access --- */
bool		safe_read_bool(bool *var, pthread_mutex_t *mutex);
long		safe_read_long(long *var, pthread_mutex_t *mutex);
void		safe_write_bool(bool *var, bool value, pthread_mutex_t *mutex);
void		safe_write_long(long *var, long value, pthread_mutex_t *mutex);
long		safe_increment_long(long *var, pthread_mutex_t *mutex);

/* --- Utils --- */
long		get_current_time_in_ms(void);
void		ft_usleep(long time_ms, t_sim *data);
void		clean_up(t_sim *data);
void		error(const char *error_msg, long x, int status, t_sim *data);

#endif
