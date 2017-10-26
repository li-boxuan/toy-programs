#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

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

    printf("prepare to mmap\n");
    char *map = mmap(0, TOT_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    long cnt = 0;
    double min_latency = 10000;
    double tot_latency = 0;
    double max_latency = 0;
    double stime = millitime();
    while (cnt < TOT_LEN)
    {
    	double stime = millitime();
        int i;
    	for (i = 0; i < BUF_LEN; i++)
    	{
	    if ((i + cnt) >=  TOT_LEN)
		break;
    	    map[i + cnt] = buf[i];
    	}
	
	if (cnt + BUF_LEN < TOT_LEN)
    	    if (msync(map + cnt, BUF_LEN, MS_SYNC) == -1)
    	    {
    	        perror("Could not sync the file to disk");
    	    }

        cnt += BUF_LEN;
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
    if (munmap(map, BUF_LEN) == -1)
    {
        close(fd);
        perror("Error un-mmapping the file");
        exit(EXIT_FAILURE);
    }
    return 0;
}
