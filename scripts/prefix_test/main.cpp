#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <cassert>
#include <bitset>
#include <vector>
#include <queue>
#include <map>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <ctype.h>
#include <unistd.h>
#include <assert.h>
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
const bool RECORDBITS = false;
std::vector<int> pbits;
std::vector<int> rbits;

// const std::string file_path = "./szdat";
std::string file_path = "./data_pre/CESM-ATM/szdats/CLOUD_1_26_1800_3600.f32.sz4.szdat";
// const int n = 100000000 / 4;
int n = 673920000 / 4;
// const int n = 2869440;
// const int n = 2869440;
// const int n = 10;
const int DEBUG = 1;

// 8, 16, 32 bit unsigned types (adjust as appropriate)
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;

// Error handler: print message if any, and exit
void quit(const char* message=0) {
  if (message) printf("%s\n", message);
  exit(1);
}

// Create an array p of n elements of type T
template <class T> void alloc(T*&p, int n) {
  p=(T*)calloc(n, sizeof(T));
  if (!p) quit("out of memory");
}

///////////////////////////// Squash //////////////////////////////

// return p = 1/(1 + exp(-d)), d scaled by 8 bits, p scaled by 12 bits
int squash(int d) {
  static const int t[33]={
    1,2,3,6,10,16,27,45,73,120,194,310,488,747,1101,
    1546,2047,2549,2994,3348,3607,3785,3901,3975,4022,
    4050,4068,4079,4085,4089,4092,4093,4094};
  if (d>2047) return 4095;
  if (d<-2047) return 0;
  int w=d&127;
  d=(d>>7)+16;
  return (t[d]*(128-w)+t[(d+1)]*w+64) >> 7;
}

//////////////////////////// Stretch ///////////////////////////////

// Inverse of squash. stretch(d) returns ln(p/(1-p)), d scaled by 8 bits,
// p by 12 bits.  d has range -2047 to 2047 representing -8 to 8.
// p has range 0 to 4095 representing 0 to 1.

class Stretch {
  short t[4096];
public:
  Stretch();
  int operator()(int p) const {
    assert(p>=0 && p<4096);
    return t[p];
  }
} stretch;

Stretch::Stretch() {
  int pi=0;
  for (int x=-2047; x<=2047; ++x) {  // invert squash()
    int i=squash(x);
    for (int j=pi; j<=i; ++j)
      t[j]=x;
    pi=i+1;
  }
  t[4095]=2047;
}

///////////////////////// state table ////////////////////////

// State table:
//   nex(state, 0) = next state if bit y is 0, 0 <= state < 256
//   nex(state, 1) = next state if bit y is 1
//
// States represent a bit history within some context.
// State 0 is the starting state (no bits seen).
// States 1-30 represent all possible sequences of 1-4 bits.
// States 31-252 represent a pair of counts, (n0,n1), the number
//   of 0 and 1 bits respectively.  If n0+n1 < 16 then there are
//   two states for each pair, depending on if a 0 or 1 was the last
//   bit seen.
// If n0 and n1 are too large, then there is no state to represent this
// pair, so another state with about the same ratio of n0/n1 is substituted.
// Also, when a bit is observed and the count of the opposite bit is large,
// then part of this count is discarded to favor newer data over old.

static const U8 State_table[256][2]={
{  1,  2},{  3,  5},{  4,  6},{  7, 10},{  8, 12},{  9, 13},{ 11, 14}, // 0
{ 15, 19},{ 16, 23},{ 17, 24},{ 18, 25},{ 20, 27},{ 21, 28},{ 22, 29}, // 7
{ 26, 30},{ 31, 33},{ 32, 35},{ 32, 35},{ 32, 35},{ 32, 35},{ 34, 37}, // 14
{ 34, 37},{ 34, 37},{ 34, 37},{ 34, 37},{ 34, 37},{ 36, 39},{ 36, 39}, // 21
{ 36, 39},{ 36, 39},{ 38, 40},{ 41, 43},{ 42, 45},{ 42, 45},{ 44, 47}, // 28
{ 44, 47},{ 46, 49},{ 46, 49},{ 48, 51},{ 48, 51},{ 50, 52},{ 53, 43}, // 35
{ 54, 57},{ 54, 57},{ 56, 59},{ 56, 59},{ 58, 61},{ 58, 61},{ 60, 63}, // 42
{ 60, 63},{ 62, 65},{ 62, 65},{ 50, 66},{ 67, 55},{ 68, 57},{ 68, 57}, // 49
{ 70, 73},{ 70, 73},{ 72, 75},{ 72, 75},{ 74, 77},{ 74, 77},{ 76, 79}, // 56
{ 76, 79},{ 62, 81},{ 62, 81},{ 64, 82},{ 83, 69},{ 84, 71},{ 84, 71}, // 63
{ 86, 73},{ 86, 73},{ 44, 59},{ 44, 59},{ 58, 61},{ 58, 61},{ 60, 49}, // 70
{ 60, 49},{ 76, 89},{ 76, 89},{ 78, 91},{ 78, 91},{ 80, 92},{ 93, 69}, // 77
{ 94, 87},{ 94, 87},{ 96, 45},{ 96, 45},{ 48, 99},{ 48, 99},{ 88,101}, // 84
{ 88,101},{ 80,102},{103, 69},{104, 87},{104, 87},{106, 57},{106, 57}, // 91
{ 62,109},{ 62,109},{ 88,111},{ 88,111},{ 80,112},{113, 85},{114, 87}, // 98
{114, 87},{116, 57},{116, 57},{ 62,119},{ 62,119},{ 88,121},{ 88,121}, // 105
{ 90,122},{123, 85},{124, 97},{124, 97},{126, 57},{126, 57},{ 62,129}, // 112
{ 62,129},{ 98,131},{ 98,131},{ 90,132},{133, 85},{134, 97},{134, 97}, // 119
{136, 57},{136, 57},{ 62,139},{ 62,139},{ 98,141},{ 98,141},{ 90,142}, // 126
{143, 95},{144, 97},{144, 97},{ 68, 57},{ 68, 57},{ 62, 81},{ 62, 81}, // 133
{ 98,147},{ 98,147},{100,148},{149, 95},{150,107},{150,107},{108,151}, // 140
{108,151},{100,152},{153, 95},{154,107},{108,155},{100,156},{157, 95}, // 147
{158,107},{108,159},{100,160},{161,105},{162,107},{108,163},{110,164}, // 154
{165,105},{166,117},{118,167},{110,168},{169,105},{170,117},{118,171}, // 161
{110,172},{173,105},{174,117},{118,175},{110,176},{177,105},{178,117}, // 168
{118,179},{110,180},{181,115},{182,117},{118,183},{120,184},{185,115}, // 175
{186,127},{128,187},{120,188},{189,115},{190,127},{128,191},{120,192}, // 182
{193,115},{194,127},{128,195},{120,196},{197,115},{198,127},{128,199}, // 189
{120,200},{201,115},{202,127},{128,203},{120,204},{205,115},{206,127}, // 196
{128,207},{120,208},{209,125},{210,127},{128,211},{130,212},{213,125}, // 203
{214,137},{138,215},{130,216},{217,125},{218,137},{138,219},{130,220}, // 210
{221,125},{222,137},{138,223},{130,224},{225,125},{226,137},{138,227}, // 217
{130,228},{229,125},{230,137},{138,231},{130,232},{233,125},{234,137}, // 224
{138,235},{130,236},{237,125},{238,137},{138,239},{130,240},{241,125}, // 231
{242,137},{138,243},{130,244},{245,135},{246,137},{138,247},{140,248}, // 238
{249,135},{250, 69},{ 80,251},{140,252},{249,135},{250, 69},{ 80,251}, // 245
{140,252},{  0,  0},{  0,  0},{  0,  0}};  // 252
U32 stnum[256][2];
#define nex(state, sel) State_table[state][sel];
/*
int nex(int state, int sel) {
	// printf("NEX: (%d, %d) --> %d\n", state, sel, State_table[state][sel]);
	stnum[state][sel]++;
	return State_table[state][sel];
}
*/
// #define nex(state,sel) State_table[state][sel]

//////////////////////////// StateMap, APM //////////////////////////

// A StateMap maps a context to a probability.  Methods:
//
// Statemap sm(n) creates a StateMap with n contexts using 4*n bytes memory.
// sm.p(y, cx, limit) converts state cx (0..n-1) to a probability (0..4095).
//     that the next y=1, updating the previous prediction with y (0..1).
//     limit (1..1023, default 1023) is the maximum count for computing a
//     prediction.  Larger values are better for stationary sources.

class StateMap {
protected:
  const int N;  // Number of contexts
  int cxt;      // Context of last prediction
  U32 *t;       // cxt -> prediction in high 22 bits, count in low 10 bits
  static int dt[1024];  // i -> 16K/(i+3)
  void update(int y, int limit) {
    assert(cxt>=0 && cxt<N);
    int n=t[cxt]&1023, p=t[cxt]>>10;  // count, prediction
    // update count
    if (n<limit) ++t[cxt];
    // update prediction
    t[cxt]+=(((y<<22)-p)>>3)*dt[n]&0xfffffc00;
  }
public:
  StateMap(int n=256);

  // update bit y (0..1), predict next bit in context cx
  int p(int y, int cx, int limit=1023) {
    assert(y>>1==0);
    assert(cx>=0 && cx<N);
    assert(limit>0 && limit<1024);
    update(y, limit);
    return t[cxt=cx]>>20;
  }
};

int StateMap::dt[1024]={0};

StateMap::StateMap(int n): N(n), cxt(0) {
  alloc(t, N);
  for (int i=0; i<N; ++i)
    t[i]=1<<31;
  if (dt[0]==0)
    for (int i=0; i<1024; ++i)
      dt[i]=16384/(i+i+3);
}

// An APM maps a probability and a context to a new probability.  Methods:
//
// APM a(n) creates with n contexts using 96*n bytes memory.
// a.pp(y, pr, cx, limit) updates and returns a new probability (0..4095)
//     like with StateMap.  pr (0..4095) is considered part of the context.
//     The output is computed by interpolating pr into 24 ranges nonlinearly
//     with smaller ranges near the ends.  The initial output is pr.
//     y=(0..1) is the last bit.  cx=(0..n-1) is the other context.
//     limit=(0..1023) defaults to 255.

class APM: public StateMap {
public:
  APM(int n);
  int pp(int y, int pr, int cx, int limit=255) {
    assert(y>>1==0);
    assert(pr>=0 && pr<4096);
    assert(cx>=0 && cx<N/24);
    assert(limit>0 && limit<1024);
    update(y, limit);
    pr=(stretch(pr)+2048)*23;
    int wt=pr&0xfff;  // interpolation weight of next element
    cx=cx*24+(pr>>12);
    assert(cx>=0 && cx<N-1);
    pr=(t[cx]>>13)*(0x1000-wt)+(t[cx+1]>>13)*wt>>19;
    cxt=cx+(wt>>11);
    return pr;
  }
};

APM::APM(int n): StateMap(n*24) {
  for (int i=0; i<N; ++i) {
    int p=((i%24*2+1)*4096)/48-2048;
    t[i]=(U32(squash(p))<<20)+6;
  }
}

//////////////////////////// Mixer /////////////////////////////

// Mixer m(N, M) combines models using M neural networks with
//     N inputs each using 4*M*N bytes of memory.  It is used as follows:
// m.update(y) trains the network where the expected output is the
//     last bit, y.
// m.add(stretch(p)) inputs prediction from one of N models.  The
//     prediction should be positive to predict a 1 bit, negative for 0,
//     nominally -2K to 2K.
// m.set(cxt) selects cxt (0..M-1) as one of M neural networks to use.
// m.p() returns the output prediction that the next bit is 1 as a
//     12 bit number (0 to 4095).  The normal sequence per prediction is:
//
// - m.add(x) called N times with input x=(-2047..2047)
// - m.set(cxt) called once with cxt=(0..M-1)
// - m.p() called once to predict the next bit, returns 0..4095
// - m.update(y) called once for actual bit y=(0..1).

inline void train(int *t, int *w, int n, int err) {
  for (int i=0; i<n; ++i)
    w[i]+=t[i]*err+0x8000>>16;
}

inline int dot_product(int *t, int *w, int n) {
  int sum=0;
  for (int i=0; i<n; ++i)
    sum+=t[i]*w[i];
  return sum>>8;
}

class Mixer {
  const int N, M;  // max inputs, max contexts
  int* tx;         // N inputs
  int* wx;         // N*M weights
  int cxt;         // context
  int nx;          // Number of inputs in tx, 0 to N
  int pr;          // last result (scaled 12 bits)
public:
  Mixer(int n, int m);

  // Adjust weights to minimize coding cost of last prediction
  void update(int y) {
    int err=((y<<12)-pr)*7;
    assert(err>=-32768 && err<32768);
    train(&tx[0], &wx[cxt*N], N, err);
    nx=0;
  }

  // Input x (call up to N times)
  void add(int x) {
    assert(nx<N);
    tx[nx++]=x;
  }

  // Set a context
  void set(int cx) {
    assert(cx>=0 && cx<M);
    cxt=cx;
  }

  // predict next bit
  int p() {
	int sk = dot_product(&tx[0], &wx[cxt*N], N)>>8;
    // printf("sk:%d\n",sk);
	return pr=squash(sk);
  }
};

Mixer::Mixer(int n, int m):
    N(n), M(m), tx(0), wx(0), cxt(0), nx(0), pr(2048) {
  assert(n>0 && N>0 && M>0);
  alloc(tx, N);
  alloc(wx, N*M);
}

//////////////////////////// HashTable /////////////////////////

// A HashTable maps a 32-bit index to an array of B bytes.
// The first byte is a checksum using the upper 8 bits of the
// index.  The second byte is a priority (0 = empty) for hash
// replacement.  The index need not be a hash.

// HashTable<B> h(n) - create using n bytes  n and B must be
//     powers of 2 with n >= B*4, and B >= 2.
// h[i] returns array [1..B-1] of bytes indexed by i, creating and
//     replacing another element if needed.  Element 0 is the
//     checksum and should not be modified.

template <int B>
class HashTable {
  U8* t;  // table: 1 element = B bytes: checksum priority data data
  const int N;  // size in bytes
public:
  HashTable(int n);
  U8* operator[](U32 i);
};

template <int B>
HashTable<B>::HashTable(int n): t(0), N(n) {
  assert(B>=2 && (B&B-1)==0);
  assert(N>=B*4 && (N&N-1)==0);
  alloc(t, N+B*4+64);
  t+=64-int(((long)t)&63);  // align on cache line boundary
}

template <int B>
inline U8* HashTable<B>::operator[](U32 i) {
  i*=123456791;
  i=i<<16|i>>16;
  i*=234567891;
  int chk=i>>24;
  i=i*B&N-B;
  if (t[i]==chk) return t+i;
  if (t[i^B]==chk) return t+(i^B);
  if (t[i^B*2]==chk) return t+(i^B*2);
  if (t[i+1]>t[i+1^B] || t[i+1]>t[i+1^B*2]) i^=B;
  if (t[i+1]>t[i+1^B^B*2]) i^=B^B*2;
  memset(t+i, 0, B);
  t[i]=chk;
  return t+i;
}

//////////////////////////// MatchModel ////////////////////////

// MatchModel(n) predicts next bit using most recent context match.
//     using n bytes of memory.  n must be a power of 2 at least 8.
// MatchModel::p(y, m) updates the model with bit y (0..1) and writes
//     a prediction of the next bit to Mixer m.  It returns the length of
//     context matched (0..62).

class MatchModel {
  const int N;  // last buffer index, n/2-1
  const int HN; // last hash table index, n/8-1
  enum {MAXLEN=62};   // maximum match length, at most 62
  U8* buf;    // input buffer
  int validlength;
  int* ht;    // context hash -> next byte in buf
  int pos;    // number of bytes in buf
  int match;  // pointer to current byte in matched context in buf
  int len;    // length of match
  U32 h1, h2; // context hashes
  int c0;     // last 0-7 bits of y
  int bcount; // number of bits in c0 (0..7)
  StateMap sm;  // len, bit, last byte -> prediction
public:
  MatchModel(int n);  // n must be a power of 2 at least 8.
  int p(int y, Mixer& m);  // update bit y (0..1), predict next bit to m
};

MatchModel::MatchModel(int n): N(n/2-1), HN(n/8-1), buf(0), ht(0), pos(0),
    match(0), len(0), h1(0), h2(0), c0(1), bcount(0), sm(56<<8) {
  assert(n>=8 && (n&n-1)==0);
  alloc(buf, N+1);
  alloc(ht, HN+1);
  validlength = 0;
}

int MatchModel::p(int y, Mixer& m) {

  // update context
  c0+=c0+y;
  ++bcount;
  if (bcount==8) {
    validlength++;
    // printf("marchmodel undating len..., original len: %d\n", len);
    bcount=0;
    h1=h1*(3<<3)+c0&HN;
    h2=h2*(5<<5)+c0&HN;
    buf[pos++]=c0;
    c0=1;
    pos&=N;
    // printf("E2: match:%d, h1:%d, h2:%d, pos:%d, len:%d\n", match, h1, h2, pos, len);
    // find or extend match
    if (len>0) {
      ++match;
      match&=N;
      if (len<MAXLEN) ++len;
    }
    else {
      match=ht[h1];
      // printf("E2: match:%d, pos: %d\n", match, pos);
      if (match!=pos) {
        int i;
        while (len<MAXLEN && (i=match-len-1&N)!=pos
               && buf[i]==buf[pos-len-1&N])
        // while (len<MAXLEN && len < validlength-1 && (i=match-len-1&N)!=pos
        //        && buf[i]==buf[pos-len-1&N])
          ++len;
      }
    }
    // printf("E3, len: %d\n", len);
    if (len<2) {
      len=0;
      match=ht[h2];
      if (match!=pos) {
        int i;
        while (len<MAXLEN && (i=match-len-1&N)!=pos
               && buf[i]==buf[pos-len-1&N])
        // while (len<MAXLEN && len < validlength-1 && (i=match-len-1&N)!=pos
        //        && buf[i]==buf[pos-len-1&N])
          ++len;
      }
    }
  }
  // printf("matchmodel find len: %d\n", len);
  // predict
  int cxt=c0;
  len = 0;
  if (len>0 && (buf[match]+256>>8-bcount)==c0) {
    int b=buf[match]>>7-bcount&1;  // next bit
    if (len<16) cxt=len*2+b;
    else cxt=(len>>2)*2+b+24;
    cxt=cxt*256+buf[pos-1&N];
  }
  int smp = sm.p(y, cxt);
  // printf("E4: cxt: %d, smp: %d, stretch_smp: %d\n", cxt, smp, stretch(smp));
  m.add(stretch(smp));

  // update index
  if (bcount==0) {
    ht[h1]=pos;
    ht[h2]=pos;
  }
  return len;
}

//////////////////////////// Predictor /////////////////////////

// A Predictor estimates the probability that the next bit of
// uncompressed data is 1.  Methods:
// Predictor(n) creates with 3*n bytes of memory.
// p() returns P(1) as a 12 bit number (0-4095).
// update(y) trains the predictor with the actual bit (0 or 1).

int MEM=1<<23;  // Global memory usage = 3*MEM bytes (1<<20 .. 1<<29)

class Predictor {
  int pr;  // next prediction
public:
  Predictor();
  int predict() const {
    assert(pr>=0 && pr<4096);
    return pr;
  }
  void update(int y);
};

Predictor::Predictor(): pr(2048) {}

int predictor_enable[7]={0,1,0,1,0,0,0};
int bitwidth_predictor_enable = 1;
int cnt = 0;
int bitwidth = 0;

void Predictor::update(int y) {
  // return;
  static U8 t0[0x10000];  // order 1 cxt -> state // 2^16
  static HashTable<16> t(MEM*2);  // cxt -> state
  static int c0=1;  // last 0-7 bits with leading 1
  static int c4=0;  // last 4 bytes
  static U8 *cp[5]={t0, t0, t0, t0, t0};  // pointer to bit history
  static int bcount=0;  // bit count
  static StateMap sm[5];
  static APM a1(0x100), a2(0x4000);
  static U32 h[5];

  static Mixer m(predictor_enable[0]+predictor_enable[1]+predictor_enable[2]+
  predictor_enable[3]+predictor_enable[4]+predictor_enable[6] + bitwidth_predictor_enable, 80);
  static MatchModel mm(MEM);  // predicts next bit by matching context
  assert(MEM>0);

  if(bitwidth && bitwidth_predictor_enable) {
    if(cnt == bitwidth-1) {
      cnt = 0;
      // cp[0] = cp[1] = cp[2] = cp[3] = cp[4] = t0;
      bcount=0;
      c0 = 1;
      c4 = 0;
    } else {
      cnt++;
    }
  }

  // update model
  assert(y==0 || y==1);
  if(predictor_enable[1]) *cp[0]=nex(*cp[0], y);
  if(predictor_enable[2]) *cp[1]=nex(*cp[1], y);
  if(predictor_enable[3]) *cp[2]=nex(*cp[2], y);
  if(predictor_enable[4]) *cp[3]=nex(*cp[3], y);
  if(predictor_enable[6]) *cp[4]=nex(*cp[4], y);
  m.update(y);

  // update context
  ++bcount;
  c0+=c0+y;
  if (c0>=256) {
    c0-=256;
    c4=c4<<8|c0;
    if(predictor_enable[1]) h[0]=c0<<8;  // order 1
    if(predictor_enable[2]) h[1]=(c4&0xffff)<<5|0x57000000;  // order 2 (c4&0xffff|0x570) << 5 // 010101110000
    if(predictor_enable[3]) h[2]=(c4<<8)*3;  // order 3
    if(predictor_enable[4]) h[3]=c4*5;  // order 4
    if(predictor_enable[6]) h[4]=h[4]*(11<<5)+c0*13&0x3fffffff;  // order 6
    if(predictor_enable[2]) cp[1]=t[h[1]]+1;
    if(predictor_enable[3]) cp[2]=t[h[2]]+1;
    if(predictor_enable[4]) cp[3]=t[h[3]]+1;
    if(predictor_enable[6]) cp[4]=t[h[4]]+1;
    c0=1;
    bcount=0;
  }
  if (bcount==4) {
    if(predictor_enable[2]) cp[1]=t[h[1]+c0]+1;
    if(predictor_enable[3]) cp[2]=t[h[2]+c0]+1;
    if(predictor_enable[4]) cp[3]=t[h[3]+c0]+1;
    if(predictor_enable[6]) cp[4]=t[h[4]+c0]+1;
  }
  else if (bcount>0) {
    int j=y+1<<(bcount&3)-1;
    if(predictor_enable[2]) cp[1]+=j;
    if(predictor_enable[3]) cp[2]+=j;
    if(predictor_enable[4]) cp[3]+=j;
    if(predictor_enable[6]) cp[4]+=j;
  }
  if(predictor_enable[1]) cp[0]=t0+h[0]+c0;
  // predict
  int len = 0;
  if(predictor_enable[0]) int len=mm.p(y, m);
  int order=0;
  if (len==0) {
    if(predictor_enable[6]) if (*cp[4]) ++order;
    if(predictor_enable[4]) if (*cp[3]) ++order;
    if(predictor_enable[3]) if (*cp[2]) ++order;
    if(predictor_enable[2]) if (*cp[1]) ++order;
  }
  else order=5+(len>=8)+(len>=12)+(len>=16)+(len>=32);
  if(predictor_enable[1]) m.add(stretch(sm[0].p(y, *cp[0])));
  if(predictor_enable[2]) m.add(stretch(sm[1].p(y, *cp[1])));
  if(predictor_enable[3]) m.add(stretch(sm[2].p(y, *cp[2])));
  if(predictor_enable[4]) m.add(stretch(sm[3].p(y, *cp[3])));
  if(predictor_enable[6]) m.add(stretch(sm[4].p(y, *cp[4])));
  m.set(order+10*(h[0]>>13));
  pr=m.p();
  pr=pr+3*a1.pp(y, pr, c0)>>2;
  pr=pr+3*a2.pp(y, pr, c0^h[0]>>2)>>2;
}

struct Encoder {
    U8* origin; int fb; 
    U32 x1, x2, x;    
    int bitwidth; 
    U32 correctcnt;
    Predictor predictor;  
    Encoder(int bitwidth = 32) {
        this->bitwidth = bitwidth;
        x1 = 0, x2 = 0xffffffff, x = 0;
        correctcnt = 0; fb = 0;
    }    

    int code(int y = 0) {
        int p = predictor.predict();
        if(RECORDBITS) {
            pbits.push_back(p);
            rbits.push_back(y == 1 ? 4096 : 0);
        }
        assert(p >= 0 && p < 4096);
        p += p < 2048;
        U32 xmid = x1 + ((x2 - x1) >> 12) * p + ((x2 - x1 & 0xfff) * p >> 12);
        assert(xmid >= x1 && xmid < x2);
        y ? (x2 = xmid) : (x1 = xmid + 1);
        predictor.update(y);
        while (((x1 ^ x2) & 0xff000000) == 0) {
            origin[fb++] = x2 >> 24;
            x1 <<= 8;
            x2 = (x2 << 8) + 255;
        }
        return y;
    }
    int decode(int y = 0) {
        int p = predictor.predict();
        assert(p >= 0 && p < 4096);
        p += p < 2048;
        U32 xmid = x1 + ((x2 - x1) >> 12) * p + ((x2 - x1 & 0xfff) * p >> 12);
        assert(xmid >= x1 && xmid < x2);
        y = x <= xmid;
        y ? (x2 = xmid) : (x1 = xmid + 1);
        predictor.update(y);
        while (((x1 ^ x2) & 0xff000000) == 0) {
            x1 <<= 8;
            x2 = (x2 << 8) + 255;
            int cur = origin[fb++];
            x = (x << 8) + (cur & 255);
        }
        return y;
    }

    void compressbyte(U8 c) {
        for (int i = 7; i >= 0; --i)
            code((c >> i) & 1);
    }
    U8 decompressbyte() {
        int c = 0;
        for (int i = 0; i < 8; ++i)
            c += c + decode();
        return c;
    }
};

U32 successcnt = 0;
void compress(int *origin, size_t length, U8** res, int* res_len) {
    bitwidth = 0;
    int maxx = origin[0];
    for(int i = 0; i < length; i++) {
        if(origin[i] > maxx) {
            maxx = origin[i];
        }
    }
    while(maxx) {
        bitwidth++;
        maxx >>= 1;
    }
    printf("bitwidth: %d\n", bitwidth);
    // actual compression process
    struct Encoder encoder(bitwidth);
    *res = (U8*)malloc((length * 2) * sizeof(U8));
    (*res)[encoder.fb++] = bitwidth;
    encoder.origin = *res;
    for(int i = 0; i < length; i++) {
        // dprintf("code %d\n", origin[i]);
        for(int j = bitwidth - 1; j >= 0; j--) {
            encoder.code((origin[i] >> j) & 1);
        }
    }
    *res_len = encoder.fb + 1;
    *res = (U8*)realloc(*res, *res_len);
    (*res)[encoder.fb] = encoder.x2 >> 24;
    successcnt  = encoder.correctcnt;
}

void decompress(U8 *origin, size_t length, int* res, int res_len) {
    bitwidth = origin[0];
    struct Encoder encoder(bitwidth);
    printf("bitwidth: %d\n", bitwidth);
    encoder.origin = origin;
    encoder.fb = 1;
    for (int i = 0; i < 4; ++i) {
        encoder.x = (encoder.x << 8) + (origin[encoder.fb++] & 255);
    }
    for(int i = 0; i < res_len; i++) {
        res[i] = 0;
        for(int j = 0; j < bitwidth; j++) {
            res[i] += res[i] + encoder.decode();
        }
    }
}

int Mode = 1; // 1 for compress, 0 for decompress

signed main(int argc, char*argv[]) {
	file_path = argv[1];
	bitwidth_predictor_enable = std::stoi(argv[3]);
	n = std::stoi(argv[2]);
	for(int i = 1; i <= 6; i++) {
		predictor_enable[i] = std::stoi(argv[i + 3]);
	}
    int* origin = (int*)malloc(n * sizeof(int));
    FILE *in = fopen(file_path.c_str(), "rb");
    for(int i = 1; i <= n; i++) {
        int s;
        fread(&s, sizeof(int), 1, in);
        origin[i - 1] = s;
    }
    fclose(in);
    if(Mode) {
      U8* res;
      int res_len;
      time_t start = time(0);
      compress(origin, n, &res, &res_len);
      double seconds_since_start = difftime(time(0), start);
      printf("original file_size: %d," 
              "Compressed length: %d,"
              "total bit num: %u,"
              "successful predict bit num: %u,"
              "bitwidth: %d,"
              "time cost: %f\n", 
              n * 4, res_len, (U32)n * bitwidth, 
              successcnt, bitwidth,
              seconds_since_start);
      FILE* ft = fopen("mae.tmp", "wb");
      for(int i = 0; i < res_len; i++) {
          putc(res[i], ft);
      }
      if(RECORDBITS) {
        FILE* fb = fopen("mae.pbits", "wb");
        for(int i = 0; i < pbits.size(); i++) {
            putc(pbits[i] * 255 / 4095, fb);
        }
        FILE* rb = fopen("mae.rbits", "wb");
        for(int i = 0; i < rbits.size(); i++) {
            putc(rbits[i] * 255 / 4095 , rb);
        }
        for(int i = 0; i < 10; i++) {
            printf("pbits[%d]: %d, rbits[%d]: %d\n", i, pbits[i] * 255 / 4095, i, rbits[i] * 255 / 4095);
        }
      }
    } else {
      FILE* ft = fopen("mae.tmp", "rb");
      U8* res;
      int res_len;
      fseek(ft, 0, SEEK_END);
      res_len = ftell(ft);
      fseek(ft, 0, SEEK_SET);
      res = (U8*)malloc(res_len * sizeof(U8));
      for(int i = 0; i < res_len; i++) {
          res[i] = getc(ft);
      }
      int* origin1 = (int*)malloc(n * sizeof(int));
      decompress(res, res_len, origin1, n);
      for(int i = 0; i < n; i++) {
            // printf("origin[%d]: %d, origin1[%d]: %d\n", i, origin[i], i, origin1[i]);
            if(origin[i] != origin1[i]) {
                printf("origin[%d]: %d, origin1[%d]: %d\n", i, origin[i], i, origin1[i]);
                break;
            }
      }
    }
    return 0;
}

