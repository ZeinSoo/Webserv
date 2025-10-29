# Webserv

Serveur HTTP implémenté en C++98 : gestion multi-ports avec epoll, configuration type Nginx, support CGI (Python/PHP), upload de fichiers, et pages d'erreur personnalisées.

## Prérequis
- Compilateur C++ (g++/clang++)
- Make
- PHP et Python (pour les scripts CGI)

## Démarrage rapide
1) Compiler le projet
```bash
make
```

2) Lancer le serveur (configuration par défaut)
```bash
./webserv
```
Ou avec un fichier de configuration personnalisé :
```bash
./webserv config.conf
```

3) Accéder au serveur
- URL: http://localhost:8080
- URL alternative: http://localhost:8090

## Commandes Make disponibles
- `make` ou `make all`: Compiler le projet
- `make clean`: Supprimer les fichiers objets
- `make fclean`: Supprimer les fichiers objets et l'exécutable
- `make re`: Recompiler complètement le projet

## Architecture et fonctionnalités
Le serveur web implémente les fonctionnalités suivantes :
- **Multi-serveurs virtuels** : plusieurs configurations de serveurs avec ports et server_name distincts
- **Gestion des requêtes HTTP** : GET, POST, DELETE
- **CGI** : exécution de scripts Python et PHP
- **Upload de fichiers** : téléversement dans des répertoires configurables
- **Directory listing** : liste des fichiers d'un répertoire
- **Redirections** : redirections HTTP configurables
- **Pages d'erreur personnalisées** : 400, 403, 404, 405, 413, 414, 415, 500, 504, 505
- **Configuration avancée** : limite de taille des requêtes, méthodes autorisées par location

## Structure de la configuration
Le fichier de configuration (`default.conf`) permet de définir :
- `server_name` : nom du serveur virtuel
- `listen` : port(s) d'écoute
- `root` : racine des fichiers statiques
- `client_max_body_size` : taille maximale du corps de la requête
- `error_page` : pages d'erreur personnalisées
- `location` : routes avec configuration spécifique
  - `root` : racine pour cette location
  - `index` : fichiers index par défaut
  - `allow_methods` : méthodes HTTP autorisées
  - `directory_listing` : activation du listing de répertoire
  - `upload_dir` : répertoire de destination pour les uploads
  - `cgi_pass` : script CGI à exécuter
  - `redirect` : URL de redirection

## Points d'accès (configuration par défaut)
- Page d'accueil : `http://localhost:8080/`
- Upload de fichiers : `http://localhost:8080/root/cgi-bin/`
- Liste des fichiers uploadés : `http://localhost:8080/root/cgi-bin/uploads/`
- Redirection Google : `http://localhost:8080/google/` (redirige vers google.com)
- Serveur secondaire : `http://127.0.0.2:9090/`

## Exemples de configuration
### Serveur simple
```
server {
    server_name localhost;
    listen 8080;
    root ./root/;
    
    location / {
        index index.html;
        allow_methods GET POST;
    }
}
```

### Location avec CGI
```
location /cgi-bin/ {
    allow_methods GET POST;
    cgi_pass cgi-bin/script.py;
}
```

### Location avec upload
```
location /upload/ {
    allow_methods GET POST DELETE;
    upload_dir /root/uploads/;
    directory_listing on;
}
```

## Fichiers de test
- `test.conf` : configuration de test alternative
- `siege.conf` : configuration pour tests de charge avec Siege

## Notes techniques
- Standard C++98
- Utilisation d'epoll pour la gestion des événements I/O
- Architecture non-bloquante
- Parsing de configuration inspiré de Nginx
- Support complet des requêtes HTTP/1.1
