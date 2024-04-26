.ONESHELL:
SHELL:= /bin/bash
.SHELLFLAGS:= -eu -o pipefail -c
.RECIPEPREFIX = >

search: main.c functions.c functions.h Makefile
> cc -O2 -Wall -Wextra -o search main.c functions.c
debug: main.c functions.c functions.h Makefile
> cc -O2 -Wall -Wextra -g -o search main.c functions.c
clean:
> rm -f search
