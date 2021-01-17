//
// Created by kps on 14/1/21.
//

#ifndef MULTIWC_MULTIWC_H
#define MULTIWC_MULTIWC_H

/**
 * DEFINES
 */

#define MAX_BUFFER_READ 16 // 1MB
#define MAX_THREADS_COUNT 16
#define MIN_LOAD_PER_THREAD 4


/**
 * STRUCTS
 */

/**
 * Describes the information that each thread
 * needs to do it's job. This info derives from
 * the segmentation of the buffer in chunks.
 * Hence, chunk_t.
 */
typedef struct chunk
{
	const char* buffer;
	int offset;
	char opposite_char;
} chunk_t;


/**
 * FUNCTIONS
 */

/**
 * Prints a given message to STDOUT.
 * @param message String to be printed to STDOUT.
 */
void message(const char* message);

/**
 * Prints a given message to STDOUT and
 * exits the program with a value of
 * EXIT_FAILURE.
 * @param message String to be printed to STDOUT.
 */
void die_with_message(const char* msg);

/**
 * Checks if a given path points to a directory
 * @param path
 * @return 1 If path points to a directory. Otherwise, 0.
 */
int is_dir(const char* path);

/**
 * Traverses the whole file and
 * checks for any non US-ASCII
 * characters. At the end, rewinds
 * the given file pointer to it's
 * start.
 * @param file File pointer to be checked.
 * @return 0 if one or more invalid characters are found. Otherwise, 1.
 */
int is_ascii_file(FILE* file);

/**
 * Checks if a given character is a word separator.
 * In our case, word separators are the chars:
 * 1. ' '
 * 2. '\n'
 * 3. '\t'
 * 4. '\0'
 * @param c The character to be checked.
 * @return 1 if the character is a word separator. Otherwise, 0.
 */
int is_wordsep(const char* c);

/**
 * Counts the number of words in the context of a given chunk_t.
 * Essentially, it counts as a "word" the transition between
 * a non-line-separator character to a line separator.
 * The check is done in pairs up to the third-to-last character.
 * There, the check is done between the second-to-last and opposite character
 * (= the first character of the next thread's chunk to solve the split-word
 * problem.
 * @param chnk The chunk_t to be used.
 * @return The number of words that were counted in the given chunk_t.
 */
void* wc(void* chnk);

/**
 * Opens a file and determines if it's a US-ASCII formatted one.
 * If yes, then reads it in chunks and:
 * 1. Divides each chunk between the available threads in the most equal way and run wc on them.
 * 2. Or decides that each chunk would be too small to be split so a single-threaded approach is selected.
 * @param path
 * @return The number of words that were counted in the given file.
 */
int multiwc(const char path[]);


#endif //MULTIWC_MULTIWC_H
