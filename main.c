#include "tag.h"

int main(int argc, char *argv[])                                // Validates command-line flags and dispatches execution to either the view, edit, or help handlers
{
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("  View: %s -v <filename.mp3>\n", argv[0]);
        printf("  Edit: %s -e <flag> \"<new_value>\" <filename.mp3>\n", argv[0]);
        printf("Flags: -t (Title), -T (Track), -a (Artist), -A (Album), -y (Year), -g (Genre), -c (Comment)\n");
        return 1;
    }

    if (strcmp(argv[1], "-v") == 0)
    {
        if (argc < 3)
        {
            printf("Error: Missing MP3 file name.\n");
            return 1;
        }
        view(argv[2]);
    }
    else if (strcmp(argv[1], "-e") == 0)
    {
        if (argc < 5)
        {
            printf("Error: Missing arguments for edit.\n");
            printf("Usage: %s -e <flag> \"<new_value>\" <filename.mp3>\n", argv[0]);
            return 1;
        }
        edit(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "-h") == 0)
    {
        printf("------------- HELP MENU -------------\n");
        printf("1. -v --> View MP3 file tags\n");
        printf("2. -e --> Edit MP3 file tags\n");
        printf("   -t --> Title\n");
        printf("   -T --> Track Number\n");
        printf("   -a --> Artist Name\n");
        printf("   -A --> Album Name\n");
        printf("   -y --> Year\n");
        printf("   -g --> Genre\n");
        printf("   -c --> Comment\n");
        return 0;
    }
    else
    {
        printf("Error: Invalid operation flag '%s'. Use -v, -e, or -h.\n", argv[1]);
        return 1;
    }

    return 0;
}