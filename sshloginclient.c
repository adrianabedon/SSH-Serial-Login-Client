#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t execute_pppd() {
    pid_t pid;
    if((pid = fork()) < 0) {
        printf("Fork failed\n");
        exit(0);
    } else if (pid == 0) {
        char *argv[] = {"/usr/sbin/pppd", "/dev/ttyS1", "115200", "lock", "nodetach", "noauth", "debug", "192.0.0.13:", 0};
	extern char** environ;
        execve(argv[0], &argv[0], environ);
        printf("Failed to execute pppd\n");
        exit(0);
    }
    return pid;
}

pid_t execute_ssh() {
    pid_t pid;
    if((pid = fork()) < 0) {
        printf("Fork failed\n");
        exit(0);
    } else if (pid == 0) {
        char *argv[] = {"/usr/bin/ssh", "aabedon@195.0.0.12", "-i", "/home/aabedon2/.ssh/id_rsa","-p", "23", 0};
	extern char** environ;
        execve(argv[0], &argv[0], environ);
        printf("Failed to execute ssh\n");
        exit(0);
    }
    return pid;
}

pid_t execute_login() {
    pid_t pid;
    if((pid = fork()) < 0) {
        printf("Fork failed\n");
		exit(0);
    } else if (pid == 0) {
        pid_t pid_pppd = execute_pppd();
        sleep(5);
        pid_t pid_ssh = execute_ssh();
        waitpid(pid_ssh, NULL, 0);
        kill(pid_pppd, SIGINT);
        waitpid(pid_pppd, NULL, 0);
        exit(0);
    }
    return pid;
}


int main() {
    pid_t pid_login = execute_login();
    waitpid(pid_login,NULL,0);
    // extern char** environ;
    // char *argv[] = {"/usr/bin/screen", "/dev/ttyS1", "115200", 0};
    // execve(argv[0], &argv[0], environ);
    // printf("Failed to execute screen\n");
    exit(0);
}
