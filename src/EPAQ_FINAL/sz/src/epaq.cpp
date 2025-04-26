#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utility>
#include <unistd.h>
#include <assert.h>
#include <malloc.h>

// 8, 16, 32 bit unsigned types (adjust as appropriate)
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;

void quit(const char* message=0) {
  if (message) printf("%s\n", message);
  exit(1);
}

// Create an array p of n elements of type T
template <class T> void alloc(T*&p, int n) {
  p=(T*)calloc(n, sizeof(T));
  if (!p) quit("out of memory");
}

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
#define nex(state, sel) State_table[state][sel];

int dt[1024];
class StateMap {
protected:
  const int N;  // Number of contexts
  int cxt;      // Context of last prediction
  U32 *t;       // cxt -> prediction in high 22 bits, count in low 10 bits
  int updating;
  void update(int y, int limit) {
    assert(cxt>=0 && cxt<N);
    int n=t[cxt]&1023, p=t[cxt]>>10; 
    if (n<limit) ++t[cxt];
    t[cxt]+=(((y<<22)-p)>>3)*dt[n]&0xfffffc00;
  }
public:
  StateMap(int n=256) : N(n), cxt(0) {
    alloc(t, N);
    for (int i=0; i<N; ++i) t[i]=1<<31;
    updating = 1;
  }
  void stop_learning() { updating = 0; }
  int p(int y, int cx, int limit=1023) {
    if(updating) update(y, limit);
    return t[cxt=cx]>>20;
  }
};


class APM: public StateMap {
public:
  APM(int n): StateMap(n*24) {
    for (int i=0; i<N; ++i) {
      int p=((i%24*2+1)*4096)/48-2048;
      t[i]=(U32(squash(p))<<20)+6;
    }
  }
  int pp(int y, int pr, int cx, int limit=255) {
    assert(y>>1==0);
    assert(pr>=0 && pr<4096);
    assert(cx>=0 && cx<N/24);
    assert(limit>0 && limit<1024);
    if(updating) update(y, limit);
    pr=(stretch(pr)+2048)*23;
    int wt=pr&0xfff;  // interpolation weight of next element
    cx=cx*24+(pr>>12);
    assert(cx>=0 && cx<N-1);
    pr=((t[cx]>>13)*(0x1000-wt)+(t[cx+1]>>13)*wt)>>19;
    cxt=cx+(wt>>11);
    return pr;
  }
};

inline void train(int *t, int *w, int n, int err) {
  for (int i=0; i<n; ++i)
    w[i]+=(t[i]*err+0x8000)>>16;
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
  Mixer(int n, int m):
    N(n), M(m), tx(0), wx(0), cxt(0), nx(0), pr(2048) {
    assert(n>0 && N>0 && M>0);
    alloc(tx, N);
    alloc(wx, N*M);
  }

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


class Predictor {
  int pr;  // next prediction
  int updating;
  StateMap sm0, sm1;
  APM a1, a2;
  Mixer m;
public:
  Predictor(): pr(2048), updating(1), a1(0x100), a2(0x4000), m(2, 80) {}
  void stop_learning() { 
    updating = 0;
  }
  int p() const {
    assert(pr>=0 && pr<4096);
    return pr;
  }
  void update(int y);
};


static int hash_table[65535];
void init_hash_table() {
  for (int i=0; i<1024; ++i) dt[i]=16384/(i+i+3);
  for(int i = 0; i < 65535; i++) {
    hash_table[i] = i;
  }
  srand(0);  
  for(int i = 0; i < 65535; i++) {
    int j = rand() % 65535;
    std::swap(hash_table[i], hash_table[j]); 
  }
}

int hash(int x) {
  int x1 = x & 0xffff;
  int x2 = (x >> 16) & 0xffff;
  return hash_table[x1] ^ hash_table[x2];
}


int cnt = 0;
int bitwidth = 0;
void Predictor::update(int y) {
  int ret = (pr > 2048 && y) || (pr < 2048 && !y);
  static U8 t0[0x10000], t1[0x10000];
  static int c0 = 1, c1 = 0;  // last 4 bytes
  static int offset0 = 0, offset1 = 0;  // hash key for order 3 state
  static int hoffset1 = 0;
  static int bcount = 0;  // bit count
  static StateMap sm0;  // state map for order 1
  static StateMap sm1;  // state map for order 3
  static APM a1(0x100), a2(0x4000);
  static U32 h0, h1;
  static Mixer m(2, 80);

  if (cnt == bitwidth - 1) {
    cnt = 0;
    bcount = 0;
    c0 = 1;
    c1 = 0;
  } else {
    cnt++;
  }

  // Update model
  assert(y == 0 || y == 1);
  t0[offset0] = nex(t0[offset0], y);           // Update order 1 state
  t1[hoffset1] = nex(t1[hoffset1], y);         // Update order 3 state
  m.update(y);

  // Update context
  ++bcount;
  c0 += c0 + y;
  if (c0 >= 256) {
    c0 -= 256;
    c1 = c1 << 8 | c0;
    h0 = c0 << 8;        // order 1
    h1 = (c1 << 8) * 3;  // order 3
    offset1 = h1;        // Set hash key for new context
    c0 = 1;
    bcount = 0;
  }
  if (bcount == 4) {
    offset1 = h1 + c0;  // Update hash key with new byte
  } else if (bcount > 0) {
    int j = (y + 1) << ((bcount & 3) - 1);
    offset1 += j;         // Adjust hash key within context
  }
  offset0 = h0 + c0;    // Update order 1 offset

  // Predict
  int order = 0;
  hoffset1 = hash(offset1);
  if (t1[hoffset1]) ++order;  // Check if order 3 state is non-zero
  m.add(stretch(sm0.p(y, t0[offset0])));
  m.add(stretch(sm1.p(y, t1[hoffset1])));
  m.set(order + ((h0 >> 13) << 1));
  pr = m.p();
  pr = (pr + 3 * a1.pp(y, pr, c0)) >> 2;
  pr = (pr + 3 * a2.pp(y, pr, c0 ^ h0 >> 2)) >> 2;
}


class Encoder {
private:
  Predictor predictor;
public:
  U8* origin;
  U32 x1, x2, x;
  int totalCnt;
  int tmpsize;
  Encoder(U32 initialx = 0, U8* originbits = NULL) {
    x1 = 0, x2 = 0xffffffff;
    totalCnt = 0, tmpsize = 0;
    x = initialx;
    origin = originbits;
  }
  void code(int y = 0, U8** outputbits = NULL, bool reset = false) {
    int p = predictor.p();
    p += p < 2048;
    U32 xmid=x1 + ((x2-x1)>>12)*p + ((x2-x1&0xfff)*p>>12);
    y ? (x2=xmid) : (x1=xmid+1);
    predictor.update(y);
    while (((x1^x2)&0xff000000)==0) {
      // printf("code output\n");
      if(tmpsize<=totalCnt) {
        U8* buffer;
        alloc(buffer, tmpsize*2);
        memcpy(buffer, *outputbits, totalCnt);
        free(*outputbits);
        *outputbits = buffer;
        tmpsize *= 2;
      }
      (*outputbits)[totalCnt++] = x2>>24;
      x1<<=8;
      x2=(x2<<8)+255;
    }
  }
  int decode() {
    int p = predictor.p();
    p += p < 2048;
    U32 xmid=x1 + ((x2-x1)>>12)*p + ((x2-x1&0xfff)*p>>12);
    int y = x<=xmid;
    y ? (x2 = xmid) : (x1 = xmid + 1);
    predictor.update(y);
    while(((x1^x2)&0xff000000) == 0) {
      x1<<=8;
      x2=(x2<<8)+255;
      x = (x << 8) + (origin[totalCnt++] & 255);
    }
    return y;
  }
  U8 decodebyte() {
    U8 c = 0;
    for(int i = 0; i < 8; i++) {
      c += c + decode();
    }
    return c;
  }
};

extern "C" void epaqdecompress(int memLevel, int inlength, U8* inputbits, U8* outputbits, int outlength, int bitwidth);
extern "C" void epaqcompress(int memLevel, int inlength, U8* inputbits, U8** outputbits, size_t* outlengt, int bitwidth);

void epaqcompress(int memLevel, int inlength, U8* inputbits, U8** outputbits, size_t* outlength, int bitpackwidth) {
  init_hash_table();
  bitwidth = bitpackwidth;
  U8* tmp = (U8*)malloc(inlength);
  Encoder encoder;
  encoder.tmpsize = inlength;
  for(int i = 0; i < inlength; i++) {
    for(int cur = i*4+3; cur >= i*4; cur--){
      for (int j = 7; j >= 0; --j) {
        encoder.code((inputbits[cur]>>j)&1, &tmp);
	  }
    }
  }
  *outputbits = (U8 *)malloc((encoder.totalCnt + 1) * sizeof(U8));
  memcpy(*outputbits, tmp, encoder.totalCnt + 1);
  (*outputbits)[encoder.totalCnt++] = encoder.x1 >> 24;
  *outlength = encoder.totalCnt;
}

void epaqdecompress(int memLevel, int inlength, U8* inputbits, U8* outputbits, int outlength, int bitpackwidth) {
  init_hash_table();
  bitwidth = bitpackwidth;
  U32 x = 0;
  for (int i = 0; i < 4; ++i) x = (x << 8)+ (inputbits[i] & 255);
  Encoder encoder = Encoder(x, inputbits + 4);
  for(int i = 0; i < outlength/4; i++) {
    outputbits[i*4+3] = encoder.decodebyte();
    outputbits[i*4+2] = encoder.decodebyte();
    outputbits[i*4+1] = encoder.decodebyte();
    outputbits[i*4] = encoder.decodebyte();
  }
}
