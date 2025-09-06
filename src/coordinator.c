#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"
#include <errno.h>
/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    if(argv !=5){
        printf("Erro: argv != 5");
        return 1;
    }
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio
    if (password_len >= 1 && password_len <= 10) 
        printf("Valido!");
    else 
        return 1;

    if (num_workers >= 1 && num_workers <= MAX_WORKERS) 
        print("num_workers validos");
    else 
        return 1;

    if (charset_len > 0) 
        printf("Charset valido");
    else 
        return 1;
  

    
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    
    // IMPLEMENTE AQUI: Rdtcm notas: nao testei!!!
    //long long total_possibilities = (long long)pow(password_len, charset_len); // --> numeros grandes pode ocorrer overflow 
    //long long passwords_per_worker = total_possibilities / MAX_WORKERS;  // ### ? max_workers está correto ? ou seria num_workers do argv
    //long long remaining = total_possibilities % MAX_WORKERS;    // ### ? max_workers está correto ? ou seria num_workers do argv

    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];   //   ### ? max_workers está correto ? ou seria num_workers do argv
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
        int inicioIntervalo = i *passwords_per_worker; //iniciar em intervalos diferentes para cada i do loop
        int fimIntervalo; // onde acaba o intervalo desse processo
        // TODO: Calcular intervalo de senhas para este worker
        if( i != (num_workers-1)){ //se nao for o ultimo então fim do intervalo = inicio + divisão das possibilidades
        // Exemplo :se for 100 possibilidades / 4 processos, então fim = inicio[0,25,50] + divisão das possibilidades[25]- 1 = [24,49,74]
            fimIntervalo = inicioIntervalo + (passwords_per_worker -1); 
        }
        else{ //caso do ultimo processo fim = 100 - 1 logo o ultimo inicia em 75 e acaba em 99.
            fimIntervalo = total_space - 1;
        }
        // TODO: Converter indices para senhas de inicio e fim
        
        // TODO 4: Usar fork() para criar processo filho
        pid_t pid = fork();
        if( pid < 0 ){
            printf("Erro no fork!");
            exit(1);
        }
        // TODO 5: No processo pai: armazenar PID
        if( pid >0 ){
            workers[i]=pid;//armazena o pid do filho
            printf("worker: %d > pid: %d > intervalo de trabalho: %d até %d \n",i, workers[i], inicioIntervalo, fimIntervalo); 
            
        }
        // TODO 6: No processo filho: usar execl() para executar worker
        else{//filho que executa
            //para exec1 e necessario converter os argumentos
            char inicioStr[32],fimStr[32],charsetLenStr[16],passLenStr[16],workerIdStr[8];
            //snprintf converte de maneira segura limitando o maximo de tamanho
            sniprintf(inicioStr, sizeof(inicioStr),"%lld",inicioIntervalo);
            sniprintf(fimStr, sizeof(fimStr),"%lld",fimIntervalo);
            sniprintf(charsetLenStr, sizeof(charsetLenStr),"%lld",charset_len);
            sniprintf(passLenStr, sizeof(passLenStr),"%d",password_len);
            sniprintf(workerIdStr, sizeof(workerIdStr),"%d", i);
            
            exec1("./worker", "./worker",target_hash, inicioStr,fimStr,
                charset,charsetLenStr,passLenStr, workerIdStr,(char *)NULL);//testar.

            perror("erro no exec1 filho!");
            _exit(1); //terminar o filho depois
        }
        // TODO 7: Tratar erros de fork() e execl()
    }
    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram
    int terminados = 0;
    int status;
    while(terminados < num_workers){
        pid_t w = wait(&status);
        //apos erro 10 é definido para errno que contem as definições valores de errros, sendo 10 para ECHILD(sem processos filhos restantes)
        if (w == -1){
            if(errno ==EINTR)continue;
            if(errno ==ECHILD) break;
            perror("erro no wait");
            break;
        }
        int worker_index = -1;//serve como flag 
        for (int i =0; i<num_workers; i++){
            if(workers[i] == w){ //compara worker[i] com o w retornado, se estiver no array>encontrado >muda o valor workers[i] para 0
                worker_index = i;
                workers[i] =0;
                break;
            }
        } 
        if(worker_index == -1){ //não existe no array de workers[], 
            //stederr para condição anômala e diagnostica. 
            fprintf(stderr,"PID retornado por wait nao existe no array", w);
        }
            
        if(WIFEXITED(status)){
            int exit_code = WEXITSTATUS(status);
            printf("Filho terminado normalmente! codigo status: %d \n",exit_code);
        } else if (WIFSIGNALED(status)){
            int sig= WTERMSIG(status);
            printf("Terminado por sinal %d\n",sig);
        } else{
            printf("Terminado com status: %d\n", status);
        }
        terminados++;
    }//https://br-c.org/doku.php?id=wait && https://learn.microsoft.com/pt-br/cpp/c-runtime-library/errno-constants?view=msvc-170

    
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    
    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado
    int sz;
    int fd = open(RESULT_FILE, O_RDONLY, 0644);
    if (fd >= 0) {
        char buffer[256];

        sz = read(fd, buffer, sizeof(buffer) - 1);
        buffer[sz] = '\0';
        close(fd);
        char* token = strtok(buffer, " : ");
        token = strtok(NULL, " : ");
        if (strcmp(token, target_hash) == 0) {
            
            printf("Senha Encontrada");//Implementar e mandar a senha coletada não na forma MD5
           
        }
    }else {
        printf("Erro ao abrir o arquivo!");
    }
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    
    return 0;
}
