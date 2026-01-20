#include <stdio.h>
#include <string.h>
#include <math.h>
#include "util.h"

char* strAfter(char* haystack, char* needle)
{
	char* found = strstr(haystack, needle);
	if (found != NULL)
		found += strlen(needle);

	return found;
}

int indexOf(char* haystack, char needle)
{
	int index = -1;
	char* found = strchr(haystack, needle);
	if (found != NULL)
		index = found - haystack;

	return index;
}

// Function to check if n squares of a given side length S fit in the rectangle
int can_fit(double S, double W, double H, int n)
{
	if (S <= 0) return 1; // Squares of size 0 always fit
	int squares_per_row = (int)floor(W / S);
	int squares_per_col = (int)floor(H / S);
	return (squares_per_row * squares_per_col) >= n;
}

// Function to find the maximum square side length to fit n squares in a WxH rectangle
int find_max_square_side(double W, double H, int n)
{
	// Initial high/low estimates (e.g., maximum of width/height or area-based bound)
	double high = fmin(W, H);
	double low = high / n;

	// Binary search for the optimal side length
	for (int i = 0; i < 100 && (high - low) > 1.0; i++)
	{ // 100 iterations for good precision
		double mid = (low + high) / 2.0;
		if (can_fit(mid, W, H, n))
		{
			low = mid;
		}
		else
		{
			high = mid;
		}
	}
	return (int)floor(low);
}

// Example usage:
//int main()
//{
//	double width = 200.0;
//	double height = 100.0;
//	int n_squares = 50;

//	double max_side = find_max_square_side(width, height, n_squares);
//	int cols = (int)floor(width / max_side);
//	int rows = (int)floor(height / max_side);

//	printf("Max square side length: %f\n", max_side);
//	printf("Arrangement: %d columns, %d rows\n", cols, rows);

//	return 0;
//}

// Linear Interpolation (Lerp)
// Returns a value between 'a' and 'b' based on the interpolation factor 't' (0.0 to 1.0)
double lerp(double a, double b, double t)
{
	return a + (b - a) * t; //
	// An alternative formula for potentially better floating point precision near edges is:
	// return a * (1.0f - t) + b * t; //
}

// Inverse Linear Interpolation (InvLerp)
// Returns the factor 't' (0.0 to 1.0) representing the position of 'v' within the range ['a', 'b']
double invlerp(double a, double b, double v)
{
// If 'a' and 'b' are the same, prevent division by zero
	if (a == b)
	{
		return 0.0f;
	}
	// Clamp the result between 0.0f and 1.0f if the input value 'v' should stay within the range
	// return clamp((v - a) / (b - a), 0.0f, 1.0f); 
	// Or without clamping, to allow extrapolation:
	return (v - a) / (b - a); //
}

// Remap (Range Mapping)
// Maps a value 'v' from an input range ['in_min', 'in_max'] to an output range ['out_min', 'out_max']
double remap(double in_min, double in_max, double out_min, double out_max, double v)
{
// The remap function uses both invlerp and lerp internally
	double t = invlerp(in_min, in_max, v);
	return lerp(out_min, out_max, t); //
}
