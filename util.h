#ifndef __UTIL_DEFINED__
#define __UTIL_DEFINED__

char* strAfter(char* haystack, char* needle);
int indexOf(char* haystack, char needle);

int can_fit(double S, double W, double H, int n);
int find_max_square_side(double W, double H, int n);

double lerp(double a, double b, double t);
double invlerp(double a, double b, double v);
double remap(double in_min, double in_max, double out_min, double out_max, double v);

#endif // __UTIL_DEFINED__
