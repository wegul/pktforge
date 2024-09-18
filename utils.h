#define ONEGb 1024 * 1024 * 1024L
#define BUFFER_SIZE ONEGb * 2
#define FSIZE BUFFER_SIZE
#define EPOCH 500
#define MAX_CONN 32
#define NR_CONN 2

struct Stat {
    uint64_t bytes;
    struct timespec start;
    struct timespec end;
};

double cal_xput(struct Stat st) {
    int bytes = st.bytes;
    double time_taken, xput;
    struct timespec start = st.start, end = st.end;
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    xput = (bytes / ONEGb) / time_taken;  // GB/s
    return xput * 8;
}

double cal_time(struct Stat st) {
    double time_taken;
    struct timespec start = st.start, end = st.end;
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    return time_taken;
}