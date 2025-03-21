#pragma once

#define F(x,y,z) ((x & y) | (~x & z))

#define G(x,y,z) ((x & z) | (y & ~z))

#define H(x,y,z) (x^y^z)

#define I(x,y,z) (y ^ (x | ~z))

#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

#define FF(a,b,c,d,x,s,ac) \
							            { \
          a += F(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
							            }
#define GG(a,b,c,d,x,s,ac) \
							            { \
          a += G(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
							            }
#define HH(a,b,c,d,x,s,ac) \
							            { \
          a += H(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
							            }
#define II(a,b,c,d,x,s,ac) \
							            { \
          a += I(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
							            }   
