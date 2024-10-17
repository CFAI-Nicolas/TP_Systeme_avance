#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Vérifie le nombre d'arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fichier_texte>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ouvre le fichier en lecture
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Erreur d'ouverture");
        return EXIT_FAILURE;
    }

    // Récupère la taille du fichier
    off_t taille = lseek(fd, 0, SEEK_END);
    if (taille == -1) {
        perror("Erreur lseek");
        close(fd);
        return EXIT_FAILURE;
    }

    // Lit le fichier à l'envers
    for (off_t i = taille - 1; i >= 0; i--) {
        if (lseek(fd, i, SEEK_SET) == -1) {
            perror("Erreur lseek");
            close(fd);
            return EXIT_FAILURE;
        }

        char c;
        if (read(fd, &c, 1) != 1) {
            perror("Erreur de lecture");
            close(fd);
            return EXIT_FAILURE;
        }

        write(STDOUT_FILENO, &c, 1);
    }

    close(fd);
    printf("\n");
    return EXIT_SUCCESS;
}
