#include <mpi.h>
#include <bc/bc.h>

int main(int argc, char *argv[])
{
	bc_chan_t *src, *sink;
	bc_plist_t *pl;
	bc_dtype_t *ntype;
	int value[2] = {1, 2};

    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	ntype = bc_dtype_create(sizeof(int));

	if (bc_rank == 0)
		pl = bc_plist_create(1, 1);
	else
		pl = bc_plist_create(1, 0);

	src = bc_src_create(pl, bc_int, BC_ROLE_PIPE);
	sink = bc_sink_create(pl, bc_int, 10, BC_ROLE_PIPE);

	bc_put(sink, value, 2);
  	bc_get(src, value, 2); 

	bc_chan_destroy(src);
	bc_chan_destroy(sink);
	bc_plist_destroy(pl);
	bc_dtype_destroy(NULL);

    bc_final();
    MPI_Finalize();
    return 0;
}
