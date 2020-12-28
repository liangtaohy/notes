对比与https://www.zhihu.com/question/20103344 的差异

# 需求和目标（Goal）[Scenario-场景和功能]
## 功能需求
* 针对给定的URL，生成一个短的且唯一的别名
* 当用户访问该短链接时，服务应该重定向到原地址
* 链接将在标准的默认时间间隔后过期。用户应该能够指定过期时间。
## 非功能需求
* HA: 系统应该是高可用的
* 实时性：URL重定向应实时进行，延迟最小
* 不可预测性：缩短的链接不应该是可猜测的（不可预测的）
* 热点链接缓存

# 容量预估和约束 [Necessary - 约束]

Read Heavy (假设读写比例为100:1)

## Traffic Estimates(流量预估)

假设一个月有500M的新URL。

* 写：200 URLs/s
* 读： 20K URLs/s

## Storage estimates (存储预估)

假设每个URL被存储5年，平均大小为500字节。


* 500 million * 5 years * 12 months = 30 billion

## Bandwidth estimates (带宽预估)

* 对于写请求：qps * one request size = 200 * 500 bytes = 100 KB/s

* 对于读请求：qps * one request size = 20K * 500 bytes = 10 MB/s

## Memory estimates (内存预估)

如果我们想缓存热点URL，我们需要多少内存呢？

基于80-20原则，20%的URL产生80%的流量，我们倾向于缓存20%的热点URL。

* 20K * 3600 seconds * 24 hours = ~1.7 billion （一天的请求量）
* 0.2 * 1.7 billion * 500 bytes = ~170GB (缓存一天所需内存容量)
# 定义系统API
'''createURL(api_dev_key, original_url, custom_alias=None, user_name=N one, expire_date=None)
deleteURL(api_dev_key, url_key)'''
* 如何检测和预防滥用？

对用户做读写限流

# 数据库设计 [Kilobit - 存储和数据结构]

* ER图
* 存储选型

# 基本的系统设计与算法 [Application - Algorithm - 算法]

核心问题是：如何生成一个短的且唯一的key？

## 思路A: Hash
hash code -> encoding (36,64,72等)
存在的问题：
Hash冲突，即key不唯一
如果多个用户输入同一个Url,会生成同样的key
encoded url如何保证key的唯一性

## 思路B: KGS (Key Generation Service)
提前随机生成好6位字符串，并存储到数据库中

## 思路C: 自增序列

long -> encoding

存在的问题：
有序，可预测
同一个url多次请求key不同
单点问题：双发号器-- 一个发单号，一个发双号

有并发问题么？

# 数据分片和副本 [Evolve - HA]

## Partition
range-based: 平衡性差

hash-based: overloaded partitions (即热点问题) (使用一致性哈希解决)

# 缓存 [Evolve]

# Purging Or DB清除

* 每当用户试图访问过期链接时，我们可以删除该链接并向用户返回错误。
* 一个单独的清理服务可以定期运行，从我们的存储和缓存中删除过期的链接。这个服务应该是非常轻量级的，并且只能在预期的用户流量较低时运行
* 我们可以为每个链接设置一个默认的过期时间（例如，两年）。
* 删除过期链接后，我们可以将key放回数据库以供重用。