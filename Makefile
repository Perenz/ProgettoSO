all: compilaCmd compilaMain

compilaCmd:
	gcc -o PROVA componenti/bulb.c

compilaMain:
	gcc -o PRINCIPALE strutture/list.c main.c

binaries=PROVA PRINCIPALE

clean:
	rm -f $(binaries)