MiniShell

MiniShell est un interpréteur de commandes minimaliste écrit en C, permettant d'exécuter des commandes Unix de base.

🚀 Fonctionnalités

📌 Exécution des commandes système (ls, pwd, whoami, etc.).

💜 Historique des commandes (history).

🏠 Commandes internes (cd, pwd, exit).

🛠 Mode batch (./my_shell -c "commande").

✅ Gestion des opérateurs (;, &&, ||).

👥 Installation

1️⃣ Compiler le projet

Assurez-vous d'avoir GCC installé, puis utilisez make :

make

2️⃣ Lancer le shell interactif

./my_shell

3️⃣ Exécuter une commande en mode batch

./my_shell -c "ls -l"

📌 Commandes internes

Commande

Description

cd <dir>

Change le répertoire actuel

pwd

Affiche le répertoire courant

exit

Quitte le shell

history

Affiche l'historique des commandes

🛠 Dépendances

GCC pour la compilation (sudo apt install gcc).

Make pour automatiser la compilation (sudo apt install make).

📌 Structure du projet

├── minishell/
│   ├── main.c          # Fichier principal du shell
│   ├── functions.h     # Déclarations des fonctions
│   ├── functions.c     # Implémentation des fonctions
│   ├── typedef.h       # Définition des structures
│   ├── Makefile        # Fichier pour compiler le projet
│   ├── README.md       # Documentation
│   ├── history.txt     # Historique des commandes

🔧 Nettoyer et recompiler le projet

make clean
make

📌 Auteur

Maxime


