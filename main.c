#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "multiwc.h"

void sig_handler(int signum)
{
	printf("Ignoring signal: %d", signum);
}

int main(int argc, char** argv)
{
	// Entry
	message("Welcome to multiwc!");

	// Path from arg
	const char* dir_path = realpath("../tests", NULL);

	// Make a DIR
	DIR* dir = opendir(dir_path);
	if (!dir)
		die_with_message("The specified directory could not be opened.");

	// "Path to dir" length
	size_t path_to_dir_l = strlen(dir_path);
	char* filename;
	size_t filename_l;

	puts("");
	pid_t pid;
	struct dirent* dir_data;
	while ((dir_data = readdir(dir))) // While there are more "things" in the dir
	{
		// Get filename
		filename = dir_data->d_name;

		if (!is_dir(filename)) // !dir == file
		{
			// Setup the output.txt file
			int fp = open("../output.txt", O_WRONLY | O_APPEND | O_CREAT);
			if (fp == -1)
				die_with_message("File could not be opened");

			// Get filename's length
			filename_l = strlen(filename);

			// Allocate a suitably sized file_path buffer
			char file_path[path_to_dir_l + 1 + filename_l + 1];

			// "Build" the file_path
			snprintf(file_path, sizeof file_path, "%s/%s", dir_path, filename);

			// Make a new process
			pid = fork();
			if (pid == 0)
			{
				// Child code
				char buffer[1024];
				snprintf(buffer, sizeof(buffer), "%d, %s, %d\n", getpid(), filename, multiwc(file_path));
				printf("%s", buffer);
				write(fp, buffer, strlen(buffer));
				exit(EXIT_SUCCESS);
			}
			else
			{
				// Treat signals
				signal(SIGINT, sig_handler);
				signal(SIGTERM, sig_handler);

				// Parent code
				wait(NULL);
			}
		}
	}
	closedir(dir);
	puts("");
	message("Results are also present in the output.txt file that was generated in the directory that you run multiwc.");
	message("Thank you for using multiwc!");
	return 0;
}