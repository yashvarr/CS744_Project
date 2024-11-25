#!/bin/bash

# Check if the user provided the number of subscribers
if [ -z "$1" ]; then
  echo "Usage: $0 <number_of_subscribers>"
  exit 1
fi

num_subscribers=$1

topics=("news" "sports" "fun")

for ((i=1; i<=num_subscribers; i++))
do
  random_topic=${topics[$RANDOM % ${#topics[@]}]}

  ./subscriber "$random_topic" &

  pid=$!

  echo "Subscriber $pid started with topic: $random_topic"
done

#wait for all processes to complete
wait
