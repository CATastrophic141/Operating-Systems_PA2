/**
 * @file   test.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the charkmod.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/charkmod.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 *
 * Adapted for COP 4600 by Dr. John Aedo
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_LENGTH 5000           ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH]; ///< The receive buffer from the LKM

// Function declarations for different testing methods
int UserInputTesting(int argc, char *argv[]);
int BruteForceMaxLengthTesting(int argc, char *argv[]);
int RandomSequenceGenerationTesting(int argc, char *argv[]);
int EmptyStringReadWriteTesting(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    // Select which testing method to run on the kernel
    //UserInputTesting(argc, argv);
    //BruteForceMaxLengthTesting(argc, argv);
    RandomSequenceGenerationTesting(argc, argv);
    //EmptyStringReadWriteTesting(argc, argv);

    // End the program
    printf("End of the program\n");
    return 0;
}


/*
--------------------------------
Below are functions used for different methods of testing the kernel.
*/


/*
Test the kernel by having the user input a string and receive the same string
*/
int UserInputTesting(int argc, char *argv[]) 
{
    // Device Setup
    if (argc != 2)
    {
        printf("Usage: test <path to device>\n");
        exit(0);
    }
    char *devicepath = argv[1];
    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    printf("Starting device test code example...\n");
    fd = open(devicepath, O_RDWR); // Open the device with read/write access
    if (fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }

    // Read user input and write to buffer
    printf("Type in a short string to send to the kernel module:\n");
    scanf("%[^\n]%*c", stringToSend); // Read in a string (with spaces)
    printf("Writing message to the device [%s].\n", stringToSend);
    ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Wait for user
    printf("Press ENTER to read back from the device...\n");
    getchar();


    // Read from buffer
    printf("Reading from the device...\n");
    ret = read(fd, receive, BUFFER_LENGTH); // Read the response from the LKM
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("The received message is: [%s]\n", receive);

    return 0;
}

/*
Test the kernel by inputting a larger string than the buffer size, and manipulating it in similar ways.
Input string: a[x499] + b[x500] + c[x4001]
Correct process:
    Write input string (should store first 1024 characters)
    Read 500 bytes: Print a[x499] + b[x1]
    Read 500 bytes: Print b[x499] + c[x1]
    Write input string (should store first 1000 characters)
    Read 2000 bytes: Print c[x24] + a[x499] + b[x500] + c[x1]
*/
int BruteForceMaxLengthTesting(int argc, char *argv[])
{
    // Device Setup
    if (argc != 2)
    {
        printf("Usage: test <path to device>\n");
        exit(0);
    }
    char *devicepath = argv[1];
    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    printf("Starting device brute force test code example...\n");
    fd = open(devicepath, O_RDWR); // Open the device with read/write access
    if (fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }

    // Create stringToSend variable
    int i;
    for (i = 0; i < 499; i++) {
        stringToSend[i] = 'a';
    }
    for (i = 499; i < 999; i++) {
        stringToSend[i] = 'b';
    }
    for (i = 999; i < BUFFER_LENGTH; i++) {
        stringToSend[i] = 'c';
    }

    // Write stringToSend
    printf("Writing message to the device [%s].\n", stringToSend);
    ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }


    // Read stringToSend (x2) with max length 500
    printf("Reading from the device...\n");
    ret = read(fd, receive, 500); // Read the response from the LKM
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("The received message is: [%s]\n", receive);
    printf("Reading from the device...\n");
    ret = read(fd, receive, 500); // Read the response from the LKM
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("The received message is: [%s]\n", receive);


    // Write stringToSend
    printf("Writing message to the device [%s].\n", stringToSend);
    ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }


    // Read stringToSend at size 2000
    printf("Reading from the device...\n");
    ret = read(fd, receive, 2000); // Read the response from the LKM
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("The received message is: [%s]\n", receive);

    return 0;
}


/*
Have the system randomly create read/write requests.
Input sizes: [0,1100] (to promote randomness while also allowing for the buffer to read the maximum size {1024} frequently)
Random process of writes and reads assigned, everything printed to the user.
*/
int RandomSequenceGenerationTesting(int argc, char *argv[])
{
    // Device Setup
    if (argc != 2)
    {
        printf("Usage: test <path to device>\n");
        exit(0);
    }
    char *devicepath = argv[1];
    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    srand(time(NULL)); // Seed for random number generation
    printf("Starting random sequence generation testing...\n");
    fd = open(devicepath, O_RDWR); // Open the device with read/write access
    if (fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }

    // Generate random read/write requests
    int i;
    int testingBufferSize = 0;
    for (i = 0; i < 20; ++i) // Perform 20 random operations
    {
        printf("\n\n");
        int size = rand() % 1101; // Random size between 0 and 1100
        if (rand() % 10 == 0) { // 1/10 chance to set size to 0
            size = 0;
        }
        int readOrWrite = rand() % 2; // Randomly choose read (0) or write (1)

        if (readOrWrite == 0) // Read operation
        {
            printf("Reading %d bytes from the device...\n", size);
            ret = read(fd, receive, size); // Read from the device
            if (ret < 0)
            {
                perror("Failed to read the message from the device.");
                return errno;
            }
            receive[ret] = '\0'; // Null-terminate the received data for printing
            printf("Received message: [%s]\n", receive);
            printf("Received message length: %ld\n", strlen(receive));

            // Update testing buffer size
            testingBufferSize -= size;
            testingBufferSize = testingBufferSize >= 0 ? testingBufferSize : 0;
            printf("Current Buffer Size: %d\n", testingBufferSize);
        }
        else // Write operation
        {
            for (int j = 0; j < size; ++j)
            {
                stringToSend[j] = 'a' + (rand() % 26); // Random lowercase alphabet character
            }
            stringToSend[size] = '\0'; // Null-terminate the string
            printf("Writing message to the device [%s].\n", stringToSend);
            ret = write(fd, stringToSend, strlen(stringToSend)); // Write to the device
            if (ret < 0)
            {
                perror("Failed to write the message to the device.");
                return errno;
            }

            // Update testing buffer size
            testingBufferSize += size;
            testingBufferSize = testingBufferSize <= 1024 ? testingBufferSize : 1024;
            printf("Current Buffer Size: %d\n", testingBufferSize);
        }
    }

    return 0;
}


/*
Have the kernel test empty cases.
    1) Write an empty string (length = 0)
    2) Read for 1 byte
    3) Write a length 1 string (Character = "A")
    4) Read for 0 bytes
*/
int EmptyStringReadWriteTesting(int argc, char *argv[])
{
    // Device Setup
    if (argc != 2)
    {
        printf("Usage: test <path to device>\n");
        exit(0);
    }
    char *devicepath = argv[1];
    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    printf("Starting empty string read/write testing...\n");
    fd = open(devicepath, O_RDWR); // Open the device with read/write access
    if (fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }

    // Write an empty string (length = 0)
    printf("Writing empty string to the device.\n");
    ret = write(fd, "", 0);
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Read for 1 byte
    char singleByte;
    printf("Reading 1 byte from the device...\n");
    ret = read(fd, &singleByte, 1);
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("Received byte: [%c]\n", singleByte);

    // Write a length 1 string (Character = "A")
    printf("Writing 'A' to the device.\n");
    ret = write(fd, "A", 1);
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Read for 0 bytes
    printf("Reading 0 bytes from the device...\n");
    ret = read(fd, receive, 0);
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("No data received.\n");

    return 0;
}
