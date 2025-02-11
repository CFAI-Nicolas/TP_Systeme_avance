#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

void erreur_fatale(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    int tube[2];
    pid_t pid1, pid2;
    int status_grep = 0;

    if (pipe(tube) == -1) {
        erreur_fatale("Erreur lors de la création du pipe");
    }

    // Premier fork pour exécuter "ps eaux"
    pid1 = fork();
    if (pid1 < 0) {
        erreur_fatale("Erreur lors du fork pour ps eaux");
    }
    if (pid1 == 0) {

        close(tube[0]);

        // Redirige la sortie standard (STDOUT) vers l'extrémité d'écriture du pipe
        if (dup2(tube[1], STDOUT_FILENO) == -1) {
            erreur_fatale("Erreur lors de la redirection de STDOUT pour ps");
        }
        close(tube[1]);

        // Exécute ps eaux
        execlp("ps", "ps", "eaux", NULL);
        erreur_fatale("Erreur lors de l'exécution de ps eaux");
    }

    // Deuxième fork pour exécuter "grep \"^root \""
    pid2 = fork();
    if (pid2 < 0) {
        erreur_fatale("Erreur lors du fork pour grep");
    }
    if (pid2 == 0) {

        close(tube[1]);

        // Redirige l'entrée standard (STDIN) vers l'extrémité de lecture du pipe
        if (dup2(tube[0], STDIN_FILENO) == -1) {
            erreur_fatale("Erreur lors de la redirection de STDIN pour grep");
        }
        close(tube[0]); // Fermeture du descripteur original

        // Redirige la sortie de grep vers /dev/null
        int devnull_fd = open("/dev/null", O_WRONLY);
        if (devnull_fd == -1) {
            erreur_fatale("Erreur lors de l'ouverture de /dev/null");
        }
        if (dup2(devnull_fd, STDOUT_FILENO) == -1) {
            erreur_fatale("Erreur lors de la redirection de STDOUT pour grep vers /dev/null");
        }
        close(devnull_fd);

        execlp("grep", "grep", "^root ", NULL);
        erreur_fatale("Erreur lors de l'exécution de grep");
    }

    // Processus père
    // Ferme les deux extrémités du pipe, plus besoin dans le père
    close(tube[0]);
    close(tube[1]);

    wait(NULL); 

    int status;
    waitpid(pid2, &status, 0);
    if (WIFEXITED(status)) {
        status_grep = WEXITSTATUS(status);
    }

    if (status_grep == 0) {
        const char *msg = "root est connecté\n";
        write(STDOUT_FILENO, msg, strlen(msg));
    }

    return EXIT_SUCCESS;
}
