/*
 * Uses a .torrent metainfo file to break up a file into pieces
 * and to reconstruct pieces into a file.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include "file_constructor.h"


// void *set_bitfield(int *fd, char *file_name, char *bitfield, int num_pieces, int piece_len, char *piece_shas) {
// 	// check if file exists
// 	if (access(file_name, F_OK) != -1) {
// 	    // file exists
// 	    char expected_piece_sha[SHA_DIGEST_LENGTH];
// 	    for (int i = 0; i < num_pieces; i++) {
// 	    	// save the piece
// 			char piece[piece_len] = get_piece(fd, i, piece_len);
// 			memcpy(expected_piece_sha, &piece_shas[i * (piece_len + 1)], piece_len);
// 			if (verify_piece(piece, expected_piece_sha, piece_len)) {
// 				// add to bitfield
// 				bitfield[i] = 1;
// 			}
// 			// unsigned char hash[SHA_DIGEST_LENGTH];
// 			// SHA1(piece, piece_len, hash);
// 			// char expected_piece_sha[SHA_DIGEST_LENGTH];
// 			// if (strcmp(hash, piece_shas[i])) {
// 			// 	// add to bitfield
// 			// }
// 		}

// 	} else {
// 		// file doesn't exist
// 		// open up file for reading and writing
// 		if ((fd = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
// 		        fprintf(stderr, "Error opening file to torrent");
// 		}
// 	}
// }


/*
 * Verifies if string has an expected SHA has.
 *
 * piece[]: the string to verify
 * expected_sha[]: the expected SHA has
 * piece_length: the length of piece
 *
 * returns: 0 if the piece is not correct, 1 if the piece is verified
 */
int verify_piece(char piece[], unsigned char expected_sha[], int piece_length) {
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(piece, piece_length, hash);
	return !memcmp(hash, expected_sha, SHA_DIGEST_LENGTH);
}

/*
 * Get a piece from a file
 *
 * fd: file descriptor
 * piece_num: which piece to get from the file (0 indexed)
 * piece_len: length of pieces
 *
 * returns: char array of the piece data
 */
char *get_piece(int fd, int piece_num, int piece_len) {

	char buffer[piece_len];
	// seek to correct position in the file
	if (lseek(fd, piece_num * piece_len, SEEK_SET) < 0){
		// error seeking
		exit(-1);
	}

	// read from file into buffer
	if (read(fd, buffer, piece_len) != piece_len) {
		exit(-1);
	}

	printf("%s\n", buffer);
	//char *piece = (char *) mallloc(sizeof(char) * piece_len);
	return strcpy(malloc(sizeof(char) * piece_len), buffer);
	//return piece;
}

/*
 * Write to a file from a buffer
 *
 * fd: file descriptor
 * piece_num: what number piece will be written
 * piece_len: length of the piece being written
 * buffer: char array of what will be written
 */
void write_piece(int fd, int piece_num, int piece_len, char *buffer) {
	// seek to correct position in the file
	if (lseek(fd, piece_num * piece_len, SEEK_SET) < 0){
		// error seeking
		exit(-1);
		return;
	}

	if (write(fd, strcat(buffer, "\n"), piece_len) != piece_len) {
		printf("Success writing to the file!!!!!\n");
		return;
	}
}

int main(int argc, char *argv[]) {
	// int fd;
	// if ((fd = open("test.txt", O_RDONLY)) < -1) {
	// 	// error opening file
	// 	return 1;
	// }
	
	// int fd_write;
	// if ((fd_write = open("write.txt", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
	// 	// error opening write file
	// 	return 1;
	// }
	
	// char* piece = get_piece(fd, 1, 5);
	// printf("%s\n", piece);
	// write_piece(fd_write, 1, 5, piece);

	
	unsigned char expect[SHA_DIGEST_LENGTH];
	char data[] = "hi";
	char data2[] = "yo";
	char data3[] = "hi";
	size_t length = sizeof(data);
	SHA1(data, length, expect);
	int i = verify_piece(data2, expect, length);
	printf("%i", i);

	return 0;
}
