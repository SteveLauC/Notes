#include <stdio.h>
#include <unistd.h>
   int main() {
        printf("Going to fork myself\n");
        pid_t pid = fork();
        printf("my pid is %d, fork() says %d\n", getpid(), pid);
}
