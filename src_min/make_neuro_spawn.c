#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define HOCFILE "../al_networkSimulation.hoc"

/*definition of the wrapper of MPI_Bcast (function for communication with NEURON) */
void MPI_Bcast_to_NEURON(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    // printf("Entered MPI_Bcast_to_NEURON\n");
    /* datatype: now support MPI_DOUBLE only*/
    MPI_Bcast(&count, 1, MPI_INT, root, comm);
    printf("MPI_Bcast &count \n");
    MPI_Bcast(buffer, count, datatype, root, comm);
    printf("MPI_Bcast buffer \n");
    fflush(stdout);
}/*MPI_Bcast_to_NEURON*/


int main(int argc, char **argv){
    int i, j, k;
    MPI_Comm parentcomm, spawn_parent_comm, intercomm, nrn_comm;
    int main_size, main_myid;
    int spawn_parent_size, spawn_parent_rank;
    int spawn_size, spawn_myid;
    int num_of_procs_nrn = 2; // change in your environment (local, K, Fx100)
    int dimension_per_one_nrnproc, num_of_only_weights,  num_of_params_of_one_nrnrproc;
    int dimension_per_one_nrnproc_w;
    int num_of_weights_per_one_nrnproc;
    // int num_of_weights;
    int num_of_my_pop, dimension;
    int num_sendparams_from_parent, num_sendparams_to_NEURON;
    int offset;

    double *pop_sendbuf_whole, *pop_rcvbuf_whole;
    // double *pop_sendbuf_child_weight;
    // double *pop_rcvbuf_child_weight;

    double *pop_sendbuf_nrn_weight, *pop_sendbuf_nrn_weight_adjust_dim;
    double *pop_rcvbuf_nrn_weight;

    double *arFunvals_child_buf1, *arFunvals_child_buf2;
    double *arFunvals_whole_buf, *arFunvals_whole;
    
    int send_count;
    double flg_termination;
    
    char *exec_neuron=NULL;
    char *neuron_option=NULL;
    char *exec_hoc=NULL;
    char **spawn_argvs=NULL;
    int spawn_argv_size=4;

    char *neuron_argv[] = {
        "-nobanner", 
        "-mpi",
        "-c",
        "{}",
        HOCFILE, 
        NULL};
    // char *neuron_argv[] = {"-mpi", "-nobanner", "/home/hp200177/u00690/neuron_kplus/hoc/test_gather.hoc", NULL};
    int neuron_mode = 1;

    char connection_data[256] = "../data/conMat.txt";
    FILE *fp;
    int dim_conMat=2; //ncell in NEURON
    int **conMat;
    int num_of_cell_combination = dim_conMat * dim_conMat;
    int gene_id;

    /* initialize the MPI process*/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &main_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &main_myid);

    /* make new intracommunicator for communicating with the parent process*/
    MPI_Comm_get_parent(&parentcomm);
    MPI_Intercomm_merge(parentcomm, 1, &spawn_parent_comm);
    MPI_Comm_size(spawn_parent_comm, &spawn_parent_size);
    MPI_Comm_rank(spawn_parent_comm, &spawn_parent_rank);

    /* recieve the information of the scale of population from parent process*/
    /* required: num_of_my_pop, dimension, */
	num_of_my_pop = atoi(argv[1]);
	dimension = atoi(argv[2]);
    num_of_procs_nrn = atoi(argv[3]);
    exec_neuron = (char *)malloc(sizeof(char) * 512);
    if(exec_neuron==NULL){
      printf("memory allocation error occurs @{exec_neuron} in make_neuro_spawn\n");
    }
    sprintf(exec_neuron, "%s", argv[4]);

    neuron_option = (char *)malloc(sizeof(char) * 512);
    if(neuron_option==NULL){
      printf("memory allocation error occurs @{neuron_option} in make_neuro_spawn\n");
    }
    sprintf(neuron_option, "%s", argv[5]);
    neuron_argv[3] = argv[5];

    exec_hoc = (char *)malloc(sizeof(char) * 512);
    if(exec_hoc==NULL){
      printf("memory allocation error occurs @{exec_hoc} in make_neuro_spawn\n");
    }
    sprintf(exec_hoc, "%s", argv[6]);
    neuron_argv[4] = argv[6];

    spawn_argvs = (char **)malloc(sizeof(char *) * spawn_argv_size);
    if(spawn_argvs==NULL){
      printf("memory allocation error occurs @{spawn_argvs}\n");
    }
    for(i=0;i<spawn_argv_size;++i){
      spawn_argvs[i] = (char *)malloc(sizeof(char) * 256);
      if(spawn_argvs[i]==NULL){
        printf("memory allocation error occurs @{spawn_argvs[%d]}\n", i);
      }
    }
    sprintf(spawn_argvs[0], "%d", num_of_my_pop);
    sprintf(spawn_argvs[2], "%d", dimension);
    spawn_argvs[3] = NULL;
    dim_conMat = atoi(argv[7]);
    num_of_cell_combination = dim_conMat * dim_conMat;
    sprintf(spawn_argvs[1], "%d", num_of_cell_combination);
    sprintf(connection_data, "%s", argv[8]);
    printf("connection_data = %s\n", connection_data);

    printf("dimension = %d, num_of_cell_combination = %d\n", dimension, num_of_cell_combination);

    printf("info@make_neuro_spawn:\n");
    printf("num_of_my_pop=%d, dimension=%d, num_of_procs_nrn=%d, exec_neuron=%s, neuron_option=%s, exec_hoc=%s, dim_conMat=%d, connection_data=%s\n", num_of_my_pop, dimension, num_of_procs_nrn, exec_neuron, neuron_option, exec_hoc, dim_conMat, connection_data);

    /* set variables for communication */
    // = num of parameters * num of all pop / num of child procs =  14 * (8 / 4) = 28
    num_sendparams_from_parent = dimension * num_of_my_pop;/* separate weight and delay ver. (for non-separate, delete / 2*/

    // = 14 / 6
    dimension_per_one_nrnproc = num_of_cell_combination / num_of_procs_nrn;
    // = 14 / 6
    dimension_per_one_nrnproc_w = dimension_per_one_nrnproc;
    // = 14 * 2
    num_of_only_weights = num_of_cell_combination * num_of_my_pop;
    // = 28 / 6
    num_of_weights_per_one_nrnproc = num_of_only_weights / num_of_procs_nrn;
    // = 28 / 6
    offset = num_of_weights_per_one_nrnproc;
    // = 14 * 2
    num_sendparams_to_NEURON = num_of_cell_combination * num_of_my_pop;

    /* allocate communication arrays (some arrays are not allocated because they do not need in these processes) */
    

    pop_rcvbuf_whole = (double *)malloc(sizeof(double) * num_sendparams_from_parent);

    pop_sendbuf_nrn_weight = (double *)malloc(sizeof(double) * (offset + num_sendparams_to_NEURON));
    pop_sendbuf_nrn_weight_adjust_dim = (double *)malloc(sizeof(double) * num_sendparams_to_NEURON);
    pop_rcvbuf_nrn_weight = (double *)malloc(sizeof(double) * num_of_weights_per_one_nrnproc);

    arFunvals_child_buf1 = (double *)calloc(num_of_my_pop, sizeof(double));
    arFunvals_child_buf2 = (double *)calloc(num_of_my_pop * num_of_procs_nrn + num_of_my_pop, sizeof(double));

    arFunvals_whole_buf = (double *)calloc(num_of_my_pop, sizeof(double));
    arFunvals_whole = (double *)calloc(num_of_my_pop * spawn_parent_size, sizeof(double));

    conMat = (int **)malloc(sizeof(int *) * dim_conMat);
    for(i=0;i<dim_conMat;++i){
	conMat[i] = (int *)malloc(sizeof(int) * dim_conMat);
    }
    if(neuron_mode){
      if((fp=fopen(connection_data, "r"))==NULL){
	    printf("connection file open error\n");
	    exit(EXIT_FAILURE);
      }
      // dim_conMat is NCELLS (4)
      for(i=0;i<dim_conMat;++i){
	    for(j=0;j<dim_conMat;++j){
	      fscanf(fp, "%d", &conMat[i][j]);
	    }
      }
      fclose(fp);
    }else{
      for(i=0;i<dim_conMat;++i){
	    for(j=0;j<dim_conMat;++j){
	      if((i*dim_conMat+j)<dimension){
	        conMat[i][j] = 1;
	        //printf("%d\t", conMat[i][j]);
	      }else{
	        conMat[i][j] = 0;
	        //printf("%d\t", conMat[i][j]);
	      }
	    }
	//printf("\n");
      }
      //printf("\n");
    }

    
    printf("############### num of nrn procs ################################\n");
    printf("num_of_procs_nrn = %d\n", num_of_procs_nrn);
    printf("############### spawn_argvs (child -> grandchild) ###############\n");
    printf("spawn_argvs[0](num_of_my_pop) = %s\n", spawn_argvs[0]);
    printf("spawn_argvs[1](num_of_cell_combination) = %s\n", spawn_argvs[1]);
    printf("spawn_argvs[2](dimension) = %s\n", spawn_argvs[2]);
    printf("spawn_argvs[3] = %s\n", spawn_argvs[3]);
    printf("#################################################################\n");

    /* make spawn of NEURON procs and make new intracommunicator 'nrn_comm'*/
    /* when it does not work, uncomment the below sentense*/
    /* for(i=0; i < 8; ++i){ */
    /* 	if(spawn_parent_rank%8 == i){ */
    // MPI_Comm_spawn(exec_neuron, spawn_argvs, num_of_procs_nrn, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
    MPI_Comm_spawn(exec_neuron, neuron_argv, num_of_procs_nrn, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
    MPI_Intercomm_merge(intercomm, 0, &nrn_comm);
    MPI_Comm_size(nrn_comm, &spawn_size);
    MPI_Comm_rank(nrn_comm, &spawn_myid);
    /* 	} */
    /* 	MPI_Barrier(MPI_COMM_WORLD); */
    /* } */

    /* send information of the scale of population to NEURON process*/
    send_count = 1;
    double info[] = {1.0, 1.0, 1.0};
    info[0] = num_of_my_pop; 
    neuron_mode = 1;

    // add
    fflush(stdout);

    if(neuron_mode){
      // make_neuro_spawn -> each NEURON : bcast num_of_my_pop
      MPI_Bcast_to_NEURON(info, send_count, MPI_DOUBLE, 0, nrn_comm);
    }


    /* for(i=0;i<dim_conMat;++i){ */
    /*   for(j=0;j<dim_conMat;++j){ */
    /* 	printf("%d\t", conMat[i][j]); */
    /*   } */
    /*   printf("\n"); */
    /* } */
    /* printf("\n"); */
    /* infinite loop for estimation (receive information of genes and pass it to NEURON process*/
    while(1){
        printf("############ 001 ############\n");
	    /* recieve the gene information from parent process*/
        // mplmcma -> make_neuro_spawn : scatter parameters vec (gene * num of parameter)
	    MPI_Scatter(pop_sendbuf_whole, num_sendparams_from_parent, MPI_DOUBLE, pop_rcvbuf_whole, num_sendparams_from_parent, MPI_DOUBLE, 0, spawn_parent_comm);
	    /* transfrom the data structure to suitable manner for communication */

        printf("############ 002 ############\n");
        fflush(stdout);

	    /* printf("\n\n"); */
	    /* printf("original gene:"); */
	    /* for(i=0;i<num_of_my_pop;++i){ */
	    /*   for(j=0;j<dimension;++j){ */
	    /*     printf("%lf\t", pop_rcvbuf_whole[i*dimension + j]); */
	    /*   } */
	    /* } */
	    /* printf("\n\n"); */
	    
	    gene_id = 0;
	    for(k=0;k<num_of_my_pop;++k){
	      for(i=0;i<dim_conMat;++i){
	    	for(j=0;j<dim_conMat;++j){
	    	  pop_sendbuf_nrn_weight_adjust_dim[(dim_conMat * dim_conMat) * k + dim_conMat * i + j] = pop_rcvbuf_whole[gene_id] * conMat[i][j];
	    	  //printf("(dim_conMat * dim_conMat) * k + dim_conMat * i + j = %d\n", (dim_conMat * dim_conMat) * k + dim_conMat * i + j);
	    	  //printf("pop_rcvbuf_whole[%d] * conMat[%d][%d] = %lf\n", gene_id, i, j, pop_rcvbuf_whole[gene_id] * conMat[i][j]);
	    	    gene_id += conMat[i][j];
	    	}
	      }
	    }
	    /* printf("\n\n"); */
	    /* printf("pop_sendbuf_nrn_weight_adjust_dim:"); */
	    /* for(i=0;i<num_of_my_pop;++i){ */
	    /*   for(j=0;j<num_of_cell_combination;++j){ */
	    /*     printf("%lf\t", pop_sendbuf_nrn_weight_adjust_dim[i*num_of_cell_combination+j]); */
	    /*   } */
	    /* } */
	    /* printf("\n\n"); */
        printf("############ 003 ############\n");
        fflush(stdout);

	    for(k=0;k<num_of_procs_nrn;k++){
	      for(i=0;i<num_of_my_pop;i++){
	        for(j=0;j<dimension_per_one_nrnproc;j++){
	          pop_sendbuf_nrn_weight[offset + (num_of_my_pop * dimension_per_one_nrnproc) * k + dimension_per_one_nrnproc * i + j]= pop_sendbuf_nrn_weight_adjust_dim[ num_of_cell_combination * i + j + dimension_per_one_nrnproc * k];
	        }
	      }
	    }

	    /* printf("\n\n"); */
	    /* printf("pop_sendbuf_nrn_weight:"); */
	    /* for(i=0;i<num_of_my_pop;++i){ */
	    /*   for(j=0;j<num_of_cell_combination;++j){ */
	    /*     printf("%lf\t", pop_sendbuf_nrn_weight[offset+i*num_of_cell_combination+j]); */
	    /*   } */
	    /* } */
	    /* printf("\n\n"); */


	    /* for(k=0;k<num_of_my_pop;++k){ */
	    /*   for(i=0;i<dim_conMat;++i){ */
	    /* 	for(j=0;j<dim_conMat;++j){ */
	    /* 	  printf("pop_sendvec_makeneurospawn[%d][%d] = %lf\n", i, j, pop_sendbuf_nrn_weight[offset + num_of_cell_combination * k + dimension_per_one_nrnproc * i + j]); */
	    /* 	} */
	    /*     } */
	    /* } */
	    
	    
	    
	    /* for(k=0;k<num_of_my_pop;++k){ */
	    /*     for(i=0;i<dim_conMat;++i){ */
	    /* 	for(j=0;j<dim_conMat;++j){ */
	    /* 	    printf("w[%d][%d] = %lf \t", i, j, pop_sendbuf_nrn_weight[offset + num_of_cell_combination * k + dimension_per_one_nrnproc * i + j]); */

	    /* 	} */
	    /* 	printf("\n"); */
	    /*     } */
	    /*     printf("\n\n"); */
	    /* } */
	    
        printf("############ 004 ############\n");
        fflush(stdout);

	    MPI_Barrier(MPI_COMM_WORLD);

        printf("############ 005 ############\n");
	    fflush(stdout);
	    
	    /*pass the gene information to NEURON process*/
        // make_neuro_spawn -> each NEURON : bcast parameters vec (gene * num of parameter / num of neuron procs)
	    // MPI_Scatter(pop_sendbuf_nrn_weight, num_of_weights_per_one_nrnproc, MPI_DOUBLE, pop_rcvbuf_nrn_weight, num_of_weights_per_one_nrnproc, MPI_DOUBLE, 0, nrn_comm);
        // dimension: num of parameter: 14
        // make_neuro_spawn -> each NEURON : bcast parameters vec (gene * num of parameter / num of neuron procs)
        MPI_Bcast_to_NEURON(pop_rcvbuf_whole, dimension * num_of_my_pop, MPI_DOUBLE, 0, nrn_comm);

        printf("############ 006 ############\n");
	    fflush(stdout);
	    
	    /* wait for finish calculation in NEURON*/
        // each NEURON -> make_neuro_spawn : gather fitting vec ([fitting value(gene1), fitting value(gene2), ...])
	    MPI_Gather(arFunvals_child_buf1, num_of_my_pop, MPI_DOUBLE, arFunvals_child_buf2, num_of_my_pop, MPI_DOUBLE, 0, nrn_comm);

        printf("############ 007 ############\n");
	    fflush(stdout);

	    /* tally the score information of my process */
	    for(i=0; i<num_of_my_pop; ++i){
	        arFunvals_whole_buf[i] = arFunvals_child_buf2[i + num_of_my_pop];
	        // printf("@@@@@@@@@@@@@@@@@@@@@@@@@\narFunvals_whole_buf[%d] = %lf\n@@@@@@@@@@@@@@@@@@@@@@@@@\n", i, arFunvals_whole_buf[i]);
	    }
	    MPI_Barrier(MPI_COMM_WORLD);

        printf("############ 008 ############\n");
	    fflush(stdout);

	    /* pass the score information to parent process*/
        // make_neuro_spawn -> mplmcma : gather whole fitting vec ([fitting value(gene1), fitting value(gene2), ...])
        // printf("arFunvals_whole_buf.size() is %d\n", arFunvals_whole_buf.size());
        printf("arFunvals_whole_buf[0] is %f\n", arFunvals_whole_buf[0]);
	    MPI_Gather(arFunvals_whole_buf, num_of_my_pop, MPI_DOUBLE, arFunvals_whole, num_of_my_pop, MPI_DOUBLE, 0, spawn_parent_comm);

        printf("############ 009 ############\n");
	    fflush(stdout);

	    /* recieve and pass the information whether the loop is terminated or not */
        // mplmcma -> make_neuro_spawn : bcast flg_termination
	    MPI_Bcast(&flg_termination, 1, MPI_DOUBLE, 0, spawn_parent_comm);

        printf("############ 010 ############\n");
	    fflush(stdout);

	    if(neuron_mode){
          // make_neuro_spawn -> each NEURON : bcast flg_termination
	      MPI_Bcast_to_NEURON(&flg_termination, 1, MPI_DOUBLE, 0, nrn_comm);
          printf("############ 011 ############\n");
	      fflush(stdout);
	    }else{
	      MPI_Bcast(&flg_termination, 1, MPI_DOUBLE, 0, nrn_comm);
	    }
	    /* termination*/
	    if((int)flg_termination){
	        break;
	    }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* free the allocated memory */
    free(exec_neuron);
    free(neuron_option);
    free(exec_hoc);
    free(pop_sendbuf_nrn_weight);
    free(pop_rcvbuf_nrn_weight);
    free(arFunvals_child_buf1);
    free(arFunvals_child_buf2);
    free(arFunvals_whole_buf);
    free(arFunvals_whole);
    // add free newly
    free(pop_rcvbuf_whole);
    free(pop_sendbuf_nrn_weight_adjust_dim);

   
    for(i=0;i<dim_conMat;++i){
	free(conMat[i]);
    }
    free(conMat);
 
    MPI_Finalize();

    return 0;
}
