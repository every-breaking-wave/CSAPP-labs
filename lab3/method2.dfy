method method2(a: array<int>, v: int) returns (b: int)
// Add a precondition here.
 requires v < a.Length && v > 0
{
   return a[v] / v;
}
