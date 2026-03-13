#!/bin/bash

# Exit on error
set -e

# Function to print usage
usage() {
    echo "Usage: ./run.sh [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  preview     Run the Hugo development server to preview the site locally"
    echo "  help        Show this help message"
    echo ""
}

# Check if an argument is provided
if [ $# -eq 0 ]; then
    usage
    exit 1
fi

COMMAND=$1

case "$COMMAND" in
    preview)
        echo "Starting Hugo development server..."
        cd docs
        # Check if hugo is installed
        if ! command -v hugo &> /dev/null; then
            echo "Error: hugo could not be found. Please install it first."
            exit 1
        fi
        hugo server -D
        ;;
    help)
        usage
        ;;
    *)
        echo "Error: Unknown command '$COMMAND'"
        echo ""
        usage
        exit 1
        ;;
esac
