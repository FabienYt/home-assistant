# Configuration Home Assistant

[![Home Assistant](https://img.shields.io/badge/Home%20Assistant-2026.8.2-41BDF5?logo=home-assistant&logoColor=white)](https://www.home-assistant.io)
[![Licence MIT](https://img.shields.io/badge/Licence-MIT-blue)](LICENSE.md)

## À propos

La configuration Home Assistant qui pilote la domotique de la maison : entités,
automatisations, notifications et firmwares ESPHome des appareils faits maison. Quand
la lumière s'allume toute seule, ça vient d'ici. Quand elle s'allume à 3 h du matin,
aussi.

Le dépôt est public, mais il ne contient que ce qui est écrit à la main. Tout ce que
Home Assistant génère à l'exécution est exclu comme : base de données, `.storage`, journaux, secrets...

La structure s'inspire de celle proposée par
[Frank Nijhof](https://github.com/frenck/home-assistant-config), qui a eu la bonne idée
de réfléchir à tout ça avant nous.

## Organisation

```
configuration.yaml              Amorce : allowlist, auth_providers, packages
integrations/                   Une intégration par fichier (packages HA)
entities/                       Entités écrites à la main, une par fichier
├── command_line/               Capteurs alimentés par un script shell
├── input_booleans/             Interrupteurs virtuels
├── sql/                        Capteurs issus d'une requête sur la base du recorder
└── templates/                  Capteurs et binary_sensors calculés
automations.yaml                Automatisations gérées depuis l'interface
automations/                    Automatisations écrites à la main
scripts.yaml                    Scripts gérés depuis l'interface
scripts/                        Scripts écrits à la main
scenes.yaml                     Scènes
rest_commands/                  Appels HTTP sortants
blueprints/                     Blueprints d'automatisations, scripts et templates
esphome/                        Firmwares des appareils (voir plus bas)
lldap-ha-auth.sh                Authentification des utilisateurs via LLDAP
supervisor-addons-offline.sh    Add-ons arrêtés, vus depuis l'API Supervisor
```

`configuration.yaml` ne sert qu'à amorcer le système : il déclare
`packages: !include_dir_named integrations` et délègue tout le reste aux
[packages](https://www.home-assistant.io/docs/configuration/packages/).

Chaque fichier de `integrations/` configure une seule intégration et se contente, le
plus souvent, de rediriger vers le dossier thématique correspondant — par exemple
`template.yaml` vers `entities/templates/`, `rest_command.yaml` vers `rest_commands/`.
Ajouter une entité revient donc à déposer un fichier dans le bon dossier, sans toucher
au reste : pas de `configuration.yaml` de 2 000 lignes dans lequel on part à la chasse
à l'indentation fautive. Les fichiers d'entités sont nommés `<domaine>.<objet>.yaml`,
par exemple `entities/templates/binary_sensor.ev_charging.yaml`.

Les notifications passent toutes par le script
[`scripts/notification_management.yaml`](scripts/notification_management.yaml), qui
envoie le message via [Apprise](https://github.com/caronc/apprise), réessaie trois fois
à dix secondes d'écart, et se rabat sur une notification persistante dans l'interface
si Apprise fait toujours la sourde oreille. La domotique a le droit de tomber en panne,
pas de le garder pour elle.

## ESPHome

Chaque appareil a un fichier d'entrée à la racine de `esphome/`, réduit à ses
`substitutions` (nom, clé de chiffrement de l'API…) et à un `packages` qui inclut son
modèle. Toute la logique est factorisée ailleurs :

```
esphome/
├── <appareil>.yaml             Point d'entrée : substitutions + packages
├── devices/                    Un modèle d'appareil complet
├── boards/                     Cartes (ESP32-C3, Olimex POE ISO, Shelly gen1…)
├── components/                 Briques réseau (Wi-Fi…)
├── common/                     Inclus partout : API, OTA, logger, temps, safe mode
├── binary_sensors/             Capteurs binaires réutilisables
├── sensors/                    Capteurs réutilisables
├── text_sensors/               Capteurs texte réutilisables
└── buttons/                    Boutons réutilisables (redémarrage, safe mode)
```

Résultat : ajouter un appareil qui reprend un modèle existant tient en une dizaine de
lignes. Le copier-coller de 300 lignes de YAML entre deux fichiers, c'était l'ancienne
vie.

## Secrets

`secrets.yaml` n'est **pas** versionné, et tout le monde s'en porte mieux. Il faut donc
le recréer à la restauration, avec les clés suivantes :

| Famille       | Usage                                                     |
| ------------- | --------------------------------------------------------- |
| `lldap_*`     | URL du serveur LLDAP pour l'authentification              |
| `apprise_*`   | URL et identifiants de la passerelle de notifications      |
| `esphome_*`   | Wi-Fi, OTA, interface web et clés d'API des appareils      |

Seule exception versionnée : `esphome/secrets.yaml`, qui ne contient aucune valeur mais
une simple redirection `<<: !include ../secrets.yaml`. Il a l'air inutile, il ne l'est
pas : sans lui, les compilations ESPHome refusent poliment de démarrer après une
restauration.

## Licence

[MIT](LICENSE.md) — Copyright (c) 2022-2026 FabienYt.

Copiez, modifiez, inspirez-vous. Si votre salon se met à clignoter en pleine nuit,
reportez-vous à la clause « AS IS ».
