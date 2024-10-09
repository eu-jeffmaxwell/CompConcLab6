# CompConcLab6

Em relação ao tempo total de execução, a prioridade de escrita não necessariamente melhora o desempenho. Na verdade, se a maioria das operações for de leitura, como no seu caso (98% de consultas), priorizar a escrita pode aumentar o tempo total de execução porque as operações de leitura serão bloqueadas mais frequentemente.
No entanto, se o padrão de uso do sistema envolver uma proporção maior de escritas e remoções, ou se a consistência e a prioridade de escrita forem mais importantes que o tempo total de execução, essa abordagem pode ser mais justa e segura em termos de integridade de dados. Portanto, a prioridade de escrita não necessariamente faz o programa mais rápido; pode até piorar o desempenho em cenários onde as leituras são mais frequentes.Logo, principal vantagem da prioridade de escrita é garantir que escritores não fiquem famintos, o que é útil em cenários com muitas leituras e poucas escritas.

Alguns métodos:

* A chamada para sched_yield() só ocorre se houver escritores esperando. Ela só será chamada enquanto há escritores em espera, em vez de ficar em um loop ocupado constantemente.

* As operações de leitura continuam a ser bloqueadas quando há escritores esperando, mas agora não há looping desnecessário se escritores ainda estiverem esperando.
  
* Assim que o último escritor finaliza, as leituras são retomadas de maneira eficiente.

* Atomicidade com __sync_fetch_and_add e __sync_fetch_and_sub: Essas funções garantem que o contador de escritores esperando (writers_waiting) seja atualizado de forma atômica, sem que múltiplas threads corrompam o valor.
