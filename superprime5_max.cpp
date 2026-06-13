#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
#include<conio.h>
#include<windows.h>
#include<time.h>
#include<sstream>
#include<cstring>
#include<algorithm>
#define int long long
using namespace std;

int stt, yyy;
void v() {}

// 2310 轮式筛参数（2310 = 2*3*5*7*11）
const int WHEEL = 2310;
vector<int> RESIDUES;   // 动态生成 φ(2310)=480 个互素余数
int RN;                 // 余数个数

struct PrimeWheelInfo {
    int p;
    vector<int> start;   // 每个余数对应的第一个倍数 c
};
vector<PrimeWheelInfo> wheelPrime;

int inv_mod(int a, int m) {
    int m0 = m, t, q;
    int x0 = 0, x1 = 1;
    if (m == 1) return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;
    return x1;
}

void build_residues() {
    if (!RESIDUES.empty()) return;
    vector<bool> is_coprime(WHEEL, true);
    int small_primes[] = {2,3,5,7,11};
    for (int p : small_primes) {
        for (int m = p; m < WHEEL; m += p)
            is_coprime[m] = false;
    }
    for (int i = 1; i < WHEEL; ++i) {
        if (is_coprime[i]) RESIDUES.push_back(i);
    }
    RN = RESIDUES.size();   // 480
}

int calc(int n) {
    build_residues();
    ofstream out(to_string(n) + "underprime.txt");
    
    int rem2idx[WHEEL];
    memset(rem2idx, -1, sizeof(rem2idx));
    for (int i = 0; i < RN; ++i) rem2idx[RESIDUES[i]] = i;
    
    int limit = sqrt(n);
    vector<unsigned char> ispri(limit + 1, 1);
    vector<int> primes;
    for (int i = 2; i <= limit; ++i) {
        if (ispri[i]) {
            primes.push_back(i);
            if (i * i <= limit) {
                for (int j = i * i; j <= limit; j += i)
                    ispri[j] = 0;
            }
        }
    }
    
    // 初始计数：只处理 2,3,5,7,11（因为 13 及以上与 2310 互素，会被轮筛捕捉）
    int small_primes[] = {2,3,5,7,11};
    int res = 0;
    for (int p : small_primes) if (p <= n) ++res;
    if (yyy) {
        for (int p : small_primes) if (p <= n) out << p << ",";
    }
    
    // 分段大小：候选数组约 28MB，适应 32MB L3 缓存
    const int TARGET_CACHE_MB = 28;
    int seg_size = max((int)sqrt(n), WHEEL * 8000);
    seg_size = (seg_size + WHEEL - 1) / WHEEL * WHEEL;
    int cnt_per_block = (seg_size / WHEEL) * RN;
    vector<unsigned char> cand(cnt_per_block, 1);
    
    int tot_blocks = (n + seg_size - 1) / seg_size;
    int log10n = (int)log10(n);
    int progress_step = max(tot_blocks / (max(log10n * log10n / 4 - 20, 1LL) * 100), 1LL);
    
    // 预计算素数（>=13）的轮信息
    wheelPrime.clear();
    wheelPrime.reserve(primes.size());
    for (int p : primes) {
        if (p < 13) continue;   // 2,3,5,7,11 已单独处理，p=13 开始进入轮筛
        PrimeWheelInfo info;
        info.p = p;
        info.start.resize(RN);
        int inv = inv_mod(p % WHEEL, WHEEL);
        for (int ri = 0; ri < RN; ++ri) {
            int rr = RESIDUES[ri];
            int c0 = (rr * inv) % WHEEL;
            if (c0 == 0) c0 = WHEEL;
            int cmin = c0;
            if (cmin < p) {
                int delta = ((p - cmin + WHEEL - 1) / WHEEL) * WHEEL;
                cmin += delta;
            }
            info.start[ri] = cmin;
        }
        wheelPrime.push_back(info);
    }
    
    for (int k = 0; k < tot_blocks; ++k) {
        int L = k * seg_size;
        int R = min(L + seg_size - 1, n);
        
        memset(cand.data(), 1, cnt_per_block);
        
        if (k % progress_step == 0 || k == tot_blocks - 1) {
            double pct = (double)k / tot_blocks * 100.0;
            printf("(%.2fs) Complete %.2f%% (block %lld/%lld)\n",
                   (clock() - stt) / 1000.0, pct, (long long)k, (long long)tot_blocks);
        }
        
        for (const auto &info : wheelPrime) {
            int p = info.p;
            int min_c_for_L = (L + p - 1) / p;
            int max_c = R / p;
            if (min_c_for_L > max_c) continue;
            
            for (int ri = 0; ri < RN; ++ri) {
                int c = info.start[ri];
                if (c < min_c_for_L) {
                    int diff = min_c_for_L - c;
                    int delta = (diff + WHEEL - 1) / WHEEL * WHEEL;
                    c += delta;
                }
                for (; c <= max_c; c += WHEEL) {
                    int val = p * c;
                    int offset = val - L;
                    int idx = (offset / WHEEL) * RN + rem2idx[offset % WHEEL];
                    cand[idx] = 0;
                }
            }
        }
        
        // 输出本段素数
        const int BUF_SIZE = 65536;
        char writeBuf[BUF_SIZE];
        int writePos = 0;
        auto flush = [&]() {
            if (writePos > 0 && yyy) {
                out.write(writeBuf, writePos);
                writePos = 0;
            }
        };
        auto addNum = [&](int num) {
            if (!yyy) return;
            char tmp[32];
            int len = sprintf(tmp, "%lld,", num);
            if (writePos + len >= BUF_SIZE) flush();
            memcpy(writeBuf + writePos, tmp, len);
            writePos += len;
        };
        
        for (int i = 0; i < cnt_per_block; ++i) {
            int num = L + (i / RN) * WHEEL + RESIDUES[i % RN];
            if (num > n) break;
            // 跳过已经单独处理的素数（2,3,5,7,11），但保留 13 及以上的候选
            if (num <= 11) continue;
            if (cand[i]) {
                ++res;
                addNum(num);
            }
        }
        flush();
    }
    
    out.close();
    return res;
}

int bl(int n) {
    ofstream out(to_string(n) + "underprime.txt");
    int res = 0;
    for (int i = 2; i <= n; ++i) {
        bool yn = true;
        for (int j = 2; j * j <= i; ++j) {
            if (i % j == 0) { yn = false; break; }
        }
        if (yn) ++res;
        if (yn && yyy) out << i << ",";
    }
    out.close();
    return res;
}

signed main() {
    int n;
    printf("请输入要计算素数的范围：");
    cin >> n;
    yyy = 0;
    while (n < 2) {
        cout << "输入不合法\n请重新输入";
        cin >> n;
    }
    double cost = ((n / log(n)) * (log10(n) + 1) / 1024 / 1024) * 1.04;
    printf("预计将会写入文件%s，大小约%.2lfMB，是否写入文件？Y/y写入文件，N/n不写入文件\n",
           (to_string(n) + "underprime.txt").c_str(), cost);
    char ch = getch();
    while (ch != 'y' && ch != 'n') ch = getch();
    if (ch == 'y') yyy = 1;
    stt = clock();
    int m = (n <= 100000 ? bl(n) : calc(n));
    if (yyy) {
        string fname = to_string(n) + "underprime.txt";
        printf("共%lld个素数，耗时%lldms(约%.2f秒)，已写入文件%s\n",
               m, (long long)(clock() - stt), (clock() - stt) / 1000.0, fname.c_str());
    } else {
        printf("共%lld个素数，耗时%lldms(约%.2f秒)\n",
               m, (long long)(clock() - stt), (clock() - stt) / 1000.0);
    }
    printf("按任意键继续......");
    getch();
    return 0;
}