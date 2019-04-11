all: compilaCmd compilaMain

compilaCmd:
	gcc -o PROVA componenti/bulb.c

compilaMain:
	gcc -o PRINCIPALE main.c

binaries=PROVA PRINCIPALE

clean:
	rm -f $(binaries)