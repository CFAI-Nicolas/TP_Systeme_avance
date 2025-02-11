#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

void creer_fils(char *argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Erreur lors de la création du fils");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        printf("Fils : PID = %d, Père = %d\n", getpid(), getppid());
        close(STDOUT_FILENO);
        
        // Création du répertoire relatif s'il n'existe pas
        const char *dossier = "./logs/";
        struct stat st = {0};

        // Vérifie si le répertoire existe
        if (stat(dossier, &st) == -1) {
            printf("Création du dossier '%s'\n", dossier);
            if (mkdir(dossier, 0700) == -1) {
                perror("Erreur lors de la création du dossier logs");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Le dossier '%s' existe déjà.\n", dossier);
        }

        // Nom du fichier temporaire à créer dans le répertoire logs
        char fichier_temp[50];
        snprintf(fichier_temp, sizeof(fichier_temp), "%sproc-exerciseXXXXXX", dossier);
        int fd = mkstemp(fichier_temp);
        
        if (fd == -1) {
            perror("Erreur lors de la création du fichier temporaire");
            exit(EXIT_FAILURE);
        }
        
        printf("Fichier temporaire ouvert avec le descripteur : %d\n", fd);
        printf("Nom du fichier temporaire : %s\n", fichier_temp);
        
        // Redirige STDOUT vers le fichier temporaire
        dup2(fd, STDOUT_FILENO);
        close(fd);

        execvp(argv[1], &argv[1]);
        
        perror("Erreur lors de l'exécution de exec");
        exit(EXIT_FAILURE);
    } else { 
        int statut;
        printf("Père : PID du fils = %d\n", pid);
        waitpid(pid, &statut, 0);
        printf("Le fils s'est arrêté avec le code : %d\n", WEXITSTATUS(statut));
        printf("C'est tout, les amis !\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <commande>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    creer_fils(argv);
    return EXIT_SUCCESS;
}
