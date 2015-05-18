#ifndef __UTIL_H__
#define __UTIL_H__

void check_mat_mul( float* a, float* b, float* c, int n, int ts);
void print_mat( float* mat, int r, int c);
void print_help(const char* prog_name);
void parse_opt(int argc, char** argv);
void init_matrix(float** a, float** b, float** c, int n);
void print_result(int n);
void gen_tile(float **mat, int r, int c, int si, int sj, int ei, int ej);

#endif
