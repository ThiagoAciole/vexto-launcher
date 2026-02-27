#!/bin/bash
set -e

# Script de desinstalação do Vexto App Launcher

COLOR_RED='\033[0;31m'
COLOR_GREEN='\033[0;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_NC='\033[0m'

function print_info() { echo -e "${COLOR_YELLOW}[INFO] $1${COLOR_NC}"; }
function print_success() { echo -e "${COLOR_GREEN}[OK] $1${COLOR_NC}"; }
function print_error() { echo -e "${COLOR_RED}[ERRO] $1${COLOR_NC}"; }

function main() {
    echo "=== Desinstalando Vexto Launcher ==="
    
    # 1. Executar o uninstall do Makefile (usando sudo)
    print_info "Removendo arquivos do sistema..."
    if sudo make uninstall; then
        print_success "Arquivos removidos com sucesso."
    else
        print_error "Falha ao remover arquivos."
        exit 1
    fi

    # 2. Atualizar caches do sistema
    print_info "Atualizando cache do sistema..."
    sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor || true
    
    # 3. Limpeza local (opcional)
    print_info "Limpando arquivos de compilação..."
    make clean

    echo ""
    print_success "DESINSTALAÇÃO CONCLUÍDA!"
    echo "-------------------------------------------------------"
    echo "O plugin foi removido do sistema."
    echo "Se ele ainda estiver aparecendo no painel, reinicie o painel com:"
    echo "xfce4-panel -r"
    echo "-------------------------------------------------------"
}

main
