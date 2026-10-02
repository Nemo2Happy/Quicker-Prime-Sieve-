目录下共五个文件，适用于不同的CPU缓存大小。

以下是使用范围建议：

|文件名|算法|建议CPU缓存大小|运算 $10^9$ 速度|运算 $10^{10}$ 速度|
|-----|-----|-----|-----|-----|
|`superprime1_low.exe`|普通分块埃氏筛|2MB或以下|7.17s|120.53s|
|`superprime2_medium.exe`|6轮筛法|2MB~4MB|3.70s|83.95s|
|`superprime3_pro.exe`|30轮筛法|4MB~12MB|2.82s|64.44s|
|`superprime4_high.exe`|210轮筛法|12MB~32MB|2.47s|54.60s|
|`superprime5_max.exe`|2310轮筛法|不建议使用|3.00s|62.92s|

其中，速度的测试环境为AMD Ryzen R5 5600X(Up to 4.8GHz)+24GB DDR4 3200 RAM，不写入文件。
