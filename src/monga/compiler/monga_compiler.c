#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "monga.y.h"
#include "monga_ast.h"
#include "monga_ast_bind.h"
#include "monga_ast_llvm.h"
#include "monga_ast_destroy.h"

extern FILE* yyin;

void yyerror(const char* err)
{
    fprintf(stderr, "%s (line %zu)\n", err, monga_get_lineno());
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s {input-file-name}\n", argv[0]);
        return 1;
    }

    const char* input_file_name = argv[1];
    FILE* input_file = fopen(input_file_name, "r");
    if (input_file == NULL) {
        fprintf(stderr, "Could not open %s\n", input_file_name);
        return 1;
    }

    yyin = input_file;

    int result = yyparse();

    fclose(input_file);

    if (result == 0) {
        char* llvm_file_name = malloc(strlen(input_file_name) + 4);
        if (llvm_file_name == NULL) {
            fprintf(stderr, "Could not allocate memory for llvm file name\n");
            return 1;
        }

        strcpy(llvm_file_name, input_file_name);
        strcat(llvm_file_name, ".ll");

        FILE* llvm_file = fopen(llvm_file_name, "w");
        if (llvm_file == NULL) {
            fprintf(stderr, "Could not open %s\n", llvm_file_name);
            free(llvm_file_name);
            return 1;
        }

        monga_ast_program_bind(root);
        monga_ast_program_llvm(root, llvm_file);
        monga_ast_program_destroy(root);

        fclose(llvm_file);

        char* command = malloc(strlen(llvm_file_name) + 5);
        if (command == NULL) {
            fprintf(stderr, "Could not allocate memory for llc command\n");
            return 1;
        }

        strcpy(command, "llc ");
        strcat(command, llvm_file_name);

        free(llvm_file_name);

        int errcode = system(command);
        if (errcode != 0) {
            fprintf(stderr, "Command \"%s\" exited with error code %d\n", command, errcode);
            free(command);
            return errcode;
        }
    
        free(command);

        char* asm_file_name = malloc(strlen(input_file_name) + 3);
        if (asm_file_name == NULL) {
            fprintf(stderr, "Could not allocate memory for asm file name\n");
            return 1;
        }

        strcpy(asm_file_name, input_file_name);
        strcat(asm_file_name, ".s");

        command = malloc(strlen(asm_file_name) + 5);
        if (command == NULL) {
            fprintf(stderr, "Could not allocate memory for gcc command\n");
            return 1;
        }

        strcpy(command, "gcc ");
        strcat(command, asm_file_name);

        free(asm_file_name);

        errcode = system(command);
        if (errcode != 0) {
            fprintf(stderr, "Command \"%s\" exited with error code %d\n", command, errcode);
            free(command);
            return errcode;
        }

        free(command);

        command = "./a.out";
        errcode = system(command);
        if (errcode != 0) {
            fprintf(stderr, "Command \"%s\" exited with error code %d\n", command, errcode);
            return errcode;
        }
    }
    
    return result;
}