# SCHSIM

## Descripció
Aquest projecte es realitza en el marc de l'assignatura de Sistemes Operatius. L'objectiu és ajudar a entendre els algorismes de planificació de la CPU i millora les habilitades de codificació en C.

## Implementació
* Actualment, permet executar FCFS, Prioritats, Robin i SJF amb procés amb una única ràfega de CPU.
* No hi ha suspensió de processos.
* El temps per intercanviar 2 processos és nul.
* Totes les operacions d'entrada/sortida de diferents processos es poden superposar.
* Únicament permet simular 1 processador.

## Com fer-ho servir
```sh
make
./main -a fcfs -m preemptive -f ./process.csv 
./main -a fcfs -m nonpreemptive -f ./process.csv 
./main -a priorities -m preemptive -f ./process.csv 
./main -a priorities -m nonpreemptive -f ./process.csv 
./main -a sjf -m preemptive -f ./process.csv 
./main -a sjf -m nonpreemptive -f ./process.csv -
./main -a rr -m preemptive -f ./process.csv 
./main -a rr -m nonpreemptive -f ./process.csv 
```

