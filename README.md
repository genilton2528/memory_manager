# memory_manager
## autor: Genilton Barbosa - 5153
### GitHub: https://github.com/genilton2528/memory_manager/

## Visão geral

Neste projeto, foi implementado vários algoritmos de substituição de página de memória virtual. O objetivo deste projeto é comparar e avaliar o impacto desses algoritmos no número de falhas de página orridas em um número variável de quadros de memória física disponíveis.

## Algoritmos de substituição de página
    
1. FIFO
2. FIFO + Bit R (Segunda Chance)
3. NRU
4. LRU

### Métrica de Desempenho

Estamos interessados em calcular o número de falhas de página ocorridas.

## Instruções de como executar:
1. Para compila o codigo execute o comando na raiz do projeto:
  ```
  gcc main.c -o main.out
  ```
2. Para executa o codigo execute o comando a seguir, onde o primeiro argumento selecona o algoritimo a rodar, e o segundo define a frequencia de clock:
  ```
  ./main.out fifo 10 < anomaly.dat
  ```
  obs: Note que aqui estamos direcionando o arquivo com a string de referencia para execução, ex: "< anomaly.dat"
  
3. Caso queira gerar novas strings de referencia execute o arquivo generate_reference_string.c na pasta tests/ 
passando com argumento o numero de paginas, requisições, e range respequitivamente. Example:
  ```
  ./grs.out 20 50 2
  ```
  obs: Caso queira saber mais leia o relatorio_pratico.pdf
