//
// Created by kps on 14/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include "../include/multiwc.h"

void message(const char* message)
{
	printf("multiwc: %s\n", message);
}

void die_with_message(const char* msg)
{
	message(msg);
	exit(EXIT_FAILURE);
}

int is_ascii_file(FILE* file)
{
	int c;
	while ((c = fgetc(file)) != EOF)
	{
		if (!isascii(c))
			return 0;
	}
	rewind(file);
	return 1;
}

int is_wordsep(const char* c)
{
	if (*c == ' ' || *c == '\n' || *c == '\t' || *c == '\0')
		return 1;
	return 0;
}

void* wc(void* chnk)
{
	chunk_t chunk = *(chunk_t*)chnk;

	int wcount = 0;
	int i = 0;
	for (; i < chunk.offset - 1; ++i)
		if (!is_wordsep(&chunk.buffer[i]) && is_wordsep(&chunk.buffer[i + 1]))
			wcount++;

	// Checking with opposite_char to eliminate the Split-word problem
	if (!is_wordsep(&chunk.buffer[i]) && is_wordsep(&chunk.opposite_char))
		wcount++;
	return (void*)wcount;
}

int multiwc(const char path[])
{
	// Open file
	FILE* fp = fopen(path, "r");
	if (!fp)
		die_with_message("The specified file could not be opened");

	// Check if it's a US-ASCII formatted file
	if (!is_ascii_file(fp))
		die_with_message("The specified file contains non US-ASCII characters.");

	// Allocate buffer
	char* buffer = malloc(MAX_BUFFER_READ * sizeof(char));
	if (!buffer)
		die_with_message("The needed memory could not be allocated.");

	// Word counter
	int wcount = 0;

	size_t buffer_s;
	size_t chunk_s;
	size_t chunk_size_remainder;

	// The last character of the previously read buffer
	char last_c = ' '; // Choose a value that will not add up during the first reading, hence ' '

	// Start reading the file
	while ((buffer_s = fread(buffer, sizeof(char), MAX_BUFFER_READ, fp)))
	{
		// Split-word during file segmentation fix
		if (!is_wordsep(&last_c) && !is_wordsep(&buffer[0]))
			wcount--;
		last_c = buffer[buffer_s - 1];

		// Create a chunk
		chunk_t chunk;

		// Calculate chunk_s and the remainder
		chunk_s = buffer_s / MAX_THREADS_COUNT;
		chunk_size_remainder = buffer_s % MAX_THREADS_COUNT;

		// Choose whether a multi-threaded or single-threaded approach should be run
		if (chunk_s >= MIN_LOAD_PER_THREAD)
		{
			// Setup the threads
			pthread_t* pthread = malloc(MAX_THREADS_COUNT * sizeof(pthread_t));

			// Setup the array of chunks
			chunk_t chunks[MAX_THREADS_COUNT];

			void* wcount_per_chunk; // Return Word counter for each thread

			char* buffer_incr = buffer; // temp buffer to mess with it

			// If the division was perfect, then the load will be divided equally among the threads
			if (chunk_size_remainder == 0)
				for (int thread = 0; thread < MAX_THREADS_COUNT; ++thread)
				{
					// Build the chunk
					chunk.buffer = buffer_incr;
					chunk.offset = chunk_s;
					chunk.opposite_char = buffer_incr[chunk_s];
					chunks[thread] = chunk;

					pthread_create(&pthread[thread], NULL, wc, &chunks[thread]);

					(buffer_incr += chunk_s);
				}
			else
			{
				// If the division is not perfect, do some load balancing
				int thread = 0;
				for (; thread < chunk_size_remainder; ++thread)
				{
					// Build the chunk
					chunk.buffer = buffer_incr;
					chunk.offset = chunk_s;
					chunk.opposite_char = buffer_incr[chunk_s + 1];
					chunks[thread] = chunk;

					pthread_create(&pthread[thread], NULL, wc, &chunks[thread]);

					(buffer_incr += chunk_s + 1);
				}
				for (; thread < MAX_THREADS_COUNT; ++thread)
				{
					// Build the chunk
					chunk.buffer = buffer_incr;
					chunk.offset = chunk_s;
					chunk.opposite_char = buffer_incr[chunk_s];
					chunks[thread] = chunk;

					pthread_create(&pthread[thread], NULL, wc, &chunks[thread]);

					(buffer_incr += chunk_s);
				}
			}

			// Join all the threads, increment by the return value and free the thread struct
			for (int j = 0; j < MAX_THREADS_COUNT; ++j)
			{
				pthread_join(pthread[j], &wcount_per_chunk);
				wcount += (int)wcount_per_chunk;
			}
			free(pthread);
		}
		else
		{
			// Single-threaded approach where the whole buffer is given to wc

			// Build the chunk
			chunk.buffer = buffer;
			chunk.offset = buffer_s;
			chunk.opposite_char = '\0';
			wcount += (int)wc(&chunk);
		}
	}
	fclose(fp);
	free(buffer);
	return wcount;
}
