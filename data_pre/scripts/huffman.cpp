#include <vector>
#include <fstream>
#include <queue>
#include <set>
#include <assert.h>
using namespace std;
#define N 256
int freq[N] = {0};
typedef struct Node {
    int cnt;
    int cur;
    int pos;
    int left;
    int right;
    int father;
    bool operator<(const Node& other) const {
        return cnt > other.cnt;
    }
}node;
vector<int> codes[N];
vector<node> nodes;
int tot;

void buildCodes(int now, vector<int> curcode) {
    if(nodes[now].cur != -1) {
        codes[nodes[now].cur] = curcode;
        return;
    }
    curcode.push_back(0);
    buildCodes(nodes[now].left, curcode);
    curcode.pop_back();
    curcode.push_back(1);
    buildCodes(nodes[now].right, curcode);
    curcode.pop_back();
}
int main(int argc, char *argv[]) {
    FILE* file = fopen(argv[1], "rb");
    int c;
    while (c = getc(file), c != EOF) {
        freq[c]++;
    }
    fclose(file);
    priority_queue<node> q;
    for(int i = 0; i < N; i++) {
        node z = node{freq[i], i, tot++, -1, -1, -2};
        nodes.push_back(z);
        q.push(z);
    }
    while(q.size() != 1) {
        node x = q.top(); q.pop();
        node y = q.top(); q.pop();
        node z = {x.cnt + y.cnt, -1, tot++, x.pos, y.pos, -2};
        nodes[x.pos].father = z.pos;
        nodes[y.pos].father = z.pos;
        nodes.push_back(z);
        q.push(z);
    }
    assert(tot == N * 2 -1);
    int head = q.top().pos;
    vector<int> vec;
    buildCodes(head, vec);
    // print codes
    // for(int i = 0; i < N; i++) {
    //     printf("%d: ", i);
    //     for(int j = 0; j < codes[i].size(); j++) {
    //         printf("%d", codes[i][j]);
    //     }
    //     printf("\n");
    // }
    // write codes;
    file = fopen(argv[1], "rb");
    FILE* file1 = fopen(argv[2], "wb");
    // write freq first 
    fwrite(freq, sizeof(int), N, file1);
    deque<int> buffer;
    while (c = getc(file), c != EOF) {
        for(auto i: codes[c]){
            buffer.push_back(i);
        }
        while(buffer.size() >= 8) {
            int tmp = 0;
            for(int i = 0; i < 8; i++) {
                tmp = tmp * 2 + buffer.front();
                buffer.pop_front();
            }
            putc(tmp, file1);
        }
    }
    // the last byte
    if(buffer.size()) {
        while(buffer.size() < 8) buffer.push_back(0);
        int tmp = 0;
        for(int i = 0; i < 8; i++) {
            tmp = tmp * 2 + buffer[i];
        }
        putc(tmp, file1);
    }
    fclose(file);
    fclose(file1);
    return 0;
}
