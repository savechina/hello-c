#ifndef FILE_IO_SAMPLE_H
#define FILE_IO_SAMPLE_H

/**
 * @brief File I/O: FILE* operations, fopen/fclose, fread/fwrite,
 *         fprintf/fscanf, fgets, fseek/ftell, ferror, error handling.
 *
 * Demonstrates text and binary file I/O. All file handles are properly closed.
 * No unsafe operations (gets() is never used). All fopen calls are NULL-checked.
 */

int main_file_io_sample(void);

#endif /* FILE_IO_SAMPLE_H */
