#!/bin/bash
#
# Interroge l'API Supervisor et renvoie, au format JSON, les add-ons installés
# qui ne tournent pas depuis plus de DEBOUNCE secondes. Destiné à un capteur
# command_line : contrairement aux entités « en cours d'exécution » fournies
# par l'intégration Supervisor, cette source n'est pas mise en cache et reflète
# l'état réel du conteneur à chaque appel.
#
# Sortie : {"count": 1, "addons": ["45df7312_zigbee2mqtt"],
#           "names": {"45df7312_zigbee2mqtt": "Zigbee2MQTT"}}

set -uo pipefail

# Un add-on doit être vu arrêté pendant au moins ce délai avant d'être signalé.
DEBOUNCE=60

# Add-ons jamais surveillés. Let's Encrypt est un « one-shot » : il est lancé
# chaque nuit, renouvelle le certificat puis s'arrête. Il est donc arrêté en
# permanence, ce qui n'est pas une panne.
EXCLUDED='["core_letsencrypt"]'

# Horodatage de première détection, par add-on. Sous /tmp volontairement :
# après un redémarrage de Home Assistant tout est réarmé, ce qui évite
# d'alerter sur un état antérieur au démarrage.
STATE_FILE=/tmp/addons-offline.state

TIMEOUT=10

fail() {
	echo "$1" >&2
	exit 1
}

[ -n "${SUPERVISOR_TOKEN:-}" ] || fail "SUPERVISOR_TOKEN absent de l'environnement"

# Le renommage add-ons -> apps de HA 2026 n'a pas touché l'API Supervisor,
# mais on tente les deux points d'entrée par précaution.
RESPONSE=""
for endpoint in addons apps; do
	RESPONSE=$(curl -f -s -m "$TIMEOUT" \
		-H "Authorization: Bearer $SUPERVISOR_TOKEN" \
		"http://supervisor/$endpoint") && break
	RESPONSE=""
done
[ -n "$RESPONSE" ] || fail "API Supervisor injoignable"

STOPPED=$(jq -ec --argjson excluded "$EXCLUDED" '
	[ (.data.addons // .data.apps // [])[]
	  | select(.state != "started")
	  | select(.slug as $s | $excluded | index($s) | not)
	  | {slug, name} ]' <<<"$RESPONSE") || fail "Réponse Supervisor illisible"

NOW=$(date +%s)
touch "$STATE_FILE"

# Reprend la date de première détection déjà connue, sinon horodate maintenant.
# Les add-ons repartis disparaissent du fichier : leur compteur est remis à zéro.
TRACKED=$(jq -Rn --argjson stopped "$STOPPED" --argjson now "$NOW" '
	([inputs | split(" ") | select(length == 2) | {(.[0]): (.[1] | tonumber)}] | add // {}) as $seen
	| [ $stopped[] | . + {since: ($seen[.slug] // $now)} ]' "$STATE_FILE") \
	|| fail "État local illisible"

jq -r '.[] | "\(.slug) \(.since)"' <<<"$TRACKED" > "$STATE_FILE"

jq -c --argjson now "$NOW" --argjson debounce "$DEBOUNCE" '
	map(select($now - .since >= $debounce))
	| { count: length,
	    addons: ([.[].slug] | sort),
	    names: (map({key: .slug, value: .name}) | from_entries) }' <<<"$TRACKED"
