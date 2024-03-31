#include <stdio.h>
#include <stdlib.h>
#include <string.h>

short xltable = 0;

typedef struct opfunc {
    char name[16];
    char name0[16];
    char name1[16];
    char name2[16];
    char name3[16];
    char name4[16];
    char name5[16];
    char name6[16];
    char name7[16];
} opfunc;
opfunc ops[8192];

int GenerateJumptable(const char* fnameIn, const char* fnameOut);
int GenerateOps(const char* fnameIn, const char* fnameOut);
int FixOpsAsm(const char* fnameIn, const char* fnameOut);

const char* ignoreops[] = {
// op68klogop.c
"Op0038","Op0078","Op0238","Op0278","Op0a38","Op0a78",
// op68kmisc.c
"Op4840","Op4880","Op48c0", // could be optimized
"Opc140","Opc148","Opc188","Opc340","Opc348","Opc388","Opc540","Opc548","Opc588",
"Opc740","Opc748","Opc788","Opc940","Opc948","Opc988","Opcb40","Opcb48","Opcb88",
"Opcd40","Opcd48","Opcd88","Opcf40","Opcf48","Opcf88","Op4890",
"Op4890","Op48a0","Op48a8","Op48b0","Op48b8","Op48d0","Op48e0","Op48e8","Op48f0",
"Op48f8","Op4c90","Op4c98","Op4ca8","Op4cb0","Op4cb8","Op4cd0","Op4cd8","Op4ce8",
"Op4cf0","Op4cf8",/*"Op4e40","Op4e48",*/"Op4e50","Op4e58","Op4e60","Op4e68","Op4e70","Op4e78",
"Op46c0","Op46d0","Op46d8","Op46e0","Op46e8","Op46f0","Op46f8",
"Op0e10","Op0e18","Op0e20","Op0e28","Op0e30","Op0e38","Op0e50","Op0e58",
"Op0e60","Op0e68","Op0e70","Op0e78","Op0e90","Op0e98","Op0ea0","Op0ea8",
"Op0eb0","Op0eb8",
// op68kmove.c
"Op7000","Op7200","Op7400","Op7600","Op7800","Op7a00","Op7c00","Op7e00",
};

int GetOpFromName(char* name) {
    if (strncmp(name, "Op", 2) != 0) {
        return -1;
    }
    for (int i=0; i<sizeof(ignoreops)/sizeof(ignoreops[0]); i++) {
        if (strcmp(name, ignoreops[i]) == 0) {
            return -1;
        }
    }
    return (int)strtol(name+2, 0, 16);
}


int ShouldExpandOp(int code) {
    if (xltable == 0)
        return 0;
    if (code < 0 || code >= 0x10000)
        return 0;
    if ((code & 7) != 0)
        return 0;
    return 1;
}

int main (int argc, char* argv[])
{
    if (argc < 3) {
        printf("usage: jmptable <out> <in>\n");
        return 0;
    }

    xltable = 1;
    const char* fnameIn = argv[1];
    const char* fnameOut = argv[2];
    //printf("converting %s -> %s\n", fnameIn, fnameOut);

    if (strcmp(strchr(fnameIn, '.'), ".0.s") == 0) {
        return FixOpsAsm(fnameIn, fnameOut);
    }else {
        if (strstr(fnameIn, "op68000.c")) {
            return GenerateJumptable(fnameIn, fnameOut);
        } else if (strstr(fnameIn, "op68k")) {
            return GenerateOps(fnameIn, fnameOut);
        }
    }

    return 0;
}

int FixOpsAsm(const char* fnameIn, const char* fnameOut) {
    FILE* fin  = fopen(fnameIn, "r");
    FILE* fout = fopen(fnameOut, "w");
    if (!fin)  { printf("failed opening %s\n\r", fnameIn); return -1; }
    if (!fout) { printf("failed opening %s\n\r", fnameOut); fclose(fin); return -1; }

    int deleteLabel = 0;
    int text = 1;

    while(1) {
        char* line = 0;
        size_t len = 0;
        ssize_t nread = getline(&line, &len, fin);
        if (nread == -1)
            break;

        if (strstr(line, ".text")) {
            text = 1;
        } else if (strstr(line, ".data")) {
            text = 0;
        } else if (strstr(line, ".bss")) {
            fprintf(fout, "%s", line);
            fprintf(fout, "    .balign 8\n");
            text = 0;
            continue;
        }

        if (text) {
            if (deleteLabel) {
                if (strstr(line, "#APP")) {
                    deleteLabel = 0;
                }
            }
            else
            {
                if (strstr(line, ".globl")) {
                    if (strstr(line, "_COp")) {
                        deleteLabel = 1;
                    } else if (
                        (strstr(line, "_Op")) ||
                        (strstr(line, "_DoIOWB")) ||
                        (strstr(line, "_DoIOWW")) ||
                        (strstr(line, "_DoIORB")) ||
                        (strstr(line, "_DoIORW")))
                    {
                        fprintf(fout, "    .balign 8\n");
                    }
                }
            }

            if (deleteLabel)
                continue;
        }

        fprintf(fout, "%s", line);
            continue;
    }

    fclose(fin);
    fclose(fout);
    return 0;
}


int GenerateOps(const char* fnameIn, const char* fnameOut) {
    FILE* fin  = fopen(fnameIn, "r");
    FILE* fout = fopen(fnameOut, "w");
    if (!fin)  { printf("failed opening %s\n\r", fnameIn); return -1; }
    if (!fout) { printf("failed opening %s\n\r", fnameOut); fclose(fin); return -1; }

    while(1) {
        char* line = 0;
        size_t len = 0;
        ssize_t nread = getline(&line, &len, fin);
        if (nread == -1)
            break;

        if (strncmp(line, "Oper (", 6) != 0 && strncmp(line, "OperS(", 6) != 0) {
            fprintf(fout, "%s", line);
            continue;
        }

        for (int i=0; i<len; i++) { if (line[i] == ',' || line[i] == '(' || line[i] == ')') { line[i] = ' '; } }
        char oper[16], code[16], op[16];
        char decls[16], gets[16], spec1[16];
        char declt[16], declea2[16], calcea2[16];
        char spec2[16], getea2[16], setea2[16], rval[16];
        int args = sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s",
            oper, code, op, decls, gets, spec1, declt, declea2, calcea2, spec2, getea2, setea2, rval);

        if (args != 13) { printf("Error: parse error\n"); break; }
        if (strlen(oper) == 4) { strcat(oper, " "); }

        int num = GetOpFromName(code);
        if (ShouldExpandOp(num)) {
            char* s1 = (strcmp(spec1, "ins7") == 0) ? spec1 : 0;
            char* s2 = (strcmp(spec2, "ins7") == 0) ? spec2 : 0;
            for (int i=0; i<=7; i++) {
                sprintf(code, "Op%04x", num+i);
                if (s1) { sprintf(s1, "%d", i); }
                if (s2) { sprintf(s2, "%d", i); }
                fprintf(fout, "%s(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)\n",
                    oper, code, op, decls, gets, spec1, declt, declea2, calcea2, spec2, getea2, setea2, rval);
            }
        } else {
            fprintf(fout, "%s(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)\n",
                oper, code, op, decls, gets, spec1, declt, declea2, calcea2, spec2, getea2, setea2, rval);
        }

    }

    fclose(fin);
    fclose(fout);
    return 0;
}


int GenerateJumptable(const char* fnameIn, const char* fnameOut) {
    FILE* fin = fopen(fnameIn, "r");
    if (!fin) { printf("failed opening %s\n\r", fnameIn); return -1; }

    // parse input file
    int c = 0;
    while(1) {
        char* line = 0;
        size_t len = 0;
        ssize_t nread = getline(&line, &len, fin);
        if (nread == -1)
            break;

        for (int i=0; i<len; i++) { if (line[i] == ',') line[i] = ' '; }
        int args = sscanf(line, "%s %s %s %s %s %s %s %s",
                          ops[c+0].name, ops[c+1].name, ops[c+2].name, ops[c+3].name,
                          ops[c+4].name, ops[c+5].name, ops[c+6].name, ops[c+7].name);

        if ((args != 8) || (ops[c+0].name[0] == '*')) { continue; }
        c += 8; if (c > 8192) { break; }
    }
    fclose(fin);
    if (c != 8192) { printf("Parsing error\n"); return -1; }

    // expand table
    for (c=0; c<8192; c++) {
        int code = GetOpFromName(ops[c].name);
        if (ShouldExpandOp(code)) {
            sprintf(ops[c].name0, "Op%04x", code+0);
            sprintf(ops[c].name1, "Op%04x", code+1);
            sprintf(ops[c].name2, "Op%04x", code+2);
            sprintf(ops[c].name3, "Op%04x", code+3);
            sprintf(ops[c].name4, "Op%04x", code+4);
            sprintf(ops[c].name5, "Op%04x", code+5);
            sprintf(ops[c].name6, "Op%04x", code+6);
            sprintf(ops[c].name7, "Op%04x", code+7);
        } else {
            strcpy(ops[c].name0, ops[c].name);
            strcpy(ops[c].name1, ops[c].name);
            strcpy(ops[c].name2, ops[c].name);
            strcpy(ops[c].name3, ops[c].name);
            strcpy(ops[c].name4, ops[c].name);
            strcpy(ops[c].name5, ops[c].name);
            strcpy(ops[c].name6, ops[c].name);
            strcpy(ops[c].name7, ops[c].name);
        }
    }

    // write output file
    FILE* fout = fopen(fnameOut, "w");
    if (!fout) { printf("Failed opening %s\n\r", fnameOut); return -1; }

    fprintf(fout, "// This file was generated from: %s\n\n", fnameIn);
    fprintf(fout, "#include \"op68k.h\"\n");
    fprintf(fout, "#include \"proto.h\"\n");
    fprintf(fout, "\n");
    fprintf(fout, "#ifndef COLDFIRE\n");
    fprintf(fout, "  void (*const jmp_table[8192*8])() = {");
    fprintf(fout, "\n");
    for (c=4096; c<8192; c ++) {
        fprintf(fout, "    %s, %s, %s, %s, %s, %s, %s, %s,\n", ops[c].name0, ops[c].name1, ops[c].name2, ops[c].name3, ops[c].name4, ops[c].name5, ops[c].name6, ops[c].name7);
    }
    for (c=0; c<4096; c ++) {
        fprintf(fout, "    %s, %s, %s, %s, %s, %s, %s, %s", ops[c].name0, ops[c].name1, ops[c].name2, ops[c].name3, ops[c].name4, ops[c].name5, ops[c].name6, ops[c].name7);
        if (c < 4095) fprintf(fout,",\n");
        else          fprintf(fout,"};\n");
    }
    fprintf(fout, "#else\n");
    fprintf(fout, "  void (*const jmp_table[8192*8])() = {");
    fprintf(fout, "\n");
    for (c=0; c<8192; c ++) {
        fprintf(fout, "    %s, %s, %s, %s, %s, %s, %s, %s", ops[c].name0, ops[c].name1, ops[c].name2, ops[c].name3, ops[c].name4, ops[c].name5, ops[c].name6, ops[c].name7);
        if (c < 8191) fprintf(fout,",\n");
        else          fprintf(fout,"};\n");
    }
    fprintf(fout, "#endif\n");
    fprintf(fout, "\n");
    fclose(fout);
    return 0;    
}
