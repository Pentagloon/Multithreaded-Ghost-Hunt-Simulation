#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    int runs = 30;
    if (argc > 1) {
        int tmp = atoi(argv[1]);
        if (tmp > 0) {
            runs = tmp;
        }
    }

    sigset_t mask;
    if (sigemptyset(&mask) == -1) {
        perror("sigemptyset");
        return 1;
    }
    if (sigaddset(&mask, SIGUSR1) == -1) {
        perror("sigaddset SIGUSR1");
        return 1;
    }
    if (sigaddset(&mask, SIGUSR2) == -1) {
        perror("sigaddset SIGUSR2");
        return 1;
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    int hunterWins = 0;
    int ghostWins  = 0;

    const char* sim_path = "./p5";

    for (int i = 0; i < runs; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        } else if (pid == 0) {
            sigprocmask(SIG_UNBLOCK, &mask, NULL);

            execl(sim_path, sim_path, "--report", (char*)NULL);

            perror("execl");
            _exit(1);
        }

        int sig = 0;
        int res = sigwait(&mask, &sig);
        if (res != 0) {
            fprintf(stderr, "sigwait failed with code %d\n", res);
        } else {
            if (sig == SIGUSR1) {
                hunterWins++;
            } else if (sig == SIGUSR2) {
                ghostWins++;
            }
        }

        int status = 0;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
        }
    }

    printf("\n================ Simulation Summary ================\n");
    int total = hunterWins + ghostWins;
    printf("Total simulations run : %d\n", total);
    printf("Hunter wins (SIGUSR1): %d\n", hunterWins);
    printf("Ghost wins  (SIGUSR2): %d\n", ghostWins);

    printf("\nHunter : Ghost win ratio = %d : %d", hunterWins, ghostWins);
    if (ghostWins > 0) {
        double ratio = hunterWins / (double)ghostWins;
        printf("  (~ %.2f)\n", ratio);
    } else {
        printf("  (ghostWins == 0, no ratio)\n");
    }

    if (total > 0) {
        double hunterPct = 100.0 * hunterWins / (double)total;
        double ghostPct  = 100.0 * ghostWins  / (double)total;
        printf("Hunter win percentage: %.2f%%\n", hunterPct);
        printf("Ghost  win percentage: %.2f%%\n", ghostPct);
    }

    printf("===================================================\n");

    return 0;
}
