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

// 模6轮式筛参数
const int WHEEL = 6;
const int RESIDUES[] = {1, 5};
const int RN = 2;

// 预计算的素数轮信息
struct PrimeWheelInfo {
    int p;
    int start[RN];   // 每个余数对应的第一个倍数c (满足p*c ≡ rr mod 6)
};
vector<PrimeWheelInfo> wheelPrime;

int calc(int n) {
    ofstream out(to_string(n) + "underprime.txt");
    
    // 余数 -> 索引映射
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
    
    // 初始计数：2 和 3
    int res = 2;
    if (yyy) out << "2,3,";
    
    // 分段大小：使每个分段内候选数组约 512KB~1MB，适应L2/L3缓存
    int seg_size = max((int)sqrt(n), 6ll * 100000);
    seg_size = (seg_size + WHEEL - 1) / WHEEL * WHEEL;
    int cnt_per_block = (seg_size / WHEEL) * RN;
    vector<unsigned char> cand(cnt_per_block, 1);
    
    int tot_blocks = (n + seg_size - 1) / seg_size;
    int log10n = (int)log10(n);
    // 进度步长：总共输出约 100 * (log10n^2/4 - 20) 次，但限制最少为1
    int progress_step = max(tot_blocks / (max(log10n * log10n / 4 - 20, 1LL) * 100), 1LL);
    
    // 预计算每个素数（>=5）的轮信息
    wheelPrime.clear();
    wheelPrime.reserve(primes.size());
    for (int p : primes) {
        if (p < 5) continue;
        PrimeWheelInfo info;
        info.p = p;
        // 对于模6，预先计算每个余数对应的最小c (c >= p 且 p*c ≡ rr mod 6)
        // 解同余方程：c ≡ rr * inv(p) mod 6。由于p与6互素（p>=5），inv存在。
        int inv = 0;
        for (int k = 1; k < WHEEL; ++k) {
            if ((p * k) % WHEEL == 1) { inv = k; break; }
        }
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
    
    // 分段筛选主循环
    for (int k = 0; k < tot_blocks; ++k) {
        int L = k * seg_size;
        int R = min(L + seg_size - 1, n);
        
        // 重置候选标记（使用 memset 加速）
        memset(cand.data(), 1, cnt_per_block);
        
        // 进度输出（修正百分比基于块数）
        if (k % progress_step == 0 || k == tot_blocks - 1) {
            double pct = (double)k / tot_blocks * 100.0;
            printf("(%.2fs) Complete %.2f%% (block %lld/%lld)\n",
                   (clock() - stt) / 1000.0, pct, (long long)k, (long long)tot_blocks);
        }
        
        // 用预计算信息筛除合数
        for (const auto &info : wheelPrime) {
            int p = info.p;
            int min_c_for_L = (L + p - 1) / p;
            int max_c = R / p;
            if (min_c_for_L > max_c) continue;
            
            for (int ri = 0; ri < RN; ++ri) {
                int c = info.start[ri];
                // 将c调整到当前分段内
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
        
        // 输出本段素数（使用固定缓冲区代替stringstream）
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
            if (num == 1) continue;
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