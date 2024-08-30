# shell

### Búsqueda en $PATH
1) La diferencia entre **exec(3)** y **execve(2)** radica en que execve realiza una llama directa al sistema operativo mientras que la familia de wrappers proporcionan una interfaz para invocar a la syscall.

    Otra de las diferencias tiene que ver con los argumentos que recibe ya que execve permite especificar el entorno para el nuevo programa, mientras que la familia de execve(3) heredan el mismo entorno que el proceso invocante. Este entorno contiene informacion como la ruta y variables.

    La llamada a execve reemplaza el proceso que lo llama con el nuevo programa con un nuevo stack, heap y data segments.

2) La llamada a exec(3) puede fallar devolviendo -1 y seteando el errno. En el caso de nuestra implementación, si falla el execvp() imprimimos un mensaje describiendo el error, se setea el errno y hace un exit(-1). Esta llamada puede fallar por diferentes motivos como archivo no encontrado, permisos insuficientes, espacio de memoria insuficiente del kernel, entre otras.

---

### Procesos en segundo plano

1) El proceso que implementamos para los procesos de segundo plano consiste en el manejo del simbolo '&' en los comandos de entrada. Una vez parseado el comando obteniendo en una entructura cmd, se ejecuta el comando en segundo plano manejandolo en el bloque 'BACK' del switch en la funcion exec_md. Esta funcion llama recursivamente a exec_cmd eviandole en este caso el cmd que tiene la estructura de back_cmd para que este se ejecute en segundo plano.

    Una vez que exec_cmd se llama recursivamente con b->c, el proceso padre no espera a que el proceso hijo (b->c) termine. Esto permite que el proceso padre continúe ejecutando otras tareas sin esperar.
    El proceso hijo (b->c) se ejecuta independientemente, y el proceso padre se libera para ejecutar otros comandos o tareas. Esto se debe a que el proceso padre llama a waitpid con el parametro WNOHANG que indica que el proceso no debe quedar bloqueado esperando a que los procesos hijos terminen.
---

### Flujo estándar

1) El operador '2>&1' es una redirección de entrada/salida en la shell que se utiliza para redirigir la salida estándar de error (stderr, el file descriptor 2) a la salida estándar (stdout, el file descriptor 1).
    Esto significa que los mensajes de error generados por un comando se enviarán a la misma ubicación que la salida estándar, lo que hace que ambos flujos (salida estándar y salida de error) se mezclen.

    La shell realiza esta redirección del stderr utilizando la syscall dup2().

    A nivel del kernel, esto implica actualizar las estructuras internas de descriptores de archivos para que ambos (stderr y stdout) apunten al mismo objeto de archivo.

2) Probando la salida de "cat out.txt" del ejemplo en nuestra shell, vemos que tanto la salida estandar como la de error fueron redirigidas al archivo "out.txt". Esto es asi ya que primero se redirige la salida estandar al archivo y luego la salida estándar de error es redirigida a la salida estándar, lo que genera que ambas salidas se guarden en el archivo en cuestión.

    Si probamos el mismo script pero inviertiendo las redirecciones, observamos que nuestra shell responde de manera idéntica al caso anterior. Esto es asi dado que al primero parsear la estructura de cmd completa, ya se encuentran definidas las redirecciones sin importar el orden en el que sucedieron. Entonces cuando nuestra implementacion recibe ese parseo, como primero preguntamos si existe el out file antes que el err file, la redireccion del error sucede luego de redirigir el output, entonces terminan en el mismo lugar sin importar el orden.

    ```python
    (/home/abrilgiordano)
    $ ls -C /home /noexiste >out.txt 2>&1
        Program: [ls -C /home /noexiste >out.txt 2>&1] exited, status: 2
    (/home/abrilgiordano)
    $ cat out.txt
    ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio /home:
    abrilgiordano
        Program: [cat out.txt] exited, status: 0
    (/home/abrilgiordano)
    $ ls -C /home /noexiste 2>&1 >out.txt
        Program: [ls -C /home /noexiste 2>&1 >out.txt] exited, status: 2
    (/home/abrilgiordano)
    $ cat out.txt
    ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio /home:
    abrilgiordano
        Program: [cat out.txt] exited, status: 0
    ```

3) Sin embargo, cuando realizamos estas mismas pruebas en bash, podemos ver un comportamiento distinto con cada prueba. La primera funciona al igual que nuestra shell, pero la segunda difiere. Esto se debe a que primero se redirige la salida de error a la salida estándar, la cual por default esta orientada a la consola. Luego, se redirige la salida estandar al archivo, pero la de error queda apuntando a la consola, lo cual genera que los errores se vean por consola y que únicamente la salida estándar se encuentre almacenada en el archivo.
---

### Tuberías múltiples
1) Cuando se ejecuta un pipe, en bash por ejemplo, el exit code que es reportado es el del último comando que fue ejecutado. Esto sucede independientemente de si ese comando es responsable de la salida estándar o de error. En cambio, en nuestra Shell el exit code del pipe siempre es 0, no se ve modificado por los codigos de los comandos ejecutados. Esto es asi ya que al crear los dos procesos hijos para los lados izquierdo y derecho, el padre espera que ambos finalicen pero sin recibir los exit codes de los mismos.

2) Si al ejecutar un pipe fracasa por ejemplo el primer comando, aún asi los siguientes comandos son ejecutados. Esto sucede ya que se continúa recibiendo la entrada que se le proporciona desde el primer comando, y la ejecución del segundo comando procede normalmente. Si por ejemplo el primer comando finaliza en exito pero el segundo falla, el pipe terminara fallando ya que la salida del primer comando se intenta a pasar a un comando que produce un error. De esta manera, analizando los exit codes en estos casos, se obtendrá como exit code del pipe aquel correspondiente al último comando que fue ejecutado, sin importar si el mismo fue exitoso o no.

```python
    :~$ nonexistent_command | echo "hello world"
hello world
nonexistent_command: orden no encontrada
    :~$ echo $?
0
    :~$ echo "hello world" | nonexistent_command
nonexistent_command: orden no encontrada
    :~$ echo $?
127
    :$
```

```python
(/home/abrilgiordano)
$ /bin/no_existe | echo "Hola"
Fallo execvp: No such file or directory
"Hola"
(/home/abrilgiordano)
$ echo $?
0
        Program: [echo $?] exited, status: 0
(/home/abrilgiordano)
$ echo "Hola" | /bin/no_existe
Fallo execvp: No such file or directory
(/home/abrilgiordano)
$ echo $?
0
    Program: [echo $?] exited, status: 0
```

---

### Variables de entorno temporarias

1) Soportamos la inclusión de las variables de entorno temporales después de la llamada a fork(2) para garantizar la eliminación adecuada de estas variables una vez que el proceso hijo haya finalizado su ejecución. Al hacerlo, podemos estar seguros de que solo afectarán al proceso hijo, manteniendo el entorno del proceso padre intacto.

2)
   a) El comportamiento no es el mismo que durante el primer caso. Las variables temporales solo son necesarias durante la ejecución del proceso hijo y no deben permanecer después de eso.
Utilizar setenv(3) ofrece más control, permitiendo eliminar cada variable después de que el proceso hijo termine. En cambio, al pasar un arreglo a exec(3), las variables pueden persistir si no se eliminan correctamente pudiendo causar problemas.
    b) Se podría crear un array para almacenar las variables de entorno temporales ya creadas que se desean establecer para el proceso hijo. Se agregarían estas variables a este array y se utilizaría una de las funciones de la familia exec(3) para ejecutar el programa, pasándolo como tercer argumento. Después de que el proceso hijo haya completado su ejecución, se eliminarían las variables de entorno temporales del array para evitar fugas de memoria.

### Pseudo-variables



- \$$: debería devolver el PID del proceso en ejecución actual

``` python
tomas@tomas-VirtualBox:~/sisop_2024a_g14/shell$ echo $$
66351
```

- $!: debería devolver el PID del último proceso en segundo plano ejecutado.
``` python
tomas@tomas-VirtualBox:~/sisop_2024a_g14/shell$ sleep 10 &
[1] 73527
tomas@tomas-VirtualBox:~/sisop_2024a_g14/shell$ echo $!
73527
[1]+  Hecho                   sleep 10
tomas@tomas-VirtualBox:~/sisop_2024a_g14/shell$
```

- $0: debería devolver el nombre de la shell
``` python
tomas@tomas-VirtualBox:~/sisop_2024a_g14/shell$ echo $0
bash
```


---

### Comandos built-in
1) El comando cd no podría ser implementado si no fuera "built-in", ya que si se ejecutara en un proceso hijo, no podría realizarse el cambio de directorio en el proceso padre. Es decir, al ejecutar este comando en un proceso hijo, se cambiaría el directorio solo para ese proceso, mientras que la shell continuaría en el directorio anterior.

    Por otro lado, el comando pwd si podría implementarse sin ser "built-in". Este comando imprime el directorio de trabajo actual en la consola, lo que significa que su funcionalidad se puede realizar en un proceso hijo de la shell, ya que el directorio actual es compartido por ambos.
    La razón principal para hacer este comando como "built-in" radica en la coherencia dentro del entorno de la shell, como asi tambien la eficacia. El no tener que crear un proceso hijo y poder realizar la ejecución dentro del mismo proceso de la shell, no solo hace que la implementación sea mas sencilla sino también ahorra recursos.
---

### Segundo plano avanzado

1) La implementación del manejo de segundo plano se centra en la utilización de un handler para la señal SIGCHLD. El objetivo es capturar esta señal utilizando la syscall sigaction, configurada de la siguiente manera:

- Capturar específicamente la señal SIGCHLD.
- Utilizar un handler personalizado en lugar del predeterminado.
- Emplear un stack alternativo para el manejo de la señal, evitando así posibles fallos que podrían afectar al stack principal de la shell.
- Asegurar que la ejecución se reanude correctamente después de manejar la señal.

    Por definición, la señal SIGCHLD es enviada por cada proceso hijo a su padre al finalizar. Sin embargo, simplemente configurar el handler no es suficiente para el funcionamiento de los procesos en segundo plano. Esto se debe a que el handler se ejecutaría cada vez que un proceso hijo termine, incluyendo aquellos en primer plano. Por lo tanto, también es necesario determinar de manera adecuada los PGID para agrupar solo aquellos procesos que se estén ejecutando en segundo plano.

    Para lograr un manejo correcto, en el handler se utiliza la syscall waitpid(0, NULL, WNOHANG). Con el primer argumento se especifica que solo se debe esperar a procesos que tengan el mismo PGID que el proceso actual, es decir, la shell. Además, el uso del flag WNOHANG garantiza que esta espera no sea bloqueante, permitiendo que la shell continúe funcionando mientras los procesos en segundo plano se ejecutan.

    Luego, solo queda configurar los PGID de los diferentes tipos de procesos. Dado que el PGID del padre se hereda en el proceso hijo tras realizar un fork, es necesario modificar el PGID en aquellos procesos que no estén en segundo plano. Para lograrlo, decidimos asignar el PGID de cada proceso con su PID único. Este proceso también se repite al ejecutar un pipe, ya que en este caso se crean dos hijos, y es necesario modificar el PGID de ambos para evitar que coincida con el del padre, dado que no son procesos en segundo plano.

2) Utilizamos señales o interrupciones por software para que haya comunicación entre los procesos o entre los procesos y el Kernel. Los escenarios principales en donde se utilizan son cuando hay una excepción de software, el usuario presiona algún caracter especial o cuando un evento que tiene que ver con el software ocurrió (por ejemplo, un proceso hijo terminó).
En nuestra shell utilizamos la señal SIGCHLD, la cual notifica cuando un proceso hijo termina su ejecución. Al capturar esta señal con la función sigaction(2) y configurar el handler, podemos liberar los recursos asociados con el proceso hijo tan pronto como termina. Además, al hacer que la llamada a waitpid no reciba -1 como argumento (sino el flag WNOHANG) la shell maneja el fin de la ejecución de los procesos en segundo plano de forma no bloqueante para que no interfiera con los de primer plano y así se mantenga el funcionamiento correcto de la shell.

### Historial

---
