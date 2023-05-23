# 设计 hash 映射

需求：

- 哈希函数要计算快；
- 哈希函数要尽可能减少冲突；
- 负责均衡，是否支持 `rehash`；

功能：

- 基本功能：哈希表操作（create/destroy）；键值操作（get/set/remove）
- 哈希函数：`jhash`（kernel使用）、`siphash`（redis、rust使用）、`memhash`（golang使用）
- `rehash`：当冲突增多时，需要增大桶数，进行重新hash。（循序渐进）

哈希函数选型维度：

- 计算速度
- 冲突概率
- 安全性

比如：

- siphash：虽然不是最快的哈希算法，但可以抵抗`hash-flooding`攻击；（通过引入密钥）

哈希函数通常分两种：

- 加密型：用于加密数据，数字摘要；（sha256、aes256）
- 非加密型：用于查找；（siphash、cityhash）

参考：

https://burtleburtle.net/bob/hash/

https://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/

https://isdanni.com/hash-flooding/

https://en.wikipedia.org/wiki/Bloom_filter
