#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Fonction pour afficher un message d'erreur et quitter
void erreur_fatale(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    int tube[2];
    pid_t pid1, pid2;
    int status_grep = 0;

    // Création du pipe
    if (pipe(tube) == -1) {
        erreur_fatale("Erreur lors de la création du pipe");
    }

    // Premier fork pour exécuter "ps eaux"
    pid1 = fork();
    if (pid1 < 0) {
        erreur_fatale("Erreur lors du fork pour ps eaux");
    }
    if (pid1 == 0) {
        // Processus fils #1 (ps)

        // Ferme l'extrémité de lecture du pipe
        close(tube[0]);

        // Redirige la sortie standard (STDOUT) vers l'extrémité d'écriture du pipe
        if (dup2(tube[1], STDOUT_FILENO) == -1) {
            erreur_fatale("Erreur lors de la redirection de STDOUT pour ps");
        }
        close(tube[1]); // On peut fermer le descripteur car dup2 l'a dupliqué

        // Exécute ps eaux
        execlp("ps", "ps", "eaux", NULL);
        // Si execlp échoue :
        erreur_fatale("Erreur lors de l'exécution de ps eaux");
    }

    // Deuxième fork pour exécuter "grep \"^root \""
    pid2 = fork();
    if (pid2 < 0) {
        erreur_fatale("Erreur lors du fork pour grep");
    }
    if (pid2 == 0) {
        // Processus fils #2 (grep)

        // Ferme l'extrémité d'écriture du pipe
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

        // Exécute grep "^root "
        execlp("grep", "grep", "^root ", NULL);
        // Si execlp échoue
        erreur_fatale("Erreur lors de l'exécution de grep");
    }

    // Processus père
    // Ferme les deux extrémités du pipe, plus besoin dans le père
    close(tube[0]);
    close(tube[1]);

    // Attend la terminaison de ps
    wait(NULL);  // On ne vérifie pas spécialement le code de sortie de ps

    // Attend la terminaison de grep et récupère son code de sortie
    int status;
    waitpid(pid2, &status, 0);
    if (WIFEXITED(status)) {
        status_grep = WEXITSTATUS(status);
    }

    // Si grep a un code de sortie 0, on affiche "root est connecté"
    if (status_grep == 0) {
        // Utilisation de write sur la sortie standard
        const char *msg = "root est connecté\n";
        write(STDOUT_FILENO, msg, strlen(msg));
    }

    return EXIT_SUCCESS;
}
