#!/bin/bash
set -e

# Script de instalação do oneLauncher reformulado
# Focado em garantir que o plugin apareça na lista do painel

COLOR_RED='\033[0;31m'
COLOR_GREEN='\033[0;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_NC='\033[0m'

function print_info() { echo -e "${COLOR_YELLOW}[INFO] $1${COLOR_NC}"; }
function print_success() { echo -e "${COLOR_GREEN}[OK] $1${COLOR_NC}"; }
function print_error() { echo -e "${COLOR_RED}[ERRO] $1${COLOR_NC}"; }

function main() {
    echo "=== Preparando instalação do Vexto App Launcher ==="
    
    # 1. Verificar dependências novamente por segurança
    print_info "Verificando pacotes necessários..."
    if ! pkg-config --exists libxfce4panel-2.0 libxfce4ui-2; then
        print_error "Faltam bibliotecas de desenvolvimento."
        echo "Por favor, rode: sudo apt install libxfce4panel-2.0-dev libxfce4ui-2-dev libgtk-3-dev"
        exit 1
    fi

    # 2. Limpar e Compilar
    print_info "Compilando o código..."
    make clean
    if make; then
        print_success "Compilação concluída."
    else
        print_error "Falha na compilação."
        exit 1
    fi

    # 3. Instalar (usando sudo)
    print_info "Instalando nos diretórios do sistema..."
    if sudo make install; then
        print_success "Arquivos copiados com sucesso."
    else
        print_error "Falha na instalação."
        exit 1
    fi

    # 4. Atualizar caches do sistema
    print_info "Atualizando cache de ícones e plugins..."
    sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor || true
    
    
    echo ""
    print_success "INSTALAÇÃO FINALIZADA!"
    echo "-------------------------------------------------------"
    echo "COMO ENCONTRAR O PLUGIN:"
    echo "1. Clique com o botão direito no seu PAINEL."
    echo "2. Vá em 'Painel' -> 'Adicionar novos itens'."
    echo "3. Na busca, digite: Vexto App Launcher"
    echo "4. Selecione e clique em 'Adicionar'."
    echo "-------------------------------------------------------"
    echo "Se ainda não aparecer, tente rodar este comando manualmente:"
    echo "xfce4-panel -r"
}

main
