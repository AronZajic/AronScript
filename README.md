Compilation:

```
gcc main.c `pkg-config --cflags --libs glib-2.0` -static-libgcc -static-libstdc++ -lcmocka -Wall -Werror -g -o AronScript.exe
```

Running as REPL:

```
./AronScript.exe
```

Running to execute file:

```
./AronScript.exe examples/fiboRec.as
```

Running to lint file:

```
./AronScript.exe check examples/fiboRec.as
```