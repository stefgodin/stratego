#ifndef ERROR_H
#define ERROR_H

#define FATAL_ERR(...) {\
    printf("Unrecoverable error reported at line %d of file %s\n", __LINE__, __FILE__);\
    printf(__VA_ARGS__);\
    printf("\n");\
    exit(1);\
    }

#endif // ERROR_H
