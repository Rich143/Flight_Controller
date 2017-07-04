#!/bin/bash

read -r -p "Do you want to commit this code? [y/N] " response
case $response in
    [yY][eE][sS]|[yY])
        echo "Commiting all modified files"
        git add -u
        git commit

        ;;
    *)
        echo "No"
        ;;
esac
