/*
RIAS 1.0

@ Regiel Glenn Salac (Lunarevex)

RIAS is the assembler and simulator for the R programming language that can output into a hex file compatible with
LOGISIM (v2.0 raw) or directly run .r files.

RIAS uses C as it's base language, intended to be simple to read but fast to compile and run programs.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Instructions, --unlimited, and amount of instructions
uint32_t instructions[65536] = {0};
int instr_count = 0;
int accuracy = 1;

// The four registers + the adder (pc)
uint16_t regs[4] = { 0 };
int pc = 0;

FILE *file;
FILE *rfile;


// Helper
int reg_num(const char *s) {
    if (s != NULL && s[0] == 'R' && s[1] >= '0' && s[1] <= '3' && s[2] == '\0') {
        return s[1] - '0';
    }
    printf("Syntax Error: Invalid register '%s'\n", s ? s : "NULL");
    return 0;
}

void emit(int we, int ig, int op, int ds, int cn) {
    if (accuracy && instr_count >= 256) {
        printf("Error: No --unlimited flag, exceeded 256-bit ROM limit.\n");
        exit(1);
    }
    if (accuracy && we == 0 && ig == 1 && cn > 63) {
        printf("Warning: Jump address %d exceeds 6-bit CONST6 limit (0-63)!\n", cn);
    }

    if (accuracy) {
        instructions[instr_count++] = ((we & 1) << 11) | ((ig & 1) << 10) | ((op & 3) << 8) | ((ds & 3) << 6) | (cn & 0x3F);
    } else {
        instructions[instr_count++] = ((we & 1) << 11) | ((ig & 1) << 10) | ((op & 3) << 8) | ((ds & 3) << 6) | (cn & 0x3F) | ((cn & 0xFFFF) << 12);
    }
}

/*
Parser (.r) | NOTE: Many parts that involve the Helpers and the Parser are AI-assisted as I mainly wanted to focus
on converting the CPU instructions into code rather than spend billion years setting up the conversion.
Though compile() and the arithmetic functions were made by me (since it wasn't difficult to do).
*/
int parser(const char *filename) {
    char sentence[256];
    int position = 0;
    int loop_stack[32];
    int loop_top = -1;
    int if_stack[32]; 
    int jump_count[32];
    int if_top = -1;

    rfile = fopen(filename, "r");
    if (rfile == NULL) {
        printf("File doesn't exist");
        return 1;
    }

    
    int character;
    while ((character = fgetc(rfile)) != EOF) {
        if (character == '/') {
            int next = fgetc(rfile);
            if (next == '/') {
                while ((character = fgetc(rfile)) != EOF && character != '\n') {
                    // pass
                }
                continue;
            }
        }
        if (character == '.') {
            sentence[position] = '\0';

            char w1[32] = "";
            char w2[32] = "";
            char w3[32] = "";
            char w4[32] = "";
            sscanf(sentence, "%s %s %s %s", w1, w2, w3, w4);

            if (strcmp(w1, "SETC") == 0) {
                int rx = reg_num(w2);
                emit(1, 1, 2, rx, 0);
                emit(1, 1, 3, rx, atoi(w4));
            }
            else if (strcmp(w1, "COPY") == 0) {
                int rx = reg_num(w2);
                int ry = reg_num(w4);
                emit(1, 1, 2, rx, 0);
                emit(1, 0, 0, rx, ry); 
            }
            else if (strcmp(w1, "CLER") == 0) {
                emit(1, 1, 2, reg_num(w2), 0);
            }
            else if (strcmp(w1, "NEGT") == 0) {
                int rx = reg_num(w2);
                if (rx == 3) {
                    printf("Error: Don't use R3.\n");
                    fclose(rfile);
                    return 1;
                }
                emit(1, 1, 2, 3, 0); 
                emit(1, 0, 1, 3, rx); 
                emit(1, 1, 2, rx, 0);
                emit(1, 0, 0, rx, 3); 
            }

            else if (strcmp(w1, "ADDN") == 0) {
                emit(1, 0, 0, reg_num(w2), reg_num(w3));
            }
            else if (strcmp(w1, "SUBN") == 0) {
                emit(1, 0, 1, reg_num(w2), reg_num(w3));
            }
            else if (strcmp(w1, "ANDN") == 0) {
                emit(1, 0, 2, reg_num(w2), reg_num(w3));
            }
            else if (strcmp(w1, "ORON") == 0) {
                emit(1, 0, 3, reg_num(w2), reg_num(w3));
            }


            else if (strcmp(w1, "ADDC") == 0) {
                emit(1, 1, 0, reg_num(w2), atoi(w3));
            }
            else if (strcmp(w1, "SUBC") == 0) {
                emit(1, 1, 1, reg_num(w2), atoi(w3));
            }
            else if (strcmp(w1, "ANDC") == 0) {
                emit(1, 1, 2, reg_num(w2), atoi(w3));
            }
            else if (strcmp(w1, "OROC") == 0) {
                emit(1, 1, 3, reg_num(w2), atoi(w3));
            }

            else if (strcmp(w1, "DOIF") == 0) {
                int rx = reg_num(w3);
                if_top++;
                if_stack[if_top] = instr_count;
                
                if (strcmp(w2, "GRET") == 0) {
                    emit(0, 1, 1, rx, 0); 
                    emit(0, 1, 2, rx, 0);
                    jump_count[if_top] = 2;
                }
                else if (strcmp(w2, "LESS") == 0) {
                    emit(0, 1, 0, rx, 0); 
                    emit(0, 1, 2, rx, 0);
                    jump_count[if_top] = 2;
                }
                else if (strcmp(w2, "YEQL") == 0) {
                    emit(0, 1, 3, rx, 0);
                    jump_count[if_top] = 1;
                }
                else if (strcmp(w2, "NEQL") == 0) {
                    emit(0, 1, 2, rx, 0);
                    jump_count[if_top] = 1;
                }
            }
            else if (strcmp(w1, "ENDN") == 0) {
                if (if_top >= 0) {
                    int patch_idx = if_stack[if_top];
                    int count = jump_count[if_top];
                    if_top--;

                    uint32_t patch = accuracy ? (instr_count & 0x3F) : ((instr_count & 0x3F) | ((uint32_t)(instr_count & 0xFFFF) << 12));
                    instructions[patch_idx] |= patch;
                    if (count == 2) {
                        instructions[patch_idx + 1] |= patch;
                    }
                }
            }

            else if (strcmp(w1, "LOOP") == 0) {
                loop_stack[++loop_top] = instr_count;
            }
            else if (strcmp(w1, "ENZO") == 0) { 
                int target = loop_stack[loop_top--];
                emit(0, 1, 2, reg_num(w2), target);
            }
            else if (strcmp(w1, "ENNZ") == 0) { 
                int target = loop_stack[loop_top--];
                emit(0, 1, 3, reg_num(w2), target);
            }
            else if (strcmp(w1, "ENNG") == 0) {
                int target = loop_stack[loop_top--];
                emit(0, 1, 0, reg_num(w2), target);
            }
            else if (strcmp(w1, "ENNL") == 0) {
                int target = loop_stack[loop_top--];
                emit(0, 1, 1, reg_num(w2), target);
            }

            else if (strcmp(w1, "PRNT") == 0) {
                emit(0, 0, 0, reg_num(w2), 0);
            }

            else if (strcmp(w1, "HALT") == 0) {
                int halt_addr = instr_count;
                emit(0, 1, 2, 0, halt_addr);
                emit(0, 1, 3, 0, halt_addr);
            }

            position = 0;
        }
        else if (position < 255) {
            sentence[position] = character;
            position++;
        }
    }

    fclose(rfile);
    return 0;
}

// Operations

uint16_t arimath(int op, uint16_t a, uint16_t b) {
    int result = 0;

    switch (op) {
        case 0: result = (a + b) & 0xFFF; break;
        case 1: result = (a - b) & 0xFFF; break;
        case 2: result = (a & b) & 0xFFF; break;
        case 3: result = (a | b) & 0xFFF; break;
    }

    return result;
}

uint16_t jumpmath(int op, uint16_t a, uint16_t b) {
    int16_t sa = (a & 0x800) ? (a | 0xF000) : a;
    switch (op) {
        case 0: return (sa > 0) & 0xFFF; break;
        case 1: return (sa < 0) & 0xFFF; break;
        case 2: return (sa == 0) & 0xFFF; break;
        case 3: return (sa != 0) & 0xFFF; break;
    }
    return 0;
}

// Assembler
int compile() {
    
    int clock = instr_count;
    while (pc < clock) {
        int jumped = 0;
        uint32_t instance = instructions[pc];
        
        uint8_t we = (instance >> 11) & 1;
        uint8_t ig = (instance >> 10) & 1;
        uint8_t op = (instance >> 8) & 3;
        uint8_t ds = (instance >> 6) & 3;
        uint16_t cn = accuracy ? (instance & 63) : ((instance >> 12) & 0xFFFF);
        uint8_t sr = cn & 3;

        switch (we) {
            case 0:
                switch (ig) {
                    case 0:
                        if (op == 0) {
                            printf("%c", regs[ds] & 0x7F);
                        }
                        break;
                    case 1:
                        if (jumpmath(op, regs[ds], 0)) {
                            if (cn == pc) return 0;
                            jumped = 1;
                            pc = cn;
                        }

                        break;
                }
                break;
            case 1:
                switch (ig) {
                    case 0:
                        regs[ds] = arimath(op, regs[ds], regs[sr]); break;
                    case 1:
                        regs[ds] = arimath(op, regs[ds], cn); break;
                }
                break;
            
        }
        if (!jumped) { pc++; };
    }

    return 0;
}

// checks for program.r by default
int main(int argc, char *argv[]){
    const char *input_file = "program.r";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--unlimited") == 0) {
            accuracy = 0;
        } 
        else {
            input_file = argv[i];
        }
    }

    if (parser(input_file) != 0) {
        return 1;
    }

    if (accuracy != 0) {
        file = fopen("output.hex", "w");
        if (file == NULL) {
            printf("File doesn't exist");
            return 1;
        }

        fprintf(file, "v2.0 raw\n");
    
        for (int i = 0; i < instr_count; i++) {
            fprintf(file, "%03X\n", instructions[i]);
        }
    
        fclose(file);
    }

    compile();
    printf("Result\n R0: %03X | R1: %03X | R2: %03X | R3: %03X\n", regs[0], regs[1], regs[2], regs[3]);
    return 0;
}

