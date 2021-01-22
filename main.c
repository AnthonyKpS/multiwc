#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include "include/multiwc.h"

/**
 * Checks if a given path points to a directory.
 * @param path
 * @return 1 If path points to a directory. Otherwise, 0.
 */
int is_dir(const char* path)
{
	struct stat stat_b;
	if (stat(path, &stat_b))
		return 0;
	return S_ISDIR(stat_b.st_mode);
}

/**
 * Calculates the number of files in a given directory.
 * @param dir
 * @return The number of files
 */
int file_count(DIR* dir)
{
	int fcount = 0;
	struct dirent* dir_data;
	while ((dir_data = readdir(dir)))
	{
		if (!is_dir(dir_data->d_name))
			fcount++;
	}
	rewinddir(dir);
	return fcount;
}


/**
 * Handles signals sent to processes by doing nothing except
 * than printing the signal's number. (=essentially ignoring the signal)
 * @param signum The signal's number
 */
void sig_handler(int signum)
{
	printf("Ignoring signal: %d", signum);
}

/**
 * Prints the number of words in every US-ASCII formatted file
 * of a given directory. Uses the multiwc library.
 * @param argc
 * @param argv
 * @return 0 on success. Otherwise EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
	// Entry
	puts("====================[ 2nd Assignment | DIT136 | it21927 | MAIN ]=================================\n");

	// Get the directory's path and it's length
	char* option = argv[1];
	const char* dir_path;
	if (argv[1] == NULL)
		dir_path = realpath(".", NULL);
	else
		dir_path = realpath(argv[1], NULL);

	// opendir() does not accept NULL as a parm so we ought to check
	if (!dir_path)
	{
		printf("The specified directory <%s> could not be opened!\n", option);
		exit(EXIT_FAILURE);
	}

	// Make a DIR
	DIR* dir = opendir(dir_path);
	if (!dir)
	{
		printf("The specified directory <%s> could not be opened!\n", option);
		exit(EXIT_FAILURE);
	}
	printf("Directory given: %s\n\n", dir_path); //
	size_t dir_path_l = strlen(dir_path);

	// Setup the output.txt file
	// Using syscall-s "open", "write" and "close" (further in the code)
	// to take advantage of their atomic behaviour!!!
	int fp = open("output.txt", O_WRONLY | O_CREAT, S_IRWXU);
	if (fp == -1)
	{
		puts("Output file could not be opened");
		exit(EXIT_FAILURE);
	}

	// wcount for further usage
	int wcount = 0;

	// Pid var for further usage
	pid_t pid;

	// Name and size of each file
	char* file_name;
	size_t file_name_l;

	// Struct holding each directory's data
	struct dirent* dir_data;

	// Traversing all the elements in the directory
	while ((dir_data = readdir(dir)))
	{
		// Get the element's name
		file_name = dir_data->d_name;

		// If it's a file
		if (!is_dir(file_name))
		{
			// Get filename's length
			file_name_l = strlen(file_name);

			// Allocate a suitably sized file_path buffer
			char file_path[dir_path_l + 1 + file_name_l + 1];

			// "Build" the file_path
			snprintf(file_path, sizeof file_path, "%s/%s", dir_path, file_name);

			// Make a new process for every new file
			pid = fork();
			if (pid == -1)
			{
				puts("New processes could not be created.");
				exit(EXIT_FAILURE);
			}

			if (pid == 0)
			{
				// Child code
				// Eye-candy timing stats
				time_t start;
				time_t end;

				time(&start); // mark the time when the process starts

				// Run multiwc
				wcount = multiwc(file_path);

				time(&end); // mark the time when wcount returns

				// Build formatted output and write it to file
				char buffer[1024];
				snprintf(buffer, sizeof(buffer), "%d, %s, %d\n", getpid(), file_name, wcount);
				write(fp, buffer, strlen(buffer));

				// Message results
				printf("PID: %d found %d words in file %s in %lds\n", getpid(), wcount, file_name, end - start);

				exit(EXIT_SUCCESS); // Make sure that the child ends
			}
			else
			{
				// Parent code
				// Treat signals
				signal(SIGINT, sig_handler);
				signal(SIGTERM, sig_handler);
			}
		}
	}

	// Wait for all my children
	int wait_status;
	do
	{
		wait_status = wait(NULL);
		if (wait_status == -1 && errno != ECHILD)
		{
			puts("An unexpected error occurred while wait()ing child processes");
			exit(EXIT_FAILURE);
		}
	} while (wait_status > 0);

	close(fp);
	closedir(dir);
	puts("\nResults are also present in the output.txt file that was generated in the root directory.");
	return 0;
}