#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <process.h>
#include <time.h>
#include <stdint.h>

/* Minimal pthread compatibility layer for Windows (_beginthreadex) */
typedef HANDLE pthread_t;
static int pthread_create(pthread_t *thread, const void *attr, void *(*start_routine)(void*), void *arg)
{
    (void)attr;
    uintptr_t handle = _beginthreadex(NULL, 0, (unsigned (__stdcall *)(void*))start_routine, arg, 0, NULL);
    if (handle == 0)
        return -1;
    *thread = (HANDLE)handle;
    return 0;
}
static int pthread_join(pthread_t thread, void **retval)
{
    (void)retval;
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

/* On Windows, fork()/wait() are not available; those parts are handled with fallbacks. */
#else
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#endif

int *arr;
int size;
int num1,num2;
int prime_limit;
int n;

int **A, **B, **C;

/* WALL CLOCK TIME FUNCTION */

double get_time()
{
#if defined(_WIN32) || defined(_WIN64)
    /* Use GetTickCount64 on Windows for wall-clock time in seconds */
    return (double)GetTickCount64() / 1000.0;
#else
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec + t.tv_usec/1000000.0;
#endif
}

/* HEAVY WORKLOAD FUNCTION */

void heavy_work()
{
    volatile long long dummy=0;

    for(long long i=0;i<500000000;i++)
    {
        dummy += i%7;
    }
}

/* ARRAY SUM FUNCTION */

void array_sum()
{
    int found1=0,found2=0;

    for(int i=0;i<size;i++)
    {
        if(arr[i]==num1)
            found1=1;

        if(arr[i]==num2)
            found2=1;
    }

    if(found1 && found2)
    {
        int result=num1+num2;

        printf("\nArray Calculation:\n");
        printf("%d + %d = %d\n",num1,num2,result);
    }
    else
    {
        printf("\nNumbers not found in array\n");
    }

    heavy_work();
}

/* PRIME NUMBER FUNCTION */

void prime_numbers()
{
    int count=0;

    printf("\nPrime numbers upto %d:\n",prime_limit);

    for(int i=2;i<=prime_limit;i++)
    {
        int flag=1;

        for(int j=2;j*j<=i;j++)
        {
            if(i%j==0)
            {
                flag=0;
                break;
            }
        }

        if(flag)
        {
            printf("%d ",i);
            count++;
        }
    }

    printf("\nTotal primes = %d\n",count);

    heavy_work();
}

/* MATRIX MULTIPLICATION */

void matrix_multiply()
{
    printf("\nMatrix Result:\n");

    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            C[i][j]=0;

            for(int k=0;k<n;k++)
            {
                C[i][j]+=A[i][k]*B[k][j];
            }

            printf("%d ",C[i][j]);
        }

        printf("\n");
    }

    heavy_work();
}

/* THREAD FUNCTIONS */

void* thread_task1(void* arg)
{
    array_sum();
    return NULL;
}

void* thread_task2(void* arg)
{
    prime_numbers();
    return NULL;
}

void* thread_task3(void* arg)
{
    matrix_multiply();
    return NULL;
}

/* SEQUENTIAL EXECUTION */

double sequential_execution()
{
    double start,end;

    start=get_time();

    array_sum();
    prime_numbers();
    matrix_multiply();

    end=get_time();

    return end-start;
}

/* THREAD EXECUTION */

double thread_execution()
{
    pthread_t t1,t2,t3;

    double start,end;

    start=get_time();

    pthread_create(&t1,NULL,thread_task1,NULL);
    pthread_create(&t2,NULL,thread_task2,NULL);
    pthread_create(&t3,NULL,thread_task3,NULL);

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_join(t3,NULL);

    end=get_time();

    return end-start;
}

/* PROCESS EXECUTION */

#ifndef _WIN32
double process_execution()
{
    double start,end;

    start=get_time();

    pid_t p1,p2,p3;

    p1=fork();
    if(p1==0)
    {
        array_sum();
        exit(0);
    }

    p2=fork();
    if(p2==0)
    {
        prime_numbers();
        exit(0);
    }

    p3=fork();
    if(p3==0)
    {
        matrix_multiply();
        exit(0);
    }

    wait(NULL);
    wait(NULL);
    wait(NULL);

    end=get_time();

    return end-start;
}
#else
/* Fallback on Windows: fork() is not available, run tasks sequentially */
double process_execution()
{
    double start,end;

    start=get_time();

    array_sum();
    prime_numbers();
    matrix_multiply();

    end=get_time();

    return end-start;
}
#endif

/* MAIN FUNCTION */

int main()
{
    double seq,thr,proc;

    printf("Enter array size: ");
    scanf("%d",&size);

    arr=malloc(size*sizeof(int));

    printf("Enter array elements:\n");

    for(int i=0;i<size;i++)
        scanf("%d",&arr[i]);

    printf("Enter first number to sum: ");
    scanf("%d",&num1);

    printf("Enter second number to sum: ");
    scanf("%d",&num2);

    printf("Enter prime limit: ");
    scanf("%d",&prime_limit);

    printf("Enter matrix size: ");
    scanf("%d",&n);

    A=malloc(n*sizeof(int*));
    B=malloc(n*sizeof(int*));
    C=malloc(n*sizeof(int*));

    for(int i=0;i<n;i++)
    {
        A[i]=malloc(n*sizeof(int));
        B[i]=malloc(n*sizeof(int));
        C[i]=malloc(n*sizeof(int));
    }

    printf("Enter Matrix A:\n");

    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            scanf("%d",&A[i][j]);

    printf("Enter Matrix B:\n");

    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            scanf("%d",&B[i][j]);

    printf("\n----- Sequential Execution -----\n");
    seq=sequential_execution();

    printf("\n----- Thread Execution -----\n");
    thr=thread_execution();

    printf("\n----- Process Execution -----\n");
    proc=process_execution();

    printf("\n===== PERFORMANCE =====\n");

    printf("Sequential Time : %f\n",seq);
    printf("Thread Time     : %f\n",thr);
    printf("Process Time    : %f\n",proc);

    if(thr<seq && thr<proc)
        printf("\nFastest Method : Multithreading\n");

    else if(proc<seq && proc<thr)
        printf("\nFastest Method : Multiprocessing\n");

    else
        printf("\nFastest Method : Sequential\n");

    return 0;
} 