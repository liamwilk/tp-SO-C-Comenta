# KERNEL

## PRUEBA PLANI

### 1. Levantar memoria

```bash
cd ./memoria

make memoria config=./configs/plani.config

## Con valgrind
make memcheck config=./configs/plani.config
```

### 2. Levantar cpu
```bash
cd ./cpu

make cpu config=./configs/plani.config

## Con valgrind
make memcheck config=./configs/plani.config
```

### 3. Levantar kernel
```bash
cd ./kernel

## Para FIFO
make kernel config=./configs/plani_fifo.config

## Para Round Robin
make kernel config=./configs/plani_rr.config

## Para Virtual Round Robin
make kernel config=./configs/plani_vrr.config

## Con valgrind
make memcheck config=./configs/plani_fifo.config
```

### 4. Tener levantada la IO de SLP1

```bash
## IO_GEN_SLEEP
make io nombre=SLP1 config=./config/plani/slp1.config

## Con valgrind
make memcheck nombre=SLP1 config=./config/plani/slp1.config
```


## PRUEBA DEADLOCK

### 1. Levantar memoria

```bash
cd ./memoria

make memoria config=./configs/deadlock.config

## Con valgrind
make memcheck config=./configs/deadlock.config
```

### 2. Levantar cpu
```bash
cd ./cpu

make cpu config=./configs/deadlock.config

## Con valgrind
make memcheck config=./configs/deadlock.config
```

### 3. Levantar kernel
```bash
cd ./kernel

make kernel config=./configs/deadlock.config

## Con valgrind
make memcheck config=./configs/deadlock.config
```

### 4. Tener levantada la IO de ESPERA

```bash
## IO_GEN_SLEEP
make io nombre=ESPERA config=./config/deadlock/espera.config

## Con valgrind
make memcheck nombre=ESPERA config=./config/deadlock/espera.config
```

## PRUEBA MEMORIA y TLB

```bash
cd ./memoria

make memoria config=./configs/memoria.config

## Con valgrind
make memcheck config=./configs/memoria.config
```

### 2. Levantar cpu
```bash
cd ./cpu

## Para fifo
make cpu config=./configs/memoria_fifo.config
## Para LRU
make cpu config=./configs/memoria_lru.config

## Con valgrind
make memcheck config=./configs/memoria.config
```

### 3. Levantar kernel
```bash
cd ./kernel

make kernel config=./configs/memoria.config

## Con valgrind
make memcheck config=./configs/memoria.config
```

### 4. Tener levantada la IO de IO_GEN_SLEEP

```bash
## IO_GEN_SLEEP
make io nombre=IO_GEN_SLEEP config=./config/memoria/io_gen_sleep.config

## Con valgrind
make memcheck nombre=IO_GEN_SLEEP config=./config/memoria/io_gen_sleep.config
```

## PRUEBA IO

```bash
cd ./memoria

make memoria config=./configs/io.config

## Con valgrind
make memcheck config=./configs/io.config
```

### 2. Levantar cpu

```bash
cd ./cpu

## Para fifo
make cpu config=./configs/io.config
## Para LRU
make cpu config=./configs/io.config

## Con valgrind
make memcheck config=./configs/io.config
```

### 3. Levantar kernel
```bash
cd ./kernel

make kernel config=./configs/io.config

## Con valgrind
make memcheck config=./configs/io.config
```

### 4. Tener levantada la IO de TECLADO,GENERICA,MONITOR

```bash
## TECLADO 
make io nombre=TECLADO config=./config/io/teclado.config

## MONITOR 
make io nombre=MONITOR config=./config/io/monitor.config

## GENERICA 
make io nombre=GENERICA config=./config/io/generica.config

## Con valgrind reemplazar {io} por NOMBRE_IO (GENERICA,MONITOR,TECLADO)
make memcheck nombre=NOMBRE config=./config/io/{io}.config
```


## PRUEBA FS

```bash
cd ./memoria

make memoria config=./configs/fs.config

## Con valgrind
make memcheck config=./configs/fs.config
```

### 2. Levantar cpu

```bash
cd ./cpu

make cpu config=./configs/fs.config

## Con valgrind
make memcheck config=./configs/fs.config
```

### 3. Levantar kernel
```bash
cd ./kernel

make kernel config=./configs/fs.config

## Con valgrind
make memcheck config=./configs/fs.config
```

### 4. Tener levantada la IO de FS,TECLADO,MONITOR

```bash

## FS (hay que reemplazar el path con el nombre del usuario owner) 
make io nombre=FS config=./config/fs/fs.config

## MONITOR 
make io nombre=MONITOR config=./config/fs/monitor.config

## TECLADO 
make io nombre=TECLADO config=./config/fs/teclado.config

## Con valgrind reemplazar {io} por NOMBRE_IO (FS,MONITOR,TECLADO)
make memcheck nombre=NOMBRE config=./config/fs/{io}.config
```

## Prueba Salvation's Edge

```bash
cd ./memoria

make memoria config=./configs/salvation_edge.config

## Con valgrind
make memcheck config=./configs/salvation_edge.config
```

### 2. Levantar cpu

```bash
cd ./cpu

make cpu config=./configs/salvation_edge.config

## Con valgrind
make memcheck config=./configs/salvation_edge.config
```

### 3. Levantar kernel

```bash
cd ./kernel

make kernel config=./configs/salvation_edge.config

## Con valgrind
make memcheck config=./configs/salvation_edge.config
```

### 4. Tener levantada la IO de FS,TECLADO,MONITOR

```bash

## ESPERA
make io nombre=ESPERA config=./config/salvation_edge/espera.config

## GENERICA 
make io nombre=GENERICA config=./config/salvation_edge/generica.config

## MONITOR 
make io nombre=MONITOR config=./config/salvation_edge/monitor.config

## SLP1 
make io nombre=SLP1 config=./config/salvation_edge/slp1.config

## TECLADO 
make io nombre=TECLADO config=./config/salvation_edge/teclado.config

## Con valgrind reemplazar {io} por NOMBRE_IO (ESPERA,TECLADO,GENERICA,SLP1,MONITOR)
make memcheck nombre=NOMBRE config=./config/salvation_edge/{io}.config
```