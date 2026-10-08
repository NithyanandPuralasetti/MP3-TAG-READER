#ifndef TAG_H
#define TAG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void view(char *str);                                           // Validates the .mp3 file, parses the 10-byte ID3v2 header, and prints all recognized metadata tags
int edit(char *ch, char *new_val, char *file);                  // Copies the MP3 stream to a temporary file while replacing the specified tag, then updates the header size
void editor(char *tag, char *new_val, FILE *temp);              // Encodes the new text value into the proper ID3 frame structure and writes it to the temporary file
unsigned int bigtolittle(const unsigned char *str);             // Converts a 4-byte big-endian byte array from ID3 frame headers into a native unsigned integer

#endif