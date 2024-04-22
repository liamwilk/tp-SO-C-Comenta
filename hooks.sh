#!/bin/bash

# Function to set up pre-commit hook
setup_pre_commit_hook(){
    HOOK_FILE=".git/hooks/pre-commit"

    # Remove existing pre-commit hook file if it exists
    if [ -f "$HOOK_FILE" ]; then
        rm "$HOOK_FILE"
    fi

    echo "Inicializando pre-commit hook ..."
    cp .git/hooks/pre-commit.sample "$HOOK_FILE"
    cat > "$HOOK_FILE" << EOF
#!/bin/bash

echo -e "\033[0;32mInicializando pre-commit hook ...\033[0m"


run_tests() {
    local module=\$1
    echo -e "\033[0;32m ✦✦ Ejecutando tests para \$module ✦✦ \033[0m"
    cd "\$module"
    make test
    cd ..
}

# Se corren los tests para cada directorio
run_tests "memoria"
run_tests "cpu"
run_tests "kernel"
run_tests "entradasalida"

if [ \$? -eq 0 ]; then
    echo -e "\033[0;32m ✦✦ Todos los tests ejecutados correctamente!! ✦✦ \033[0m"
else
    echo -e "\033[0;31m Error: Al menos un test falló \033[0m"
    exit 1
fi
EOF

    chmod +x "$HOOK_FILE"
    echo "Pre-commit hook successfully set up"
}

setup_hooks(){
    setup_pre_commit_hook
}

setup_hooks
