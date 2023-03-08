#pragma once

#define max(a,b) (((a) < (b)) ? (b) : (a))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define clamp(a,b,c) max(min(a,c),b)