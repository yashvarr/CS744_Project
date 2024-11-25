CC=gcc
CFLAGS=-pthread -I/usr/include -L/usr/lib/x86_64-linux-gnu

all: broker publisher subscriber

broker: multisend_broker.c
	$(CC) $(CFLAGS) -o broker multisend_broker.c -lsqlite3

publisher: publisher.c
	$(CC) $(CFLAGS) -o publisher publisher.c

subscriber: new_subscriber.c
	$(CC) $(CFLAGS) -o subscriber new_subscriber.c

clean:
	rm -f broker publisher subscriber pubsub.db
