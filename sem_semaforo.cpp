#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <unistd.h>
#include <cstdlib>

#define SIZE_STACK 65536

class Conta {
private:
    std::string nome;
    float saldo;

public:
    Conta(std::string nome, float saldo) : nome(nome), saldo(saldo) {}

    void sacar(float valor, const std::string& caixa) {
        if (valor > 0 && valor <= saldo) {
            saldo -= valor;
            std::cout << caixa << " realizou um saque de " << valor << ". Novo saldo: " << saldo << std::endl;
        } else {
            std::cout << caixa << " tentou sacar " << valor << ". Saldo insuficiente." << std::endl;
        }
    }

    void depositar(float valor, const std::string& caixa) {
        if (valor > 0) {
            saldo += valor;
            std::cout << caixa << " realizou um depósito de " << valor << ". Novo saldo: " << saldo << std::endl;
        } else {
            std::cout << caixa << " tentou depositar um valor inválido: " << valor << std::endl;
        }
    }

    void verSaldo() const {
        std::cout << "Nome: " << nome << ", Saldo atual: " << saldo << std::endl;
    }

    // Métodos para serem usados nas threads
    void depositarThread(float valor, const std::string& caixa) {
        depositar(valor, caixa);
    }

    void sacarThread(float valor, const std::string& caixa) {
        sacar(valor, caixa);
    }

    static int threadDeposito(void* arg) {
        Conta* conta = static_cast<Conta*>(arg);
        conta->depositarThread(100, "Carlos");
        return 0;
    }

    static int threadSaque(void* arg) {
        Conta* conta = static_cast<Conta*>(arg);
        conta->sacarThread(200, "Orlando");
        return 0;
    }
};

int main() {
    void *stack1, *stack2;
    int pid1, pid2;

    Conta conta("João", 500); // Cria uma conta com saldo inicial de 500

    conta.verSaldo();

    if ((stack1 = malloc(SIZE_STACK)) == nullptr) {
        perror("Erro ao alocar stack1");
        exit(1);
    }
    
    if ((stack2 = malloc(SIZE_STACK)) == nullptr) {
        perror("Erro ao alocar stack2");
        free(stack1); // Liberar memória alocada anteriormente
        exit(1);
    }

    // Cria as threads com clone
    pid1 = clone(Conta::threadDeposito, (char*)stack1 + SIZE_STACK, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD, &conta);
    pid2 = clone(Conta::threadSaque, (char*)stack2 + SIZE_STACK, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD, &conta);

    if (pid1 == -1 || pid2 == -1) {
        perror("Erro ao criar threads");
        free(stack1);
        free(stack2);
        exit(1);
    }

    // Espera as threads terminarem
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    conta.verSaldo();

    // Libera memória alocada
    free(stack1);
    free(stack2);

    return 0;
}
