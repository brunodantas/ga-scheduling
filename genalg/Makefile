# Main target

.PHONY: genalg

genalg: genalg.c
	mpicc ../domain/individual.c population.c selection.c ../domain/crossover.c reinsertion.c ../domain/graph.c ../domain/getinput.c ../domain/list.c multipop.c genalg.c -o genalg -O3