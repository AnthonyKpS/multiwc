#!/bin/bash

# Generate tests
create_tests() {
  local prompt=""
  read -r -p "Should 14 ASCII formatted samples with varying byte size be created in a /samples directory? [Y/n]" prompt

  # Check if the samples dir exists
  if [ ! -d "samples" ]; then
    mkdir samples
  fi

  # toLowercase prompt for easier check
  prompt=$(echo "$prompt" | tr '[:upper:]' '[:lower:]') # tr-anslate only ECHOES the result hence the "echo" command

  case $prompt in
  y | ye | yes)
    bytes=0 # Number of bytes per test
    echo "Generating tests..."
    echo "This can take up to 10 minutes."
    for ((power = 5; power <= 30; power += 2)); do
      bytes=$((2 ** power))
      tr -dc '[:alnum:] \n' </dev/urandom | head -c $bytes > samples/S"$bytes".txt
    done
    echo "Tests can be found in the /samples dir."
    ;;

  n | no)
    echo "No tests for you!"
    ;;
  *)
    echo "I did not get that."
    echo "Exiting..."
    ;;
  esac
}
create_tests
exit 0
