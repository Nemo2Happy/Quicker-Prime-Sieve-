#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
#include<conio.h>
#include<windows.h>
#include<time.h>
#include<sstream>
#include<algorithm>
#include<cstring>
#define int long long
using namespace std;

int stt, yyy;
void v() {}

// 30 轮式筛参数
const int WHEEL = 30;
const int RN = 8;
const int RESIDUES[RN] = {1,7,11,13,17,19,23,29};

// 预计算的素数轮信息（修正版）
struct PrimeWheelInfo {
    int p;
    int start[RN];   // 每个余数对应的第一个倍数 c（全局 c，不是索引）
    int step;        // = p * WHEEL
};
vector<PrimeWheelInfo> wheelPrime;

int calc(int n) {
    ofstream out(to_string(n) + "underprime.txt");
    
    // 余数到索引的映射表
    int rem2idx[WHEEL];
    memset(rem2idx, -1, sizeof(rem2idx));
    for (int i = 0; i < RN; ++i) rem2idx[RESIDUES[i]] = i;
    
    // 筛出 sqrt(n) 以内的素数
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
    
    // 初始计数：2,3,5
    int res = 0;
    for (int p : {2,3,5}) if (p <= n) ++res;
    if (yyy) out << "2,3,5,";
    
    // 分段大小：使每个分段内候选数组约 8~16 MB，适应 L3 缓存
    int seg_size = max((int)sqrt(n), 30ll * 50000);   // 每段约 50000 个轮周期
    seg_size = (seg_size + WHEEL - 1) / WHEEL * WHEEL;
    int cnt_per_block = (seg_size / WHEEL) * RN;    // 候选数个数
    vector<unsigned char> cand(cnt_per_block, 1);
    
    int tot_blocks = (n + seg_size - 1) / seg_size;
    int log10n = (int)log10(n);
    int progress_step = max(tot_blocks / (max(log10n * log10n / 4 - 20, 1LL) * 100), 1LL);
    
    // 预计算所有素数（>=7）的轮信息
    wheelPrime.clear();
    wheelPrime.reserve(primes.size());
    for (int p : primes) {
        if (p < 7) continue;
        PrimeWheelInfo info;
        info.p = p;
        info.step = p * WHEEL;
        for (int ri = 0; ri < RN; ++ri) {
            int rr = RESIDUES[ri];
            // 最小的 c 满足 p*c ≡ rr (mod WHEEL) 且 c >= p
            // 解同余方程：c ≡ rr * p^{-1} (mod WHEEL)
            int inv = 0;
            for (int k = 1; k < WHEEL; ++k) {
                if ((p * k) % WHEEL == 1) { inv = k; break; }
            }
            int c0 = (rr * inv) % WHEEL;
            if (c0 == 0) c0 = WHEEL;
            int cmin = c0;
            // 调整到 >= p
            if (cmin < p) {
                int delta = ((p - cmin + WHEEL - 1) / WHEEL) * WHEEL;
                cmin += delta;
            }
            info.start[ri] = cmin;
        }
        wheelPrime.push_back(info);
    }
    
    // 分段筛选主循环
    for (int k = 0; k < tot_blocks; ++k) {
        int L = k * seg_size;
        int R = min(L + seg_size - 1, n);
        
        // 重置标记数组（使用 memset 更快）
        memset(cand.data(), 1, cnt_per_block);
        
        // 进度输出（避免浮点运算过多）
        if (k % progress_step == 0 || k == tot_blocks - 1) {
            double pct = (double)k / tot_blocks * 100.0;
            printf("(%.2fs) Complete %.2f%% (block %lld/%lld)\n",
                   (clock() - stt) / 1000.0, pct, (long long)k, (long long)tot_blocks);
        }
        
        // 用预计算的轮信息筛除合数
        for (const auto &info : wheelPrime) {
            int p = info.p;
            int step = info.step;
            // 当前分段的最小倍数边界
            int min_c_for_L = (L + p - 1) / p;
            int max_c = R / p;
            if (min_c_for_L > max_c) continue;
            
            for (int ri = 0; ri < RN; ++ri) {
                int c = info.start[ri];
                // 调整 c 到当前分段范围内
                if (c < min_c_for_L) {
                    int diff = min_c_for_L - c;
                    int delta = (diff + WHEEL - 1) / WHEEL * WHEEL;
                    c += delta;
                }
                // 标记合数
                for (; c <= max_c; c += WHEEL) {
                    int val = p * c;
                    int offset = val - L;
                    int idx = (offset / WHEEL) * RN + rem2idx[offset % WHEEL];
                    cand[idx] = 0;
                }
            }
        }
        
        // 输出本段素数（使用 fwrite 替代 stringstream 加速）
        const int BUF_SIZE = 65536;
        char writeBuf[BUF_SIZE];
        int writePos = 0;
        auto flushBuf = [&]() {
            if (writePos > 0 && yyy) {
                out.write(writeBuf, writePos);
                writePos = 0;
            }
        };
        auto addToBuf = [&](int num) {
            if (!yyy) return;
            char tmp[32];
            int len = sprintf(tmp, "%lld,", num);
            if (writePos + len >= BUF_SIZE) flushBuf();
            memcpy(writeBuf + writePos, tmp, len);
            writePos += len;
        };
        
        for (int i = 0; i < cnt_per_block; ++i) {
            int num = L + (i / RN) * WHEEL + RESIDUES[i % RN];
            if (num > n) break;
            if (num < 7) continue;
            if (cand[i]) {
                ++res;
                addToBuf(num);
            }
        }
        flushBuf();
    }
    
    out.close();
    return res;
}

int bl(int n) {
    ofstream out(to_string(n) + "underprime.txt");
    int res = 0;
    for (int i = 2; i <= n; ++i) {
        int yn = 1;
        for (int j = 2; j * j <= i; ++j) {
            if (i % j == 0) { yn = 0; break; }
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