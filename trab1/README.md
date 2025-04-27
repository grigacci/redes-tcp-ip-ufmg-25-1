### Como usar

## Building
Nesta pasta, execute os seguintes comandos para criar a pasta de build e buildar os codigos:

```shell
mkdir build
cd build/
cmake ..
cmake --build .
```

## Running
Após buildar os códigos, é feito o uso do arquivo run.sh para executar os programas. Os programas precisam de alguns paramentros para iniciar, então aqui fica algumas instruções

```shell
chmod +x run_local.sh
chmod +x run_server.sh
chmod +x run_clients.sh
```

Este comando acima serve apenas para garantir que o script tem permissão para executar.

# ./run_local.sh
Esse script precisa de 2 paremetros, o numero de clientes e o protocolo TCP/UDP
Exemplo:

```shell
./run_local.sh 5 udp
```
# ./run_server.sh
Esse script precisa apenas de 1 parametro, o protocolo TCP/UDP
Exemplo:

```shell
./run_server.sh tcp
```

# ./run_clients.sh
Esse script precisa de 3 parametros, o numero de clientes, o IP de destino e o protocolo TCP/UDP.
Exemplo:

```shell
./run_clients 3 192.168.15.1 udp
```


