#!/bin/bash

# Funcion para pre-commit hook
setup_pre_commit_hook(){
    HOOK_FILE=".git/hooks/pre-commit"

    if [ -f "$HOOK_FILE"]; then
        echo "A pre-commit hook already exits. Skipping setup"
    else 
        echo "Setting up pre-commit hook ..."
        cp .git/hooks/pre-commit.sample "$HOOK_FILE"
        cat > "$HOOK_FILE" << EOF
#!/bin/bash
RED="\033[0;31m"
GREEN="\033[0;32m"
NC="\033[0m"


cd utils/

echo -e "${GREEN} ✦✦ Ejecutando utils tests ✦✦ ${NC}"
make test

if [ $? -eq 0 ]; then
	echo -e "${GREEN} ✦✦ Test ejecutados correctamente!! ✦✦ ${NC}"
else
	echo -e "${RED} Error: Los test fallaron ${NC}"
	exit 1
fi
EOF

        chmod +x "$HOOK_FILE"
        echo "Pre-commit  hook succesfully set up"
    fi
}


setup_hooks(){
    setup_pre_commit_hook
}

setup_hooks
