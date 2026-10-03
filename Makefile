CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -g -pthread

race:
	cc $(CFLAGS) test/test_race.c src/hashtable.c -o test_race
	./test_race

tsan:
	cc $(CFLAGS) -fsanitize=thread test/test_race.c src/hashtable.c -o test_race_tsan
	./test_race_tsan

clean:
	rm -f test_race test_race_tsan