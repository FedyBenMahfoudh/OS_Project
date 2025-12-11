# Plan de Développement Agile - Simulateur d'Ordonnancement

Ce document décrit le backlog du produit et la planification des sprints pour le développement du projet de simulateur d'ordonnancement.

- **Équipe :** 5 développeurs
- **Durée totale :** 4 semaines
- **Méthodologie :** Scrum
- **Sprints :** 2 sprints de 2 semaines chacun

---

## 1. Product Backlog (Carnet de Produit)

Le backlog contient l'ensemble des fonctionnalités (Epics), des récits utilisateurs (User Stories) et des tâches techniques nécessaires pour réaliser le projet. Les items sont priorisés.

| ID  | Type        | Description                                                                                                                              | Priorité |
|:----|:------------|:-----------------------------------------------------------------------------------------------------------------------------------------|:---------|
| PB-1| Epic        | **Fondations du Projet :** Mettre en place les structures de données et le parser.                                                       | **Doit être fait** |
| PB-2| Epic        | **Moteur de Simulation :** Créer le cœur du simulateur capable de gérer le temps et les processus.                                       | **Doit être fait** |
| PB-3| Epic        | **Politiques d'Ordonnancement :** Implémenter les différents algorithmes (FIFO, RR, SJF, etc.).                                           | **Doit être fait** |
| PB-4| Epic        | **Interface Utilisateur (CLI) :** Permettre à l'utilisateur de lancer et configurer la simulation.                                       | **Doit être fait** |
| PB-5| Epic        | **Sorties et Rapports :** Afficher les résultats de la simulation (logs, diagramme de Gantt, métriques).                                  | **Doit être fait** |
| PB-6| Epic        | **Tests et Validation :** Assurer la robustesse et la correction du simulateur.                                                          | **Devrait être fait** |
| PB-7| Epic        | **Documentation :** Documenter le code et l'utilisation du projet.                                                                       | **Devrait être fait** |
| PB-8| Epic        | **Fonctionnalités Avancées :** Améliorer l'interface utilisateur avec une TUI (ncurses) ou des graphiques.                                 | **Pourrait être fait** |

---

## 2. Sprint 1 : Construction des Fondations (Semaines 1-2)

**Objectif du Sprint :** À la fin de ce sprint, l'équipe disposera d'un simulateur de base fonctionnel capable de lancer une simulation non-préemptive (FIFO) à partir d'un fichier de configuration et d'afficher des logs de base.

### Tâches du Sprint 1 :

| ID     | Récit Utilisateur / Tâche                                                                                                                      | Priorité |
|:-------|:-----------------------------------------------------------------------------------------------------------------------------------------------|:---------|
| S1-T1  | En tant que développeur, je veux implémenter les structures `Process`, `Queue` et `Stack` pour avoir les briques de base du projet.             | **Haute** |
| S1-T2  | En tant qu'utilisateur, je veux définir une liste de processus dans un fichier `.conf` pour fournir les entrées de la simulation. (Parser)      | **Haute** |
| S1-T3  | En tant que développeur, je veux définir une `policy_interface.h` pour standardiser la communication entre le moteur et les politiques.        | **Haute** |
| S1-T4  | En tant que développeur, je veux un `Makefile` de base pour compiler l'ensemble du projet facilement.                                           | **Haute** |
| S1-T5  | En tant que développeur, je veux un moteur de simulation simple qui gère le temps et l'état des processus (NEW, READY, RUNNING, TERMINATED).    | **Moyenne**|
| S1-T6  | En tant qu'utilisateur, je veux exécuter la politique **FIFO** pour valider le fonctionnement du moteur non-préemptif.                           | **Moyenne**|
| S1-T7  | En tant que développeur, je veux un module `logger` simple qui affiche les événements (arrivée, début/fin) avec le temps de simulation.        | **Basse**  |
| S1-T8  | En tant que développeur, je veux commencer à mettre en place le script `test_runner.sh` avec un premier cas de test pour FIFO.                  | **Basse**  |

---

## 3. Sprint 2 : Politiques Avancées, Rapports et Finitions (Semaines 3-4)

**Objectif du Sprint :** Compléter toutes les fonctionnalités essentielles, y compris les politiques préemptives, l'affichage des résultats, et assurer que le projet est robuste, testé et documenté pour la livraison finale.

### Tâches du Sprint 2 :

| ID     | Récit Utilisateur / Tâche                                                                                                                          | Priorité |
|:-------|:---------------------------------------------------------------------------------------------------------------------------------------------------|:---------|
| S2-T1  | En tant que développeur, je veux implémenter une structure de `Heap` pour les politiques basées sur la priorité.                                    | **Haute**  |
| S2-T2  | En tant qu'utilisateur, je veux utiliser les politiques préemptives **RR** (Round Robin) et **SRT** (Shortest Remaining Time).                      | **Haute**  |
| S2-T3  | En tant que développeur, je dois mettre à jour le moteur de simulation pour qu'il gère la logique de préemption.                                    | **Haute**  |
| S2-T4  | En tant qu'utilisateur, je veux voir un **diagramme de Gantt** en ASCII et les **métriques de performance** (temps d'attente/rotation moyen) à la fin. | **Haute**  |
| S2-T5  | En tant qu'utilisateur, je veux pouvoir lancer le simulateur avec des arguments en ligne de commande (`-f`, `-p`, `-q`). (Finalisation du CLI)     | **Moyenne**|
| S2-T6  | En tant que développeur, je veux enrichir la suite de tests pour couvrir toutes les politiques et les cas aux limites (préemption, etc.).          | **Moyenne**|
| S2-T7  | En tant que développeur et utilisateur, je veux une documentation claire (`README.md`, guide d'utilisation) pour comprendre et utiliser le projet. | **Moyenne**|
| S2-T8  | **(Objectif Ambitieux / Stretch Goal)** En tant qu'utilisateur, je veux une vue interactive en temps réel de la simulation via `ncurses`.         | **Basse**  |
| S2-T9  | **(Revue et Débogage)** Allouer du temps pour la revue de code, la correction de bugs et la validation avec `valgrind`.                             | **Haute**  |

