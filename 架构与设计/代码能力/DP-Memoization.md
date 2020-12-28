# 概述

动态规划程序有两种常见解法：

* 表格法(Tabulation)：自下而上 (Bottom up)
* 记忆法(Memoization)：自上而下 (Top down)

解决DP中大多数问题的一种较简单的方法是先编写递归代码，然后再编写自下而上的制表方法或自上而下的递归函数记忆。
自顶向下方法解决DP问题的步骤是：

* 编写递归代码
* 记住返回值并使用它减少递归调用(剪枝)

# 1-D Memoization (一维记忆法)

1-D Memo适用于只有一个变量的递归场景，比如Fibonacci数列。

```java
import java.io.*; 
  
class TopDown {
	// Fibonacci Series
	// using Recursion
	static int fib(int n) {
  
    	// Base case
    	if (n <= 1)
        	return n;
  
    	// recursive calls 
    	return fib(n - 1) +
           	fib(n - 2);
	}
  
	// Driver Code 
	public static void main (String[] args) {
    	int n = 6;
    	System.out.println(fib(n));
	}
}
```

上面的递归实现执行了大量重复的工作（请参见下面的递归树）。因此，如果要找到第N个Fibonacci数，这将花费大量的时间。


```
                            fib(5)   
                     /                 \        
               fib(4)                  fib(3)   
             /      \                /       \
         fib(3)      fib(2)         fib(2)    fib(1)
        /   \          /    \       /      \ 
  fib(2)   fib(1)  fib(1) fib(0) fib(1) fib(0)
  /    \ 
fib(1) fib(0) 

```

下面是1-D Memo的解法。我们将fib(n)的结果保存在memo[n]的位置。如fib(3)的结果存放在memo[3]。后面碰到fib(3)就可以直接返回mem[3]了。


```java
import java.io.*; 
  
class TopDown {

	// Fibonacci Series
	// using Recursion
	static int fib(int n, int[] memo) {
  
    	// Base case
    	if (n <= 1)
        	return n;

        if (memo[n] != 0)
        	return memo[n];
  
    	// recursive calls 
    	memo[n] = fib(n - 1) +
           	fib(n - 2);
        return memo[n];
	}
  
	// Driver Code 
	public static void main (String[] args) {
    	int n = 6;

    	int[] memo = new int[1000];

    	System.out.println(fib(n, memo));
	}
}
```

# 2-D Memoization (二维记忆法)

二维记忆法适用于两个变量的场景。常见的例子是[LCS](https://www.geeksforgeeks.org/longest-common-subsequence/)。

递归算法如下：

```c++
// A Naive recursive implementation of LCS problem 
#include <bits/stdc++.h> 
  
int max(int a, int b); 
  
// Returns length of LCS for X[0..m-1], Y[0..n-1] 
int lcs(char* X, char* Y, int m, int n) 
{ 
    if (m == 0 || n == 0) 
        return 0; 
    if (X[m - 1] == Y[n - 1]) 
        return 1 + lcs(X, Y, m - 1, n - 1); 
    else
        return max(lcs(X, Y, m, n - 1), 
                  lcs(X, Y, m - 1, n)); 
} 
  
// Utility function to get max of 2 integers 
int max(int a, int b) 
{ 
    return (a > b) ? a : b; 
} 
  
// Driver Code 
int main() 
{ 
    char X[] = "AGGTAB"; 
    char Y[] = "GXTXAYB"; 
  
    int m = strlen(X); 
    int n = strlen(Y); 
  
    printf("Length of LCS is %dn", lcs(X, Y, m, n)); 
  
    return 0; 
}
```

其递归树如下

```
                  lcs("AXYT", "AYZX")
                       /                 \
         lcs("AXY", "AYZX")            lcs("AXYT", "AYZ")
         /           \                   /               \
lcs("AX", "AYZX") lcs("AXY", "AYZ")   lcs("AXY", "AYZ") lcs("AXYT", "AY")
```

2-D Memo解法

```c++
// C++ program to memoize 
// recursive implementation of LCS problem 
#include <bits/stdc++.h> 
int arr[1000][1000]; // external memoization definition
int max(int a, int b);  // external max function definition
  
// Returns length of LCS for X[0..m-1], Y[0..n-1] */ 
// memoization applied in recursive solution 
int lcs(char* X, char* Y, int m, int n) 
{ 
    // base case 
    if (m == 0 || n == 0) 
        return 0; 
  
    // if the same state has already been 
    // computed 
    if (arr[m - 1][n - 1] != -1) 
        return arr[m - 1][n - 1]; 
  
    // if equal, then we store the value of the 
    // function call 
    if (X[m - 1] == Y[n - 1]) { 
  
        // store it in arr to avoid further repetitive  
        // work in future function calls 
        arr[m - 1][n - 1] = 1 + lcs(X, Y, m - 1, n - 1); 
        return arr[m - 1][n - 1]; 
    } 
    else { 
        // store it in arr to avoid further repetitive  
        // work in future function calls 
        arr[m - 1][n - 1] = max(lcs(X, Y, m, n - 1), 
                                lcs(X, Y, m - 1, n)); 
        return arr[m - 1][n - 1]; 
    } 
} 
  
// Utility function to get max of 2 integers 
int max(int a, int b) 
{ 
    return (a > b) ? a : b; 
} 
  
// Driver Code 
int main() 
{ 
    memset(arr, -1, sizeof(arr)); 
    char X[] = "AGGTAB"; 
    char Y[] = "GXTXAYB"; 
  
    int m = strlen(X); 
    int n = strlen(Y); 
  
    printf("Length of LCS is %d", lcs(X, Y, m, n)); 
  
    return 0; 
}
```

# 结尾

DP是针对最优解问题的通用解法。DP类问题的根本矛盾在于维数。如果问题维数巨大，则其状态是巨大无比的，此时，求解最优解的成本是非常大的。因此，DP只适用于寻找低维度问题的最优解。