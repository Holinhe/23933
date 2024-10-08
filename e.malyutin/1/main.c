#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

extern char **environ;

void usage(char *name) {
    printf("usage: %s [-i] [-s] [-p] [-u] [-Unew_limit] [-c] [-Csize] [-d] "
           "[-v] [-Vname=value] [-h]\n",
           name);
}

unsigned long read_uint(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            printf("invalid number\n");
            exit(1);
        }
    }

    return atoi(str);
}

int main(int argc, char **argv, char **envp) {
    int c;

    if (argc == 1) {
        usage(argv[0]);
        return 0;
    }

    while ((c = getopt(argc, argv, "hispuU:cC:dvV:")) != -1) {
        if (c == 'i') {
            uid_t ruid = getuid();
            uid_t uid = geteuid();
            gid_t rgid = getgid();
            gid_t gid = getegid();

            printf("real uid: %d\n", ruid);
            printf("effective uid: %d\n", uid);
            printf("real gid: %d\n", rgid);
            printf("effective gid: %d\n", gid);
        } else if (c == 's') {
            setpgid(0, 0);
        } else if (c == 'p') {
            pid_t pid = getpid();
            pid_t ppid = getppid();
            pid_t pgid = getpgrp();

            printf("process id: %d\n", pid);
            printf("parent process id: %d\n", ppid);
            printf("process group id: %d\n", pgid);
        } else if (c == 'u') {
            struct rlimit rlimit;
            if (getrlimit(RLIMIT_NOFILE, &rlimit) != 0) {
                perror("getrlimit failed");
                return 1;
            }
            printf("ulimit: %lu\n", rlimit.rlim_cur);
        } else if (c == 'U') {
            rlim_t set = read_uint(optarg);

            struct rlimit rlimit;
            if (getrlimit(RLIMIT_NOFILE, &rlimit) != 0) {
                perror("getrlimit failed");
                return 1;
            }

            rlimit.rlim_cur = set;

            if (setrlimit(RLIMIT_NOFILE, &rlimit) != 0) {
                perror("setrlimit failed");
                return 1;
            }
        } else if (c == 'c') {
            struct rlimit rlimit;
            if (getrlimit(RLIMIT_CORE, &rlimit) != 0) {
                perror("getrlimit failed");
                return 1;
            }
            printf("core file max size: %lu\n", rlimit.rlim_cur);
        } else if (c == 'C') {
            rlim_t set = read_uint(optarg);

            struct rlimit rlimit;
            if (getrlimit(RLIMIT_CORE, &rlimit) != 0) {
                perror("getrlimit failed");
                return 1;
            }

            rlimit.rlim_cur = set;

            if (setrlimit(RLIMIT_CORE, &rlimit) != 0) {
                perror("setrlimit failed");
                return 1;
            }
        } else if (c == 'd') {
            char buf[1024];
            if (getcwd(buf, sizeof(buf)) == NULL) {
                perror("getcwd failed");
                return 1;
            }
            printf("%s\n", buf);
        } else if (c == 'v') {
            for (char **env = environ; *env != 0; env++) {
                printf("%s\n", *env);
            }
        } else if (c == 'V') {
            int n = 0;
            while (1) {
                if (optarg[n] == '=') {
                    break;
                } else if (optarg[n] == 0) {
                    printf("please key value\n");
                    return 1;
                }
                n++;
            }

            char *name = malloc(n);
            memcpy(name, optarg, n);
            name[n] = 0;

            int kl = strlen(optarg) - n;
            char *value = malloc(kl);
            memcpy(value, &optarg[n + 1], kl);
            value[kl] = 0;

            if (setenv(name, value, 1) != 0) {
                perror("setenv failed");
                return 1;
            }

            free(name);
            free(value);
        } else if (c == 'h') {
            usage(argv[0]);
        } else {
            usage(argv[0]);
            return 1;
        }
    }
}