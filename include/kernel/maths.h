#pragma once

#define max(a,b) (((a) < (b)) ? (b) : (a))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define clamp(a,b,c) max(min(a,c),b)
#define sign(a) ((a < 0) ? -1 : ((a > 0) ? 1 : 0))
#define abs(a) (a * sign(a))