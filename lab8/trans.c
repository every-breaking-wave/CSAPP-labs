/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */

// id = 520030910021 name = Feng Yifei
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

// s = 5, E = 1, b = 5

char transpose_submit_desc[] = "Transpose submission";

void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, tmp, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;

    //    for (i = 0; i < N; i++) {
    //        for (j = 0; j < M; j++) {
    //            tmp = A[i][j];
    //            B[j][i] = tmp;
    //        }
    //    }

    if (M == 32 && N == 32)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 8; k++)
                {
                    tmp = A[i * 8][j * 8 + k];
                    tmp1 = A[i * 8 + 1][j * 8 + k];
                    tmp2 = A[i * 8 + 2][j * 8 + k];
                    tmp3 = A[i * 8 + 3][j * 8 + k];
                    tmp4 = A[i * 8 + 4][j * 8 + k];
                    tmp5 = A[i * 8 + 5][j * 8 + k];
                    tmp6 = A[i * 8 + 6][j * 8 + k];
                    tmp7 = A[i * 8 + 7][j * 8 + k];
                    B[j * 8 + k][i * 8] = tmp;
                    B[j * 8 + k][i * 8 + 1] = tmp1;
                    B[j * 8 + k][i * 8 + 2] = tmp2;
                    B[j * 8 + k][i * 8 + 3] = tmp3;
                    B[j * 8 + k][i * 8 + 4] = tmp4;
                    B[j * 8 + k][i * 8 + 5] = tmp5;
                    B[j * 8 + k][i * 8 + 6] = tmp6;
                    B[j * 8 + k][i * 8 + 7] = tmp7;
                }
                // read a 8*8 block
            }
        }
        return;
    }

    if (M == 64 && N == 64)
    {
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                for (k = 0; k < 4; k++)
                {
                    tmp = A[i * 8 + k][j * 8];
                    tmp1 = A[i * 8 + k][j * 8 + 1];
                    tmp2 = A[i * 8 + k][j * 8 + 2];
                    tmp3 = A[i * 8 + k][j * 8 + 3];
                    tmp4 = A[i * 8 + k][j * 8 + 4];
                    tmp5 = A[i * 8 + k][j * 8 + 5];
                    tmp6 = A[i * 8 + k][j * 8 + 6];
                    tmp7 = A[i * 8 + k][j * 8 + 7];
                    B[j * 8][i * 8 + k] = tmp;
                    B[j * 8 + 1][i * 8 + k] = tmp1;
                    B[j * 8 + 2][i * 8 + k] = tmp2;
                    B[j * 8 + 3][i * 8 + k] = tmp3;
                    B[j * 8][i * 8 + k + 4] = tmp4;
                    B[j * 8 + 1][i * 8 + k + 4] = tmp5;
                    B[j * 8 + 2][i * 8 + k + 4] = tmp6;
                    B[j * 8 + 3][i * 8 + k + 4] = tmp7;
                }
                for (k = 0; k < 4; k++)
                {
                    tmp = B[j * 8 + k][i * 8 + 4];
                    tmp1 = B[j * 8 + k][i * 8 + 5];
                    tmp2 = B[j * 8 + k][i * 8 + 6];
                    tmp3 = B[j * 8 + k][i * 8 + 7];

                    tmp4 = A[i * 8 + 4][j * 8 + k];
                    tmp5 = A[i * 8 + 5][j * 8 + k];
                    tmp6 = A[i * 8 + 6][j * 8 + k];
                    tmp7 = A[i * 8 + 7][j * 8 + k];

                    B[j * 8 + k][i * 8 + 4] = tmp4;
                    B[j * 8 + k][i * 8 + 5] = tmp5;
                    B[j * 8 + k][i * 8 + 6] = tmp6;
                    B[j * 8 + k][i * 8 + 7] = tmp7;

                    tmp4 = A[i * 8 + 4][j * 8 + k + 4];
                    tmp5 = A[i * 8 + 5][j * 8 + k + 4];
                    tmp6 = A[i * 8 + 6][j * 8 + k + 4];
                    tmp7 = A[i * 8 + 7][j * 8 + k + 4];

                    B[j * 8 + 4 + k][i * 8] = tmp;
                    B[j * 8 + 4 + k][i * 8 + 1] = tmp1;
                    B[j * 8 + 4 + k][i * 8 + 2] = tmp2;
                    B[j * 8 + 4 + k][i * 8 + 3] = tmp3;
                    B[j * 8 + 4 + k][i * 8 + 4] = tmp4;
                    B[j * 8 + 4 + k][i * 8 + 5] = tmp5;
                    B[j * 8 + 4 + k][i * 8 + 6] = tmp6;
                    B[j * 8 + 4 + k][i * 8 + 7] = tmp7;
                }
            }
        }
        return;
    }

    if (M == 61 && N == 67)
    {
        for (j = 0; j < 61; j += 8)
        {
            for (i = 0; i < 67; i++)
            {
                tmp = A[i][j];
                tmp1 = A[i][j + 1];
                tmp2 = A[i][j + 2];
                tmp3 = A[i][j + 3];
                tmp4 = A[i][j + 4];
                tmp5 = A[i][j + 5];
                tmp6 = A[i][j + 6];
                tmp7 = A[i][j + 7];
                B[j][i] = tmp;
                B[j + 1][i] = tmp1;
                B[j + 2][i] = tmp2;
                B[j + 3][i] = tmp3;
                B[j + 4][i] = tmp4;
                B[j + 5][i] = tmp5;
                B[j + 6][i] = tmp6;
                B[j + 7][i] = tmp7;
            }
        }

        return;
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

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
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

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
