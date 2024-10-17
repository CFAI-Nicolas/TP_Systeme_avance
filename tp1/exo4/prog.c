#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define TAILLE_TAMPON 1024

// Définition de la structure FICHIER, analogue à FILE
typedef struct {
    int descripteur_fichier; // descripteur de fichier
    char tampon[TAILLE_TAMPON]; // tampon pour bufferiser les entrées/sorties
    int position_tampon; // position courante dans le tampon
    int taille_tampon;   // taille des données valides dans le tampon (lecture)
    int mode;            // mode : 0 = lecture, 1 = écriture
    int eof;             // indicateur de fin de fichier
} FICHIER;

// Fonction analogue à fopen, pour ouvrir un fichier
FICHIER* my_open(const char *nom_fichier, const char *mode) {
    FICHIER *fichier = malloc(sizeof(FICHIER));
    if (fichier == NULL) {
        perror("Erreur de mémoire");
        return NULL;
    }

    // Mode lecture
    if (strcmp(mode, "r") == 0) {
        fichier->descripteur_fichier = open(nom_fichier, O_RDONLY);
        fichier->mode = 0;
    }
    // Mode écriture
    else if (strcmp(mode, "w") == 0) {
        fichier->descripteur_fichier = open(nom_fichier, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        fichier->mode = 1;
    } else {
        fprintf(stderr, "Mode non supporté\n");
        free(fichier);
        return NULL;
    }

    // Gestion des erreurs à l'ouverture du fichier
    if (fichier->descripteur_fichier == -1) {
        perror("Erreur à l'ouverture du fichier");
        free(fichier);
        return NULL;
    }

    fichier->position_tampon = 0;
    fichier->taille_tampon = 0;
    fichier->eof = 0;

    return fichier;
}

// Fonction analogue à fgetc, pour lire un caractère bufferisé
int my_getc(FICHIER *fichier) {
    if (fichier->mode != 0) {
        fprintf(stderr, "Fichier non ouvert en lecture\n");
        return EOF;
    }

    // Recharger le tampon si vide
    if (fichier->position_tampon >= fichier->taille_tampon) {
        fichier->taille_tampon = read(fichier->descripteur_fichier, fichier->tampon, TAILLE_TAMPON);
        fichier->position_tampon = 0;

        if (fichier->taille_tampon == 0) {
            fichier->eof = 1;
            return EOF;
        } else if (fichier->taille_tampon == -1) {
            perror("Erreur lors de la lecture");
            return EOF;
        }
    }

    return (unsigned char)fichier->tampon[fichier->position_tampon++];
}

// Fonction analogue à fputc, pour écrire un caractère bufferisé
int my_putc(int c, FICHIER *fichier) {
    if (fichier->mode != 1) {
        fprintf(stderr, "Fichier non ouvert en écriture\n");
        return EOF;
    }

    fichier->tampon[fichier->position_tampon++] = (unsigned char)c;

    // Écriture du tampon si plein
    if (fichier->position_tampon == TAILLE_TAMPON) {
        if (write(fichier->descripteur_fichier, fichier->tampon, TAILLE_TAMPON) != TAILLE_TAMPON) {
            perror("Erreur lors de l'écriture");
            return EOF;
        }
        fichier->position_tampon = 0;
    }

    return c;
}

// Fonction analogue à fclose, pour fermer le fichier
int my_close(FICHIER *fichier) {
    // Si en écriture, vider le tampon restant
    if (fichier->mode == 1 && fichier->position_tampon > 0) {
        if (write(fichier->descripteur_fichier, fichier->tampon, fichier->position_tampon) == -1) {
            perror("Erreur lors de la fermeture (écriture finale)");
            free(fichier);
            return EOF;
        }
    }

    if (close(fichier->descripteur_fichier) == -1) {
        perror("Erreur lors de la fermeture");
        free(fichier);
        return EOF;
    }

    free(fichier);
    return 0;
}

// Programme de test pour écrire et lire dans un fichier avec my_putc et my_getc
int main() {
    // Test d'écriture dans un fichier
    FICHIER *fichier = my_open("test.txt", "w");
    if (fichier == NULL) {
        return 1;
    }

    my_putc('H', fichier);
    my_putc('e', fichier);
    my_putc('l', fichier);
    my_putc('l', fichier);
    my_putc('o', fichier);
    my_putc('\n', fichier);

    my_close(fichier);

    // Test de lecture depuis un fichier
    fichier = my_open("test.txt", "r");
    if (fichier == NULL) {
        return 1;
    }

    int c;
    while ((c = my_getc(fichier)) != EOF) {
        putchar(c);
    }

    my_close(fichier);

    return 0;
}
