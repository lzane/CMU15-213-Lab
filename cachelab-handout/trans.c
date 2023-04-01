/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int B_col = 1<<(5-2);// each cache line can hold B_col int
    int B_row = B_col;

    if(M==32&&N==32){
        B_row = 8;
    }

    if(M==64&&N==64){
        // because the cache can only contains 4 matrix lines, use 8x8 block size will cause 
        // conflict misses when writing to B
        B_row = 4; 
    }

    if(M==61&&N==67){
        B_row = 8;
    }

    int ti;
    int tj;
    int tv;
    
    for(int i=0; i<M; i+=B_row){
        for(int j=0; j<N; j+=B_col){
            for(int jj=j; jj<MIN(N, j+B_col); jj++){
                ti = -1;
                tj = -1;
                for(int ii=i; ii<MIN(M, i+B_row);ii++){
                    // postpone the write in diagonal to avoid conflict misses
                    if(ii == jj){
                        ti = ii;
                        tj = jj;
                        tv = A[jj][ii];
                        continue;
                    }
                    B[ii][jj] = A[jj][ii];
                }
                if(ti!=-1){
                    B[ti][tj] = tv;
                }
            }
        }
    }
}

/**
 * only use blocking
*/
char blocking_trans_desc[] = "blocking transform";
void blocking_trans(int M, int N, int A[N][M], int B[M][N])
{
    int B_col = 1<<(5-2);// each cache line can hold B_col int
    int B_row = B_col;

    if(M==32&&N==32){
        B_row = 8;
    }

    if(M==64&&N==64){
        B_row = 4;
    }

    if(M==61&&N==67){
        B_row = 8;
    }

    for(int i=0; i<M; i+=B_row){
        for(int j=0; j<N; j+=B_col){
            for(int jj=j; jj<MIN(N, j+B_col); jj++){
                for(int ii=i; ii<MIN(M, i+B_row);ii++){
                    B[ii][jj] = A[jj][ii];
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    registerTransFunction(blocking_trans, blocking_trans_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

