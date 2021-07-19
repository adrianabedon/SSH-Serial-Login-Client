#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t execute_pppd()
{
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        printf("Fork failed\n");
        exit(0);
    }
    else if (pid == 0)
    {
        char *argv[] = {"/usr/sbin/pppd", "/dev/ttyS1", "115200", "lock", "nodetach", "noauth", "debug", "192.0.0.13:", 0};
        extern char **environ;
        execve(argv[0], &argv[0], environ);
        printf("Failed to execute pppd\n");
        exit(0);
    }
    return pid;
}

pid_t execute_ssh(char *host, char *keyloc)
{
    if (host == NULL)
    {
        printf("no host address specified\n");
    }
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        printf("Fork failed\n");
        exit(0);
    }
    else if (pid == 0)
    {
        if (keyloc == NULL)
        {
            char *argv[] = {"/usr/bin/ssh", host, "-p", "23", 0};
            extern char **environ;
            execve(argv[0], &argv[0], environ);
            printf("Failed to execute ssh\n");
            exit(0);
        }
        else
        {
            char *argv[] = {"/usr/bin/ssh", host, "-i", keyloc, "-p", "23", 0};
            extern char **environ;
            execve(argv[0], &argv[0], environ);
            printf("Failed to execute ssh\n");
            exit(0);
        }
    }
    return pid;
}

pid_t execute_login(char *host, char *keyloc)
{
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        printf("Fork failed\n");
        exit(0);
    }
    else if (pid == 0)
    {
        pid_t pid_pppd = execute_pppd();
        sleep(5);
        pid_t pid_ssh = execute_ssh(host, keyloc);
        waitpid(pid_ssh, NULL, 0);
        kill(pid_pppd, SIGINT);
        waitpid(pid_pppd, NULL, 0);
        exit(0);
    }
    return pid;
}

int main(int argc, char *argv[])
{
    int usekey = 0;
    if (argc == 1)
    {
        printf("You must specify a host name (Ex/ aabedon@195.0.0.12)");
        return 1;
    }
    if (argc > 3)
    {
        printf("To many arguments provided");
        return 1;
    }
    printf("Connecting to %s\n", argv[1]);
    if (argc == 2)
    {
        printf("No key specified, login using password.\n");
        pid_t pid_login = execute_login(argv[1], NULL);
        waitpid(pid_login, NULL, 0);
        exit(0);
    }
    else if (argc == 3)
    {
        printf("Will use key at specified directory %s.\n", argv[2]);
        pid_t pid_login = execute_login(argv[1], argv[2]);
        waitpid(pid_login, NULL, 0);
        exit(0);
    }
}
