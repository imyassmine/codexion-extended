#include "codexion_processes.h"

static bool	is_positive_int(const char *value)
{
	long	nbr;
	int		i;

	nbr = 0;
	i = 0;
	if (!*value || value[0] == '-')
		return (false);
	if (value[i] == '+')
		i++;
	if (value[i] < '0' || value[i] > '9')
		return (false);
	while (value[i])
	{
		if (value[i] < '0' || value[i] > '9')
			return (false);
		nbr = nbr * 10 + (value[i] - '0');
		if (nbr > INT_MAX)
			return (false);
		i++;
	}
	return (true);
}

void	parse_arguments(int argc, char **argv)
{
	const char	*args[] = {"number_of_coders", "time_to_burnout",
		"time_to_compile", "time_to_debug", "time_to_refactor",
		"number_of_compiles_required", "dongle_cooldown"};
	size_t		i;

	if (argc != 9)
		error("Invalid number of arguments\n", 0, 1, NULL);
	i = 0;
	while (i < sizeof(args) / sizeof(*args))
	{
		if (is_positive_int(argv[i + 1]) == false)
		{
			fprintf(stderr, "Error: %s must be a positive integer\n",
				args[i]);
			exit(1);
		}
		i++;
	}
	if (atoi(argv[1]) == 0)
		error("Error: number_of_coders must be at least 1\n", 0, 1, NULL);
	if (strcmp(argv[8], "fifo") && strcmp(argv[8], "edf"))
		error("Error: scheduler must be either 'fifo' or 'edf'\n", 0, 1, NULL);
}
