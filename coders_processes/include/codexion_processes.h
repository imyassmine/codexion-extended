#ifndef CODEXION_PROCESSES_H
# define CODEXION_PROCESSES_H

/* ================================ INCLUDES ================================ */

# include <pthread.h>
# include <semaphore.h>
# include <fcntl.h>
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <limits.h>
# include <stdbool.h>
# include <stdint.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <sys/mman.h>
# include <signal.h>
# include <time.h>

/* ================================ DEFINES ================================= */

# define SEM_NAME_LEN 64
# define SEM_WAIT_STEP 100

/* =============================== STRUCTURES =============================== */

typedef struct s_sim	t_sim;
typedef struct s_coder	t_coder;

typedef enum e_status
{
	DEAD = 1,
	FULL = 2
}	t_status;

typedef enum e_action
{
	ACTION_WAIT = 0,
	ACTION_TAKEN = 1,
	ACTION_STOPPED = 2
}	t_action;

typedef struct s_request
{
	bool	enqueued;
	long	wait_until;
}	t_request;

/* --- Heap node (lives in shared memory) --- */
typedef struct s_heap_node
{
	long	key;
	int		coder_id;
}	t_heap_node;

/* --- Dongle (the state lives in shared memory) --- */
typedef struct s_dongle
{
	bool		in_use;
	int			owner;
	long		ready_time;
	size_t		size;
	size_t		capacity;
	t_heap_node	*nodes;
}	t_dongle;

/* --- Cross-process data (lives in shared memory) --- */
typedef struct s_shared
{
	long		start_time;
	long		fifo_counter;
	bool		one_burnout;
	t_dongle	*dongles;
	pid_t		*pids;
}	t_shared;

/* --- Coder (one copy per child process) --- */
struct s_coder
{
	int			id;
	long		number_of_compiles;
	long		last_compile_start;
	long		deadline;
	bool		full;
	pthread_t	monitor_tid;
	t_sim		*data;
};

/* --- Simulation data (one copy per process) --- */
struct s_sim
{
	int			number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	long		number_of_compiles_required;
	long		dongle_cooldown;
	const char	*scheduler;
	long		sem_prefix;
	t_shared	*shared;
	t_heap_node	*heap_nodes;
	t_coder		*coders;
	sem_t		**dongle_sems;
	sem_t		**grant_sems;
	sem_t		*log_sem;
	sem_t		*fifo_sem;
	sem_t		*start_sem;
	sem_t		*deadline_sem;
	size_t		shared_size;
	size_t		dongles_size;
	size_t		nodes_size;
	size_t		pids_size;
};

/* ========================== FUNCTION PROTOTYPES =========================== */

/* --- Parsing --- */
void		parse_arguments(int argc, char **argv);

/* --- Initialization --- */
void		init_data(char **argv, t_sim *data);
void		init_coders(t_sim *data);
void		init_shared(t_sim *data);
void		semaphores_init(t_sim *data);
void		semaphores_clean(t_sim *data);
void		make_sem_name(char *name, t_sim *data, const char *kind, int idx);

/* --- Heap management --- */
void		heap_push(t_dongle *dongle, long key, int coder_id);
void		heap_pop(t_dongle *dongle);

/* --- Dongle management --- */
bool		dongle_acquire(t_sim *data, int idx, t_coder *coder);
void		dongle_release(t_sim *data, int idx);
void		get_dongle_indexes(t_coder *coder, int n, int *first, int *next);
void		wait_on_grant(t_sim *data, t_coder *coder, long until);

/* --- Coder life cycle --- */
t_status	coder_lifecycle(t_coder *coder, t_sim *data);

/* --- Monitor thread --- */
void		*monitor_coder(void *arg);

/* --- Processes --- */
void		create_processes(t_sim *data);
void		wait_for_children(t_sim *data);
void		kill_children(t_sim *data);

/* --- Safe variable access --- */
bool		safe_read_bool(bool *var, sem_t *sem);
void		safe_write_bool(bool *var, bool value, sem_t *sem);
long		safe_read_long(long *var, sem_t *sem);
void		safe_write_long(long *var, long value, sem_t *sem);
long		increment_long(long *var, sem_t *sem);

/* --- Utils --- */
long		get_current_time_in_ms(void);
void		display_state(t_sim *data, int id, const char *state);
void		ft_usleep(long time_ms, t_sim *data);
bool		is_stopped(t_sim *data);

/* --- Clean up --- */
void		clean_up(t_sim *data);
void		error(const char *error_msg, long x, int status, t_sim *data);

#endif
