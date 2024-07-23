#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/mman.h>
#include <vector>

#define SIZE_STACK 65536

 


class Registro {

public:
    std::string nome;
    int id;
    double saldo;

    void mostrarSaldo() const {
        std::cout << "A conta de " << nome << " possui R$" << saldo << ".\n";
    }

};

class Cadastros{
    private:
        std::vector<Registro> contas;

    public:
        void criarConta(const std::string& nome, double saldo, int id) {
            Registro novaConta;
            novaConta.nome = nome;
            novaConta.saldo = saldo;
            novaConta.id = id;
            contas.push_back(novaConta);
    }
    
        Registro lerConta(int id) {
        for (const auto& conta : contas) {
            if (conta.id == id) {
                return conta;
            }
        }
        std::cout << "Conta com ID " << id << " não encontrada.\n";
        return Registro(); // Retorna um objeto Registro vazio
    }
        void atualizarSaldo(double valor, Registro conta) {
            std::cout << "Iniciando operação [R$" << valor << "] (saldo atual R$" << conta.saldo << ")\n";
            usleep(10000);
            conta.saldo += valor;
            std::cout << "Operação terminada [R$" << valor << "] (saldo atual R$" << conta.saldo << ")\n";
    }

};


    int threadDeposito(void* arg) {
        Cadastros* cadastro = static_cast<Cadastros*>(arg);
        cadastro->atualizarSaldo(100,cadastro->lerConta(1));
        return 0;
    }

    int threadSaque(void* arg) {
        Cadastros* cadastro = static_cast<Cadastros*>(arg);
        cadastro->atualizarSaldo(-200,cadastro->lerConta(1));
        return 0;
    }

int main() {
    Cadastros cadastro;
    void *stack1, *stack2;
    int pid1, pid2;

 
    cadastro.criarConta("Joao", 500, 1);
    cadastro.lerConta(1).mostrarSaldo();

    if ((stack1 = malloc(SIZE_STACK)) == nullptr) {
        perror("Erro ao alocar stack1");
        exit(1);
    }

    if ((stack2 = malloc(SIZE_STACK)) == nullptr) {
        perror("Erro ao alocar stack2");
        free(stack1);
        exit(1);
    }

    pid1 = clone(threadDeposito, static_cast<char*>(stack1) + SIZE_STACK, CLONE_VM | SIGCHLD, &cadastro);
    pid2 = clone(threadSaque, static_cast<char*>(stack2) + SIZE_STACK, CLONE_VM | SIGCHLD, &cadastro);

    if (pid1 == -1 || pid2 == -1) {
        perror("Erro ao criar threads");
        free(stack1);
        free(stack2);
        exit(1);
    }

    waitpid(pid1, 0, 0);
    waitpid(pid2, 0, 0);


    free(stack1);
    free(stack2);

    return 0;
}
