all : script program
	
script : script.sh fact.s
	sh script.sh fact.s
program : prog.c
	gcc prog.c -lm -o cpu

