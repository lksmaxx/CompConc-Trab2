
all: quick_sort.out
	echo "realizando testes...(10 s)\n"
	./quick_sort.out > testes.txt

quick_sort.out: quick_sort2.c
	gcc quick_sort2.c -o quick_sort.out -lpthread -Wall
