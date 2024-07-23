#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#define STACK_SIZE 65536 // Valor padrão para pilha, potência de 2 otimiza

// Estrutura em C, parece com uma tabela
typedef struct {
    char nome[50];
    double saldo;
} Registro;

Registro registro[500]; // 500 elementos que podem ser populados

void criar_conta(const char *nome, int numero, float saldo) {
    /*
    A função stpcpy é usada em vez do operador de atribuição (=) para copiar strings
    porque é mais segura, se usar = pode estourar o buffer em arrays de char
    */
    strncpy(registro[numero].nome, nome, sizeof(registro[numero].nome) - 1);
    registro[numero].nome[sizeof(registro[numero].nome) - 1] = '\0';
    registro[numero].saldo = saldo;
}

void mostrar_saldo(int numero) {
    printf("A conta de %s possui R$%.2f.\n", registro[numero].nome, registro[numero].saldo);
}

Registro le_registro(int conta) {
    return registro[conta];
}

void gravar_registro(Registro reg, int conta) {
    registro[conta] = reg;
}

void atualiza_saldo(double valor, int conta) {
    Registro reg;
    reg = le_registro(conta);
    printf("Iniciando operação [%.2f] (saldo atual R$%.2f)\n", valor, reg.saldo);
    usleep(10000);
    reg.saldo += valor;
    gravar_registro(reg, conta);
    printf("Operação terminada [%.2f] (saldo atual R$%.2f)\n", valor, reg.saldo);
}

int thread_deposito(void *arg) {
    atualiza_saldo(100, 1);
    return 0;
}

int thread_saque(void *arg) {
    atualiza_saldo(-200, 1);
    return 0;
}

int main() {
    criar_conta("Joao", 1, 500);
    printf("Saldo antes das operações: ");
    mostrar_saldo(1);

    // Pedaço de memória compartilhada
    void* shared_memory = mmap(
        nullptr, // Aloca no espaço definido pelo SO
        sizeof(char) * 1024, // Tamanho da memória a ser alocada
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, // Ver web
        -1,
        0
    );

    if (shared_memory == MAP_FAILED) {
        perror("Erro ao alocar memória compartilhada");
        return 1;
    }

    int clone_flags = CLONE_VM | SIGCHLD;
    void* stack1 = malloc(STACK_SIZE);
    void* stack2 = malloc(STACK_SIZE);

    if (stack1 == nullptr || stack2 == nullptr) {
        perror("Erro ao alocar pilha");
        return 1;
    }

    int pid1 = clone(thread_deposito, static_cast<char*>(stack1) + STACK_SIZE, clone_flags, shared_memory);
    int pid2 = clone(thread_saque, static_cast<char*>(stack2) + STACK_SIZE, clone_flags, shared_memory);

    if (pid1 == -1 || pid2 == -1) {
        perror("Erro ao criar threads");
        return 1;
    }

    // Wait for threads to finish (optional)
    waitpid(pid1, 0, 0);
    waitpid(pid2, 0, 0);

    // Cleanup
    free(stack1);
    free(stack2);

    printf("Saldo após as operações: ");
    mostrar_saldo(1);

    return 0;
}
