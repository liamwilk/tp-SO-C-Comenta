# redonditOS

Implementación de [C-Comenta](https://faq.utnso.com.ar/tp-c-comenta/) - Sistemas Operativos - 1C 2024 - UTN FRBA

## Test cases

En este tp utilizamos  `mumuki/cspec`, se tiene que habilitar globalmente como una biblioteca:

```bash
git clone https://github.com/mumuki/cspec.git
cd cspec
make install
```

## Git hooks
Para utilizar los pre-commits & git hooks:

```bash
chmod +x hooks.sh 
sh hooks.sh
```

Si queres saltearte las verificaciones:

```bash
git commit -m "Mensaje commit" --no-verify
```

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante se guardará en la carpeta `bin` del módulo.

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```

Asegúrense de que el código compila y cumple con los requisitos del checkpoint
antes de subir el tag.

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=entradasalida "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

Ante cualquier duda, podés consultar la documentación en el repositorio de
[so-deploy], o utilizar el comando `./deploy.sh -h`.

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy

<hr>
<div id="footer" align="center">
  <a href="https://www.frba.utn.edu.ar/">
  <img src="https://github.com/sisoputnfrba/tp-2024-1c-Operativos-y-los-Redonditos-de-Ricota/assets/94919997/e11f9148-822e-427d-b593-f12d608b0693" style="width:70%; height:auto;">
  </a>
</div>
