#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <climits>
#include <stack>
#include <fstream>
using namespace std;

#define vi vector<int>
#define pii pair<int, int>
#define vii vector<pii>
#define ff first
#define ss second
#define rep(i, a, b) for (long long i = a; i < b; i++)
#define f(a,n) for (long long i = a; i < n; i++)

int main()
{
    // Redirect input and output
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
 //your code starts here (remove the rest as you like)
    long long t;
    cin >> t;
    while (t--)
    {
        long long n;
        cin >> n;
        vector<long long> a(n);
        f(0,n){cin >> a[i];}
        f(0,n){cout << a[i] << " ";}
        cout << endl;
    }

    return 0;
}
