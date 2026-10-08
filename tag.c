#include "tag.h"

unsigned int bigtolittle(const unsigned char *str)             // Converts a 4-byte big-endian byte array from ID3 frame headers into a native unsigned integer
{
    return ((unsigned int)str[0] << 24) |
           ((unsigned int)str[1] << 16) |
           ((unsigned int)str[2] << 8)  |
           ((unsigned int)str[3]);
}

void view(char *str)                                           // Validates the .mp3 file, parses the 10-byte ID3v2 header, and prints all recognized metadata tags
{
    int len = strlen(str);
    if (len < 4 || strcmp(str + len - 4, ".mp3") != 0)
    {
        printf("Error: Enter a valid .mp3 file name.\n");
        return;
    }

    FILE *fptr = fopen(str, "rb");
    if (!fptr)
    {
        perror("File opening failed");
        return;
    }

    unsigned char header[10];
    if (fread(header, 1, 10, fptr) != 10)
    {
        printf("Error: File is too small to contain a valid ID3 header.\n");
        fclose(fptr);
        return;
    }

    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("Error: Not a valid ID3 tagged file.\n");
        fclose(fptr);
        return;
    }

    printf("MP3 Tag Reader & Editor:\n");
    printf("------------------------------------------------------------\n");
    printf("Version ID: %d.%d\n", header[3], header[4]);
    printf("------------------------------------------------------------\n");

    unsigned int total_tag_size = ((header[6] & 0x7F) << 21) |
                                  ((header[7] & 0x7F) << 14) |
                                  ((header[8] & 0x7F) << 7)  |
                                  (header[9] & 0x7F);

    unsigned int bytes_read = 0;
    while (bytes_read < total_tag_size)
    {
        char tag[5];
        unsigned char size_buf[4];
        unsigned char flags[2];

        if (fread(tag, 1, 4, fptr) != 4) break;
        bytes_read += 4;
        tag[4] = '\0';

        if (tag[0] == '\0' || 
            !((isupper((unsigned char)tag[0]) || isdigit((unsigned char)tag[0])) &&
              (isupper((unsigned char)tag[1]) || isdigit((unsigned char)tag[1])) &&
              (isupper((unsigned char)tag[2]) || isdigit((unsigned char)tag[2])) &&
              (isupper((unsigned char)tag[3]) || isdigit((unsigned char)tag[3]))))
        {
            break;
        }

        if (fread(size_buf, 1, 4, fptr) != 4) break;
        bytes_read += 4;

        if (fread(flags, 1, 2, fptr) != 2) break;
        bytes_read += 2;

        unsigned int frame_size = bigtolittle(size_buf);
        if (frame_size == 0 || frame_size > (total_tag_size - bytes_read))
        {
            break;
        }

        char *label = NULL;
        int is_comment = 0;

        if (strcmp(tag, "TIT2") == 0)      label = "TITLE";
        else if (strcmp(tag, "TPE1") == 0) label = "ARTIST";
        else if (strcmp(tag, "TALB") == 0) label = "ALBUM";
        else if (strcmp(tag, "TYER") == 0) label = "YEAR";
        else if (strcmp(tag, "TRCK") == 0) label = "TRACK";
        else if (strcmp(tag, "TCON") == 0) label = "GENRE";
        else if (strcmp(tag, "COMM") == 0)
        {
            label = "COMMENT";
            is_comment = 1;
        }

        if (label == NULL)
        {
            fseek(fptr, frame_size, SEEK_CUR);
            bytes_read += frame_size;
            continue;
        }

        char *payload = malloc(frame_size + 1);
        if (!payload) break;

        if (fread(payload, 1, frame_size, fptr) != frame_size)
        {
            free(payload);
            break;
        }
        payload[frame_size] = '\0';
        bytes_read += frame_size;

        int min_req = is_comment ? 5 : 1;
        char *display_str = (frame_size > (unsigned int)min_req) ? (is_comment ? (payload + 5) : (payload + 1)) : "";

        printf("+%.*s+\n| %s :: %s |\n+%.*s+\n",
               (int)(strlen(label) + strlen(display_str) + 6),
               "------------------------------------------------------------",
               label, display_str,
               (int)(strlen(label) + strlen(display_str) + 6),
               "------------------------------------------------------------");

        free(payload);
    }

    fclose(fptr);
}

void editor(char *tag, char *new_val, FILE *temp)              // Encodes the new text value into the proper ID3 frame structure and writes it to the temporary file
{
    unsigned int size = (strcmp(tag, "COMM") == 0) ? (strlen(new_val) + 5) : (strlen(new_val) + 1);

    unsigned char big[4];
    big[0] = (size >> 24) & 0xFF;
    big[1] = (size >> 16) & 0xFF;
    big[2] = (size >> 8)  & 0xFF;
    big[3] = size & 0xFF;

    unsigned char flags[2] = {0x00, 0x00};
    fwrite(big, 1, 4, temp);
    fwrite(flags, 1, 2, temp);

    if (strcmp(tag, "COMM") == 0)
    {
        fputc(0x00, temp);
        fwrite("eng", 1, 3, temp);
        fputc(0x00, temp);
        fwrite(new_val, 1, strlen(new_val), temp);
    }
    else
    {
        fputc(0x00, temp);
        fwrite(new_val, 1, strlen(new_val), temp);
    }
}

int edit(char *ch, char *new_val, char *file)                  // Copies the MP3 stream to a temporary file while replacing the specified tag, then updates the header size
{
    if (strcmp(file, "temp.mp3") == 0)
    {
        printf("Error: Target file cannot be named temp.mp3\n");
        return 0;
    }

    FILE *org = fopen(file, "rb");
    if (!org)
    {
        perror("Error opening source file");
        return 0;
    }

    FILE *temp = fopen("temp.mp3", "wb");
    if (!temp)
    {
        perror("Error creating temporary file");
        fclose(org);
        return 0;
    }

    unsigned char header[10];
    if (fread(header, 1, 10, org) != 10)
    {
        printf("Error: Invalid MP3 header\n");
        fclose(org);
        fclose(temp);
        remove("temp.mp3");
        return 0;
    }

    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("Error: Not an ID3v2 tagged file\n");
        fclose(org);
        fclose(temp);
        remove("temp.mp3");
        return 0;
    }

    unsigned int orig_tag_size = ((header[6] & 0x7F) << 21) |
                                 ((header[7] & 0x7F) << 14) |
                                 ((header[8] & 0x7F) << 7)  |
                                 (header[9] & 0x7F);

    fwrite(header, 1, 10, temp);

    unsigned int bytes_read = 0;
    int size_diff = 0;
    int tag_found = 0;

    while (bytes_read < orig_tag_size)
    {
        char tag[5];
        unsigned char size_buf[4];
        unsigned char flags[2];

        if (fread(tag, 1, 4, org) != 4) break;
        bytes_read += 4;
        tag[4] = '\0';

        if (tag[0] == '\0' || 
            !((isupper((unsigned char)tag[0]) || isdigit((unsigned char)tag[0])) &&
              (isupper((unsigned char)tag[1]) || isdigit((unsigned char)tag[1])) &&
              (isupper((unsigned char)tag[2]) || isdigit((unsigned char)tag[2])) &&
              (isupper((unsigned char)tag[3]) || isdigit((unsigned char)tag[3]))))
        {
            fseek(org, -4, SEEK_CUR);
            bytes_read -= 4;
            while (bytes_read < orig_tag_size)
            {
                int ch_pad = fgetc(org);
                if (ch_pad == EOF) break;
                fputc(ch_pad, temp);
                bytes_read++;
            }
            break;
        }

        if (fread(size_buf, 1, 4, org) != 4) break;
        bytes_read += 4;

        if (fread(flags, 1, 2, org) != 2) break;
        bytes_read += 2;

        unsigned int lit_size = bigtolittle(size_buf);
        if (lit_size == 0 || lit_size > (orig_tag_size - bytes_read))
        {
            break;
        }

        unsigned char *str = malloc(lit_size);
        if (!str) break;

        if (fread(str, 1, lit_size, org) != lit_size)
        {
            free(str);
            break;
        }
        bytes_read += lit_size;

        if ((strcmp(ch, "-t") == 0 && strcmp(tag, "TIT2") == 0) ||
            (strcmp(ch, "-T") == 0 && strcmp(tag, "TRCK") == 0) ||
            (strcmp(ch, "-a") == 0 && strcmp(tag, "TPE1") == 0) ||
            (strcmp(ch, "-A") == 0 && strcmp(tag, "TALB") == 0) ||
            (strcmp(ch, "-g") == 0 && strcmp(tag, "TCON") == 0) ||
            (strcmp(ch, "-y") == 0 && strcmp(tag, "TYER") == 0) ||
            (strcmp(ch, "-c") == 0 && strcmp(tag, "COMM") == 0))
        {
            printf("Updating frame: %s\n", tag);
            fwrite(tag, 1, 4, temp);
            int new_size = (strcmp(tag, "COMM") == 0) ? (strlen(new_val) + 5) : (strlen(new_val) + 1);
            size_diff += (new_size - (int)lit_size);
            editor(tag, new_val, temp);
            tag_found = 1;
        }
        else
        {
            fwrite(tag, 1, 4, temp);
            fwrite(size_buf, 1, 4, temp);
            fwrite(flags, 1, 2, temp);
            fwrite(str, 1, lit_size, temp);
        }
        free(str);
    }

    if (!tag_found)
    {
        printf("Note: Tag '%s' was not found in the file to edit.\n", ch);
    }

    fseek(org, 10 + orig_tag_size, SEEK_SET);

    unsigned char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), org)) > 0)
    {
        if (fwrite(buffer, 1, bytes, temp) != bytes)
        {
            printf("Error writing audio stream to temp file\n");
            break;
        }
    }

    unsigned int updated_tag_size = orig_tag_size + size_diff;
    header[6] = (updated_tag_size >> 21) & 0x7F;
    header[7] = (updated_tag_size >> 14) & 0x7F;
    header[8] = (updated_tag_size >> 7)  & 0x7F;
    header[9] = updated_tag_size & 0x7F;

    fseek(temp, 0, SEEK_SET);
    fwrite(header, 1, 10, temp);

    fclose(org);
    fclose(temp);

    if (remove(file) != 0)
    {
        perror("Error removing original file");
        return 0;
    }

    if (rename("temp.mp3", file) != 0)
    {
        perror("Error renaming temporary file");
        return 0;
    }

    printf("Changes saved successfully to %s\n", file);
    return 1;
}