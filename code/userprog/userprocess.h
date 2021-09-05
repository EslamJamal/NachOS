


typedef struct {
    char *filename;
    int pid;
}wrapper;

static int pid = 1;

extern int do_ForkAndExec(char *filepath);
extern void StartNewProcess(int args);
