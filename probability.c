#include "probability.h"

// Function to calculate combinations (nCr) using long double to handle large numbers
long double nCm(int n, int m)
{
	if (m < 0 || m > n) return 0;
	if (m == 0 || m == n) return 1;
	if (m > n / 2) m = n - m; // Optimization

	long double res = 1;
	for (int i = 1; i <= m; ++i)
		res = res * (n - i + 1) / i;

	return res;
}

// Function to calculate the probability (odds) for a specific prize tier
// N1: total main balls, D1: drawn main balls, M1: matched main balls
// N2: total bonus balls, D2: drawn bonus balls, M2: matched bonus balls
long double calculate_odds(int N1, int D1, int M1, int N2, int D2, int M2)
{
	// Ways to choose M1 winning main balls from N1 winning main balls
	long double ways_m1 = nCm(D1, M1);
	// Ways to choose (K_main - M1) losing main balls from (N_main - K_main) losing main balls
	long double ways_l1 = nCm(N1 - D1, D1 - M1);

	// Ways to choose M2 winning bonus balls from K_bonus winning bonus balls (if applicable)
	long double ways_m2 = 1;
	// Ways to choose (K_bonus - M2) losing bonus balls from (N_bonus - K_bonus) losing bonus balls
	long double ways_l2 = 1;

	// Only calculate bonus ball combinations if N_bonus is greater than 0
	if (N2 > 0)
	{
		ways_m2 = nCm(D2, M2);
		ways_l2 = nCm(N2 - D2, D2 - M2);
	}

	// Total successful combinations
	long double successful_ways = ways_m1 * ways_l1 * ways_m2 * ways_l2;

	// Total possible combinations (T)
	// T = combinations(N_main, K_main) * combinations(N_bonus, K_bonus)
	long double total_combinations = nCm(N1, D1);
	if (N2 > 0)
	{
		total_combinations *= nCm(N2, D2);
	}

	// Odds = Total combinations / Successful combinations (expressed as 1 in X)
	long double odds = total_combinations / successful_ways;
	return odds;
}

/*
int main()
{
	printf("Size of long double: %zu bytes\n", sizeof(long double));

	// Example: Powerball (5 main balls from 69, 1 bonus ball from 26)
	// Odds of winning the jackpot: 5 main balls matched AND 1 bonus ball matched
	long double jackpot_odds = calculate_odds(69, 5, 5, 26, 1, 1);
	printf("Odds of winning jackpot (5 main + 1 bonus) are 1 in %.0Lf\n", jackpot_odds);

	// Odds of winning second prize: 5 main balls matched AND 0 bonus balls matched
	long double second_odds = calculate_odds(69, 5, 5, 26, 1, 0);
	printf("Odds of winning second prize (5 main + 0 bonus) are 1 in %.0Lf\n", second_odds);

	// Example: A simpler lottery, Mass Cash (35 main balls draw 5, no bonus ball)
	// Odds of winning jackpot (5 main matched, no bonus consideration)
	long double simple_odds = calculate_odds(35, 5, 5, 0, 0, 0);
	printf("Odds of winning simple lottery (5 main) are 1 in %.2Lf\n", simple_odds);
	// Odds of winning match 4)
	simple_odds = calculate_odds(35, 5, 4, 0, 0, 0);
	printf("Odds of winning simple lottery (4 main) are 1 in %.2Lf\n", simple_odds);
	// Odds of winning match 3)
	simple_odds = calculate_odds(35, 5, 3, 0, 0, 0);
	printf("Odds of winning simple lottery (3 main) are 1 in %.2Lf\n", simple_odds);

	return 0;
}
*/
