#include "codexion.h"

int	main(int argc, char **argv)
{
	t_sim	data;

	parse_arguments(argc, argv);
	init_data(argv, &data);
	create_threads(&data);
	wait_for_threads(&data);
	clean_up(&data);
	return (0);
}
