#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define TAILLE_TAMPON 1024

int main(int argc, char *argv[]) {
    // Vérifie le nombre de paramètres
    if (argc != 3) {
        fprintf(stderr, "Utilisation: %s <fichier_source> <fichier_dest>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ouvre le fichier source (f1) en mode lecture
    int fd_source = open(argv[1], O_RDONLY);
    if (fd_source == -1) {
        perror("Erreur d'ouverture du fichier source"); // Affiche l'erreur si ça ne marche pas
        return EXIT_FAILURE; // Sortie avec une erreur
    }

    // Ouvre le fichier de destination (f2) pour écrire + crée si il n'existe pas, vide s'il existe
    int fd_dest = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dest == -1) {
        perror("Erreur de création du fichier destination");
        close(fd_source);
        return EXIT_FAILURE;
    }

    char tampon[TAILLE_TAMPON];
    ssize_t lu, ecrit;

    // Lire et écrire jusqu'à la fin
    while ((lu = read(fd_source, tampon, TAILLE_TAMPON)) > 0) {
        // Écrit dans le fichier de destination
        ecrit = write(fd_dest, tampon, lu);
        if (ecrit != lu) {
            perror("Erreur d'écriture dans le fichier destination");
            close(fd_source);
            close(fd_dest);
            return EXIT_FAILURE;
        }
    }

    if (lu == -1) {
        perror("Erreur de lecture du fichier source");
    }

    close(fd_source);
    close(fd_dest);

    printf("Fichier copié avec succès.\n");

    return EXIT_SUCCESS;
}