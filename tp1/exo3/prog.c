#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define FICHIER_LOG "log.txt"

// Fonction pour écrire dans le fichier de log
void ecrire_dans_log(int fd_log, const char *message) {
    write(fd_log, message, strlen(message));
}

// Fonction pour afficher les informations d'un fichier et les écrire dans le log
void afficher_infos_fichier(const char *chemin, const char *nom_fichier, int fd_log) {
    struct stat infos_fichier;
    char chemin_complet[1024];

    snprintf(chemin_complet, sizeof(chemin_complet), "%s/%s", chemin, nom_fichier);

    // Vérification : stat du fichier
    if (stat(chemin_complet, &infos_fichier) == -1) {
        char erreur_msg[256];
        snprintf(erreur_msg, sizeof(erreur_msg), "Erreur lors de l'obtention des infos pour le fichier %s: %s\n", nom_fichier, strerror(errno));
        ecrire_dans_log(fd_log, erreur_msg);
        return;
    }

    char permissions[11];
    permissions[0] = (S_ISDIR(infos_fichier.st_mode)) ? 'd' : '-';
    permissions[1] = (infos_fichier.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (infos_fichier.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (infos_fichier.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (infos_fichier.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (infos_fichier.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (infos_fichier.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (infos_fichier.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (infos_fichier.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (infos_fichier.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';

    struct passwd *proprietaire = getpwuid(infos_fichier.st_uid);
    struct group *groupe = getgrgid(infos_fichier.st_gid);

    char tampon_temps[80];
    struct tm *tm_infos = localtime(&infos_fichier.st_mtime);
    strftime(tampon_temps, sizeof(tampon_temps), "%d%m%y @ %Hh%M", tm_infos);

    char resultat[256];
    snprintf(resultat, sizeof(resultat), "%s – %s %s : %s – %lld – %s\n", 
             nom_fichier, 
             permissions, 
             proprietaire->pw_name, 
             groupe->gr_name, 
             (long long)infos_fichier.st_size, 
             tampon_temps);

    // Afficher sur la console
    printf("%s", resultat);

    // Écrire dans le fichier de log
    ecrire_dans_log(fd_log, resultat);
}

// Fonction pour lister les fichiers d'un répertoire et les inscrire dans le log
void lister_repertoire(const char *chemin, int fd_log) {
    DIR *repertoire;
    struct dirent *entree;

    if ((repertoire = opendir(chemin)) == NULL) {
        char erreur_msg[256];
        snprintf(erreur_msg, sizeof(erreur_msg), "Erreur lors de l'ouverture du répertoire %s: %s\n", chemin, strerror(errno));
        ecrire_dans_log(fd_log, erreur_msg);
        return;
    }

    while ((entree = readdir(repertoire)) != NULL) {
        if (strcmp(entree->d_name, ".") != 0 && strcmp(entree->d_name, "..") != 0) {
            afficher_infos_fichier(chemin, entree->d_name, fd_log);
        }
    }

    if (errno != 0) {
        char erreur_msg[256];
        snprintf(erreur_msg, sizeof(erreur_msg), "Erreur lors de la lecture du répertoire %s: %s\n", chemin, strerror(errno));
        ecrire_dans_log(fd_log, erreur_msg);
    }

    if (closedir(repertoire) == -1) {
        char erreur_msg[256];
        snprintf(erreur_msg, sizeof(erreur_msg), "Erreur lors de la fermeture du répertoire %s: %s\n", chemin, strerror(errno));
        ecrire_dans_log(fd_log, erreur_msg);
    }
}

int main(int argc, char *argv[]) {
    const char *chemin;

    // Vérification du nombre d'arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <chemin_du_repertoire>\n", argv[0]);
        return EXIT_FAILURE;
    }

    chemin = argv[1];

    struct stat infos_chemin;
    if (stat(chemin, &infos_chemin) == -1) {
        fprintf(stderr, "Erreur lors de la vérification du chemin %s: %s\n", chemin, strerror(errno));
        return EXIT_FAILURE;
    }

    if (!S_ISDIR(infos_chemin.st_mode)) {
        fprintf(stderr, "Le chemin fourni %s n'est pas un répertoire.\n", chemin);
        return EXIT_FAILURE;
    }

    // Ouvrir ou créer le fichier de log et le vider
    int fd_log = open(FICHIER_LOG, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_log == -1) {
        perror("Erreur lors de l'ouverture ou la création du fichier de log");
        return EXIT_FAILURE;
    }

    // Écrire l'en-tête dans le log
    char en_tete_log[256];
    snprintf(en_tete_log, sizeof(en_tete_log), "Log pour le répertoire : %s\n", chemin);
    ecrire_dans_log(fd_log, en_tete_log);

    // Lister les fichiers du répertoire et écrire dans le log
    lister_repertoire(chemin, fd_log);

    // Fermer le fichier de log
    if (close(fd_log) == -1) {
        perror("Erreur lors de la fermeture du fichier de log");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
