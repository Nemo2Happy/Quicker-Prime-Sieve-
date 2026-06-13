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

// 优化后的分段筛法
int calc(int n) {
    ofstream out(to_string(n) + "underprime.txt");
    
    // 1. 生成 sqrt(n) 以内的素数
    int limit = sqrt(n);
    vector<char> ispri(limit + 1, 1);
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
    
    // 2. 分段大小：使用约 1MB 的块（可调整，适应 L2/L3 缓存）
    const int BLOCK_SIZE = 1 << 18;   // 1048576
    int tot_blocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int log10n = (int)log10(n);
    int progress_step = max(tot_blocks / (max(log10n * log10n / 4 - 20, 1LL) * 100), 1LL);
    
    // 3. 结果计数（2 单独处理，因为分段筛从 2 开始但块边界需统一）
    int res = (n >= 2) ? 1 : 0;
    if (yyy && n >= 2) out << "2,";
    
    // 4. 主分段循环
    for (int k = 0; k < tot_blocks; ++k) {
        int L = k * BLOCK_SIZE;
        int R = min(L + BLOCK_SIZE - 1, n);
        
        // 分配当前块的标记数组（char 节省内存，且比 vector<bool> 快）
        int seg_len = R - L + 1;
        vector<char> block(seg_len, 1);
        
        // 进度输出
        if (k % progress_step == 0 || k == tot_blocks - 1) {
            double pct = (double)k / tot_blocks * 100.0;
            printf("(%.2fs) Complete %.2f%% (block %lld/%lld)\n",
                   (clock() - stt) / 1000.0, pct, (long long)k, (long long)tot_blocks);
        }
        
        // 对每个素数 p 标记合数
        for (int p : primes) {
            // 找到 p 在 [L, R] 中的第一个倍数
            int start = max(p * p, ((L + p - 1) / p) * p);
            for (int j = start; j <= R; j += p) {
                block[j - L] = 0;
            }
        }
        
        // 特殊处理 0 和 1（仅当 L == 0 时）
        if (L == 0) {
            if (0 <= R) block[0] = 0;
            if (1 <= R) block[1] = 0;
        }
        
        // 输出本块素数（使用固定缓冲区代替 stringstream）
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
        
        for (int i = 0; i < seg_len; ++i) {
            int num = L + i;
            if (num < 2) continue;
            if (block[i]) {
                ++res;
                addNum(num);
            }
        }
        flush();
    }
    
    out.close();
    return res;
}

// 朴素算法（n 较小时使用）
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