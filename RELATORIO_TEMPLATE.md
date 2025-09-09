# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Alef (RA:10431891 ), Derick (RA:), Renan (RA:10438120), Ryan (RA: 10352727) 
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

[Explique seu algoritmo de divisão]
Cada worker recebeu a divisão do total pelo num_workers(total_space/num_workers), salvo em caso que valor de intervalo não resulte em uma divisão certa pra cada worker, exemplo: 10 possibilidades com 3 workers: ( 10/3 ) então os intervalos para cada worker= [0]worker=3, [1]worker=3 e [2]worker=4, logo o ultimo worker receberia a sobra de uma divisão não certa, apesar disso, não representa uma uma sobrecarga do ultimo.

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
long long total_space = calculate_search_space(charset_len, password_len);
long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
.
.
.// EM CADA ITERAÇÃO.
        long long inicioIntervalo = i *passwords_per_worker; //iniciar em intervalos diferentes para cada i do loop
        long long fimIntervalo; // onde acaba o intervalo desse processo
        // TODO: Calcular intervalo de senhas para este worker
        if( i != (num_workers-1)){ //se nao for o ultimo então fim do intervalo = inicio + divisão das possibilidades
        // Exemplo :se for 100 possibilidades / 4 processos, então fim = inicio[0,25,50] + divisão das possibilidades[25]- 1 = [24,49,74]
            fimIntervalo = inicioIntervalo + (passwords_per_worker -1); 
        }
        else{ //caso do ultimo processo fim = 100 - 1 logo o ultimo inicia em 75 e acaba em 99.
            fimIntervalo = total_space - 1;
        }
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

[Explique em um parágrafo como você criou os processos, passou argumentos e esperou pela conclusão]
Começando pelo loop que itera até a quantidade de workers da variavel (num_workers=argv[4]) escolhido ao iniciar o programa, realizando no loop a criação de um processo duplicado(filho) com fork em cada iteração retornando (pid==0) para o filho que executa o mesmo código, após isso, com execl o processo filho é substituido por outro programa que executa nesse caso o ./worker não retornando mais, por isso se faz necessário o '_exit(1)' e 'perror' para tratamento de erro. Através do wait() bloqueia até que um os processos filhos termine, com isso o processo pai consegue saber qual PID(worker) retorna e com ele seu status de sáida que representa como o processo filho terminou. 
**Código do fork/exec:**
```c
for (int i = 0; i < num_workers; i++) {
        long long inicioIntervalo = i *passwords_per_worker; //iniciar em intervalos diferentes para cada i do loop
        long long fimIntervalo; // onde acaba o intervalo desse processo
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
            char inicioSenha[32],fimSenha[32] ;
            //snprintf converte de maneira segura limitando o maximo de tamanho
            snprintf(inicioStr, sizeof(inicioStr),"%lld",inicioIntervalo);
            snprintf(fimStr, sizeof(fimStr),"%lld",fimIntervalo);
            //snprintf(charsetLenStr, sizeof(charsetLenStr),"%lld",charset_len);
            snprintf(passLenStr, sizeof(passLenStr),"%d",password_len);
            snprintf(workerIdStr, sizeof(workerIdStr),"%d", i);

            index_to_password(inicioIntervalo,charset, charset_len,password_len,inicioSenha);
            index_to_password(fimIntervalo,charset, charset_len,password_len,fimSenha);
            
            
            execl("./worker",target_hash, inicioSenha,fimSenha,
                charset,passLenStr, workerIdStr, (char *)NULL);//testar.

            perror("erro no exec1 filho!");
            _exit(1); //terminar o filho depois
        }
        // TODO 7: Tratar erros de fork() e execl()
    }
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

[Explique como você implementou uma escrita atômica e como isso evita condições de corrida]
Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**

Ele le o arquivo do resultado, caso o arquivo não exista ele continua , caso ele exista ele para em todos outros workes, esse ciclo
é repetido a cada PROGRESS_INTERVAL no nosso caso definimos ele como 100000 o que funciona para senhas maiores onde existe um tempo
necessario maior

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | real:0m0.007s | real:0m0.007s | real:0m0.007s | 0m0.001s |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | real:0m4.474s | real:0m7.808s | real:0m1.598s | 2.799S |

**O speedup foi linear? Por quê?**
[Analise se dobrar workers realmente dobrou a velocidade e explique o overhead de criar processos]

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
[Descreva um problema e como resolveu. Ex: "Tive dificuldade com o incremento de senha, mas resolvi tratando-o como um contador em base variável"]

Tivemos dificuldades ao usar o fork() para criar múltiplos processos filhos, pois todos os filhos estavam executando o mesmo código do processo pai, o que causava repetição de tarefas ou conflitos. Para resolver isso, usamos fork() seguido de exec(), permitindo que cada filho substituísse sua imagem de processo por uma nova tarefa especificia, garantindo a separação correta das responsabilidades entre os processos.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [X] Código compila sem erros
- [X] Todos os TODOs foram implementados
- [ ] Testes passam no `./tests/simple_test.sh`
- [ ] Relatório preenchido
