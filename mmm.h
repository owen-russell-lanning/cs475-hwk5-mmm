#ifndef MMM_H_
#define MMM_H_

// globals (anything here would be shared with all threads) */
// I would declare the pointers to the matrices here (i.e., extern double **A, **B, **C),
// as well as the size of the matrices, etc.

void mmm_init(int size, int threads);
void mmm_reset(int **);
void mmm_freeup();
void mmm_seq();
void *mmm_par(void *args);
double mmm_verify();
int get_random_int(int max);

// create data structure to pass to threads
struct Mat_Params
{
    int start_row;
    int end_row;
};

#endif /* MMM_H_ */
