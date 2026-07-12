#!/bin/bash
set -e

# Docker entrypoint script for the open.mp server

echo "Starting EXP Gaming Cops And Robbers (open.mp) Server..."

# Check if required files exist
if [ ! -f "omp-server" ]; then
    echo "ERROR: omp-server binary not found!"
    exit 1
fi

# Check if gamemode is compiled
if [ ! -f "gamemodes/CnR.amx" ]; then
    echo "WARNING: gamemodes/CnR.amx not found. Server may fail to start."
fi

# Create necessary directories
mkdir -p logs saves

# Check if config.json exists, otherwise fall back to server.cfg
if [ -f "config.json" ]; then
    echo "Using config.json for configuration"
    exec /samp/omp-server --config-path /samp/config.json "$@"
elif [ -f "server.cfg" ]; then
    echo "Using server.cfg for configuration"
    exec /samp/omp-server "$@"
else
    echo "ERROR: No configuration file found (config.json or server.cfg)"
    exit 1
fi
