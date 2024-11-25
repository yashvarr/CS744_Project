#!/bin/bash

# Check if the user provided the number of subscribers
if [ -z "$1" ]; then
  echo "Usage: $0 <number_of_subscribers>"
  exit 1
fi

# Number of subscribers to generate
num_subscribers=$1

# Define the list of possible topics
topics=("news" "sports" "fun")

# Loop to create n subscribers
for ((i=1; i<=num_subscribers; i++))
do
  # Randomly select a topic from the list
  random_topic=${topics[$RANDOM % ${#topics[@]}]}

  # Start the subscriber with the randomly selected topic
  ./subscriber "$random_topic" &

  pid=$!

  # Output the chosen topic for the subscriber
  echo "Subscriber $pid started with topic: $random_topic"
done

# Wait for all background processes to complete
wait
