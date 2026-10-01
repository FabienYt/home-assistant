#!/bin/bash
#
# Interroge l'API Supervisor et renvoie l'état actuel de l'add-on
# Let's Encrypt (started/stopped/...), sans mise en cache.
#
# Sortie : {"state": "started"}

set -uo pipefail

SLUG=core_letsencrypt
TIMEOUT=10

fail() {
	echo "$1" >&2
	exit 1
}

[ -n "${SUPERVISOR_TOKEN:-}" ] || fail "SUPERVISOR_TOKEN absent de l'environnement"

RESPONSE=""
for endpoint in addons apps; do
	RESPONSE=$(curl -f -s -m "$TIMEOUT" \
		-H "Authorization: Bearer $SUPERVISOR_TOKEN" \
		"http://supervisor/$endpoint/$SLUG/info") && break
	RESPONSE=""
done
[ -n "$RESPONSE" ] || fail "API Supervisor injoignable"

jq -c '{state: .data.state}' <<<"$RESPONSE" || fail "Réponse Supervisor illisible"
