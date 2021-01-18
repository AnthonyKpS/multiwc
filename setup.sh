#!/bin/bash

echo "====================[ 2nd Assignment | DIT136 | it21927 | SETUP]================================="

# Check CMAKE dependency
check_dependencies() {
  echo "Checking CMAKE dependency"
  if command cmake --version >/dev/null 2>&1; then
    echo "CMAKE was found"
    echo "Version: $(cmake --version | head -n 1)"
  else
    echo "CMAKE was not found"
    local prompt=""
    read -r -p "Install latest CMAKE? [Y/n]" prompt

    # toLowercase prompt for easier check
    prompt=$(echo "$prompt" | tr '[:upper:]' '[:lower:]') # tr-anslate only ECHOES the result hence the "echo" command

    case $prompt in

    y | ye | yes)
      command sudo snap install cmake
      ;;

    n | no)
      echo ""
      echo "Cmake is necessary!"
      exit 0
      ;;
    *)
      echo ""
      echo -e "I did not get that."
      echo "Exiting..."
      exit 0
      ;;

    esac
  fi
}

# Generate tests
create_tests() {
  local prompt=""
  read -r -p "Generate some tests? [Y/n]" prompt

  # Check if the samples dir exists
  if [ ! -d "/samples" ]; then
    mkdir samples
  fi

  # toLowercase prompt for easier check
  prompt=$(echo "$prompt" | tr '[:upper:]' '[:lower:]') # tr-anslate only ECHOES the result hence the "echo" command

  case $prompt in
  y | ye | yes)
    bytes=0 # Number of bytes per test
    echo "Generating tests..."
    for ((power = 5; power <= 30; power += 2)); do
      bytes=$((2 ** power))
      tr -dc '[:alnum:] \n' </dev/urandom | head -c $bytes >samples/S"$bytes".txt
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

# Run CMAKE
create_makefile() {
  local prompt=""
  read -r -p "Create makefile? [Y/n]" prompt

  # toLowercase prompt for easier check
  prompt=$(echo "$prompt" | tr '[:upper:]' '[:lower:]') # tr-anslate only ECHOES the result hence the "echo" command

  case $prompt in
  y | ye | yes)
    cmake . # using cmake in the root folder
    echo "Run <make> in the root directory to compiler the main program."
    ;;

  n | no)
    echo "To generate the makefile manually, run <cmake .> in the root directory."
    ;;
  *)
    echo "I did not get that."
    echo "Exiting..."
    ;;
  esac
}

check_dependencies
create_tests
create_makefile

exit 1
