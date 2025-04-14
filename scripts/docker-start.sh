#!/bin/sh

SDK_KEY=$1 docker compose -f docker/docker-compose-local.yml -p dji up -d
