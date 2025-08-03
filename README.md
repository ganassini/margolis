
Margolis
-----

Você irá simular (usando C e PThreads) o funcionamento de um sistema de controle
de tráfego aéreo em um aeroporto internacional com alta demanda de operações.

O aeroporto recebe e envia voos domésticos e internacionais, cada um seguindo
regras diferentes para aquisição de recursos durante pousos, desembarques e decolagens.

O aeroporto possui os seguintes recursos limitados:

* _3 pistas_ (para pouso e decolagem) -> recursos exclusivos
* _5 portões de embarque_ (para entrada e saída de passageiros) -> recursos exclusivos
* _1 torres de controle_ (gerencia a comunicação e liberação de cada operação) -> recurso compartilhado
, mas __só atendem, no máximo, duas operações simultaneamente.__

Cada _avião (thread)_ passará pelas seguintes operações:

### 1. Pouso

* _Requer_: __1 pista__ + __1 torre de controle__
* _Liberação___: __pista__ e __torre__ após pouso
* _Após o pouso__, solicita um __portão__ para desembarque

### 2. Desembarque

* _Requer_: __1 portão__ + __1 torre de controle__
* _Liberação_: __torre__ e, após um tempo, o __portão__
* _Após desembarque_, o avião aguarda para decolar

### 3. Decolagem

* _Requer_: __1 portão__ + __1 pista__ + __1 torre de controle__
* _Liberação_: __todos__ após a decolagem

A principal fonte de deadlocks está na diferença de ordem de alocação de recursos 
entre voos domésticos e internacionais. 

Além disso, voos internacionais possuem prioridade em relação aos domésticos.

---

### Voos Internacionais:

| Operação      | Ordem de Solicitação     |
|---------------|--------------------------|
| Pouso         | Pista -> Torre           |
| Desembarque   | Portão -> Torre          |
| Decolagem     | Portão -> Pista -> Torre |

### Voos Domésticos:

| Operação      | Ordem de Solicitação     |
|---------------|--------------------------|
| Pouso         | Torre -> Pista           |
| Desembarque   | Torre -> Portão          |
| Decolagem     | Torre -> Portão -> Pista |

---

### Problemas a Serem Monitorados:

* Deadlock

* Starvation: Voos domésticos podem nunca conseguir uma torre se há fluxo constante
de voos internacionais. Considere:
    - Após 60s de espera: entrar em estado de alerta crítico
    - Após 90s: o avião "cai" (a thread é finalizada com erro, para simular falha 
    operacional)

### Condições de Parada da simulação:

* A simulação deve rodar por um _tempo total ajustável (ex: 5 minutos)_
* A função principal _main cria threads continuamente_ com _intervalos randômicos_
* Após o tempo total, novas threads deixam de ser criadas
* As threads em execução continuam até finalizarem suas operações
* Ao final, um _relatório final com as métricas é exibido_

Durante a execução, _imprima tudo que ocorrer_. E, ao final da simulação, o sistema deverá:

* Mostrar um _resumo com o estado final de cada avião_
* Informar os _casos de sucesso e falha_
* Evidenciar as _situações de deadlock e starvation (ou no mínimo, apresentar a contagem)_
* Permitir que o _código seja reutilizado para experimentos com diferentes quantidades
de pistas, torres e portões_
