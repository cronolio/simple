.ONESHELL:
SHELL:= /bin/bash
.SHELLFLAGS:= -eu -o pipefail -c
.RECIPEPREFIX = >

search: search.c functions.c functions.h Makefile
> cc -O2 -Wall -Wextra -o search search.c functions.c
debug: search.c functions.c functions.h Makefile
> cc -O2 -Wall -Wextra -g -o search search.c functions.c
clean:
> rm -f search
