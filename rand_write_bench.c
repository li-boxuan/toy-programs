#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 5000
#define BUF_LEN 4096
#define TOT_LEN 1024 * 1024 * 1024

static inline double millitime(){
    struct timeval now;
    gettimeofday(&now, NULL);
    double ret = now.tv_sec + now.tv_usec/1000.0/1000.0;
    return ret;
}
int main(int argc, char* argv[])
{
    int ret;
    int fd = open(argv[1], O_RDWR | O_CREAT);
    time_t t;
    srand((unsigned) time(&t));
    printf("srand success\n");
    if (fd == -1)
    {
        printf("please input a file to write to, e.g. './tmp.txt'\n");
        perror("open");
        abort();
    }
    char buf[BUF_SIZE] = "";
    int len = BUF_LEN;
    int i;
    for (i = 0; i < len; i++)
        buf[i] = '+';
    long cnt = 0;
    double min_latency = 10000;
    double tot_latency = 0;
    double max_latency = 0;
    double stime = millitime();
    int offset;
    while (cnt < TOT_LEN)
    {
        offset = rand() % TOT_LEN;
    	double stime = millitime();
        ret = lseek(fd, offset, SEEK_SET);
        if (ret < 0)
        {
            perror("lseek");
            abort();
        }
        ret = write(fd, buf, len);
        if (ret != len)
        {
            perror("write");
            abort();
        }
        cnt += ret;
        ret = fsync(fd);
        if (ret != 0)
        {
            perror("fsync");
            abort();
        }
	double latency = millitime() - stime;
	if (latency < min_latency)
	    min_latency = latency;
	if (latency > max_latency)
	    max_latency = latency;
	tot_latency += latency;
    }
    double time_cost = millitime() - stime;
    double tput = cnt / 1024 / 1024 / time_cost; 
    printf("Throughput (MB/s): %.6f\n", tput);
    printf("min_latency = %f\n", min_latency * 1000000);
    printf("max_latency = %f\n", max_latency * 1000000);
    printf("tot_latency = %f\n", tot_latency * 1000000);
    printf("avg_latency = %f\n", tot_latency / cnt * BUF_LEN * 1000000);
    return 0;
}
