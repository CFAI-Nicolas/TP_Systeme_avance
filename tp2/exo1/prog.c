#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void gestion_erreure(const char *mesage) {
    perror(mesage);
    exit(EXIT_FAILURE);
}

void cree_fils() {
    pid_t pid = getpid();
    if (pid == -1) {
        gestion_erreure("Erreure pour récuperer le PID du fils");
    }

    pid_t pid_pere = getppid();
    if (pid_pere == -1) {
        gestion_erreure("Erreure pour récuperer le PID du pere");
    }

    printf("Fils : PID = %d, Pere = %d\n", pid, pid_pere);

    int code_reteur = pid % 10; // Code de reteur basé sur le derniere chiffre du PID
    exit(code_reteur);
}

void atendre_fils(pid_t pid_fils) {
    int statut;

    printf("Pere : PID du fils = %d\n", pid_fils);

    if (waitpid(pid_fils, &statut, 0) == -1) {
        gestion_erreure("Erreure pour atendre le fils avec waitpid");
    }

    if (WIFEXITED(statut)) {
        int code_reteur_fils = WEXITSTATUS(statut);
        printf("Le fils s'est arreter avec code : %d\n", code_reteur_fils);
    } else if (WIFSIGNALED(statut)) {
        int signal = WTERMSIG(statut);
        printf("Le fils arreter par signal : %d\n", signal);
    } else {
        printf("Le fils ne s'est pas arreter normalment.\n");
    }
}

int main() {
    pid_t pid_fils = fork();

    if (pid_fils == -1) {
        gestion_erreure("Erreure de creer le fils avec fork");
    }

    if (pid_fils == 0) {
        cree_fils(); // Appel pour le code du fils
    } else {
        atendre_fils(pid_fils); // Appel pour le code du pere
    }

    return EXIT_SUCCESS;
}
