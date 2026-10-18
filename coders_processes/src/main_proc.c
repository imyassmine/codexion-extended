#include "codexion_processes.h"

int	main(int argc, char **argv)
{
	t_sim	data;

	(void)setvbuf(stdout, NULL, _IONBF, 0);
	parse_arguments(argc, argv);
	init_data(argv, &data);
	create_processes(&data);
	wait_for_children(&data);
	clean_up(&data);
	return (0);
}
