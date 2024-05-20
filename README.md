# Rapport de Projet : Shell Personnalisé avec Contrôle de Processus

## Présentation du Projet

Nous sommes ravis de vous présenter notre shell nommé `jsh`, développé dans le cadre de notre L3 Informatique. Cet interpréteur de commandes interactif met l'accent sur un contrôle de processus robuste et offre un ensemble complet de fonctionnalités pour améliorer l'expérience utilisateur.

## Principales Fonctionnalités

### Contrôle de Processus

- **Gestion des Jobs :** `jsh` excelle dans la gestion de plusieurs processus simultanément, que ce soit en avant-plan ou en arrière-plan. Il fournit des mises à jour en temps réel sur les jobs en arrière-plan.
- **États des Jobs :** Notre shell gère les états des jobs, notamment Running, Stopped, Detached, Killed et Done.

### Commandes Externes

- `jsh` exécute de manière transparente toutes les commandes externes, en tenant compte du `PATH` système pour l'accessibilité.

### Commandes Internes

- Des commandes internes telles que `pwd`, `cd`, `?` (pour voir la valeur de retour de la dernière commande) et `exit` sont intégrées pour la commodité de l'utilisateur.
- Les commandes internes renvoient 0 en cas de succès et 1 en cas d'échec.

### Gestion de la Ligne de Commande

- En mode interactif, `jsh` interprète efficacement les commandes de l'utilisateur. L'invite de commande affiche des informations pertinentes sur le nombre de jobs et le répertoire actuel.
- Nous avons intégré `readline` pour l'édition et l'historique de la ligne de commande.

### Redirections

- `jsh` gère les redirections d'entrée/sortie, l'ajout à des fichiers et les pipelines, en utilisant des symboles tels que `<`, `>` et `|`.

### Gestion des Signaux

- Le shell gère les signaux tel que `SIGINT`, `SIGTERM`, `SIGTTIN`, `SIGQUIT`, `SIGTTOU` et `SIGTSTP`.

## Structure du Projet

Notre projet est organisé dans un dépôt `git`, avec un `Makefile` complet et un document `ARCHITECTURE.md` détaillant les choix architecturaux, les structures de données et les algorithmes.

## Le groupe de travail pour `jsh`

- **ABOUNAIM Elias**
- **DIALLO Amadou Oury**
- **POSTOLACHI Marin**

## Conclusion

En conclusion, `jsh` est le fruit de nos efforts collectifs, et nous sommes convaincus qu'il répondra à vos attentes.
