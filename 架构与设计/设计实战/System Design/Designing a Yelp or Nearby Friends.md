# 需求和目标 [Scenario-场景和功能]

我们希望通过类似Yelp的服务实现什么？我们的服务将存储不同地方的信息，以便用户可以对其进行搜索。查询后，我们的服务将返回用户周围的位置列表。

## 功能性需求

* 用户应该能够添加/删除/更新位置。
* 给定他们的位置（经度/纬度），用户应该能够找到给定半径范围内的所有附近地点。
* 用户应该能够添加关于某个位置的反馈/评论。反馈可以有图片、文本和评分

## 非功能性需求

* 用户应该拥有一个最小延迟的实时查询体验
* 我们的服务应该支持繁重的搜索负载。与添加新地点相比，我们的服务会有很多搜索请求

# 容量评估（水平扩展能力评估）[Necessary - 约束]

* 带宽评估

* 流量评估

* 存储评估

* 内存评估

# 数据模型 [Kilobit - 存储和数据结构]

每个位置都可以有如何字段：

1. LocationID (8 bytes): Uniquely identifies a location.
2. Name (256 bytes)
3. Latitude (8 bytes)
4. Longitude (8 bytes)
5. Description (512 bytes)
6. Category (1 byte)

我们还需要存储一个位置的评论、照片和评级。我们可以有一个单独的表来存储这些信息。

1. LocationID (8 bytes)
2. ReviewID (4 bytes): 假设一个位置最多有 2^32 个评论
3. ReviewText (512 bytes)
4. Rating (1 byte)

# 核心接口

(return json) search(api_dev_key, search_terms, user_location, radius_filter, maximum_results_to_return, category_filter, sort, page_token)

* api_dev_key: 注册帐户的API开发人员密钥。用户的配额将基于此分配给其他用户。国内通常为appid。
* search_terms（string）：包含搜索项的字符串。
* user_location: 用户位置
* radius_filter (number): 可选的搜索半径，单位为米
* maximum_results_to_return (number): 可返回的业务结果数
* category_filter (string) : 分类
* sort (string) : 排序方式，如：最佳匹配（0-默认）、最小距离（1）、最高额定值（2）。

# 基本的系统设计和算法 [Alg]

以下设计基于假设：一个地方的位置不会经常改变，我们不需要担心数据的频繁更新。

## 1. SQL Solution

一个简单的解决方案是将所有数据存储在像MySQL这样的数据库中。每个位置将存储在单独的行中，由LocationID唯一标识。每个地方的经度和纬度将分别存储在两个不同的列中，为了执行快速搜索，我们应该在这两个字段上创建索引。

要查找半径“D”内给定位置（X，Y）的所有附近位置，我们可以这样查询：

```
Select * from Places where Latitude between X-D and X+D and Longitude between Y-D and Y+D
```

上面的查询并不完全准确，因为我们知道要找到两点之间的距离，我们必须使用距离公式（毕达哥拉斯定理），但为了简单起见，让我们采用这个公式。

此查询的效率如何？我们估计有5亿个仓库。因为我们有两个独立的索引，每个索引都可以返回一个巨大的位置列表，并且在这两个列表上执行交集是没有效率的。另一种看待这个问题的方法是，“X-D”和“X+D”之间可能有太多的位置，“Y-D”和“Y+D”之间也可能有太多的位置。

如果我们能够以某种方式缩短这些列表，我们便可以提高查询的性能。

## 2. Grid Solution

我们可以把整个地图分成更小的网格，从而把地点分成更小的集合。每个网格将存储位于经纬度特定范围内的所有位置。这个方案可以让我们只查询几个网格来找到附近的位置。根据给定的位置和半径，我们可以找到所有相邻的网格，然后通过查询这些网格来查找附近的位置。

具体查询方法如下：

```
Select * from Places where Latitude between X D and X D and Longitude between Y-D and Y+D and GridID in (GridID, GridID1, GridID2, ..., GridID
```

有待思考的问题：

1. 合理的网格尺寸是多少？
2. 如果将索引放在内存中，需要多大的内存呢？（假设业务是覆盖全球的）

## 3. Dynamic size grids Solution

假设我们不希望一个网格中超过500个位置，因为这样我们就可以更快地搜索。所以，每当一个网格达到这个极限，我们就把它分成四个大小相等的网格，并在其中分配位置。这意味着像旧金山市中心这样人口稠密的地区将有很多网格。而像太平洋这样人口稀少的地区将有大型网格，这些网格沿海岸线分布。

抽象来看，即什么数据类型可以支持不同粒度的网格？

```
什么样的数据结构可以保存这些信息？每个节点有四个子节点的树可以满足我们的需要。每个节点将表示一个网格，并包含该网格中所有位置。如果一个节点达到了500个位置的限制，我们将对其进行分解，在其下创建四个子节点并在其中分配位置。这样，所有叶节点将表示无法进一步分解的网格。因此叶节点将保留一个位置列表。这种每个节点可以有四个子节点的树结构称为四叉树
```

[四叉树wiki](https://baike.baidu.com/item/%E5%9B%9B%E5%8F%89%E6%A0%91/8557650), 由 拉斐尔·芬科尔(Raphael Finkel) 与 J. L. Bentley 在1974年发展出来。


有待思考的问题：

* 如何构造一颗四叉树？
* 我们如何找到给定位置的网格
* 我们如何找到给定网格的相邻网格
* 检索流程
* 存储四叉树需要多少内存
* 如何插入一个新位置

## Data Partitioning [Evolve]

假设我们有大量的位置信息，以至于我们的索引不能放入一台机器的内存中，那会怎么样？随着每年20%的增长，我们终将达到服务器的内存限制。另外，如果一台服务器不能提供所需的读流量怎么办（即这台服务器上的服务挂了或服务器自身挂了）？为了解决这些问题，我们必须划分我们的四叉树！

### 基于region做sharding

### 基于位置做sharding

## 副本和容错 [Evolve - HA]

## Load Balancing (负载均衡) [Evolve - HA]

## Ranking

参见Top-K问题

## GeoHash

地理位置的索引，除quadtree方案外，geohash也是不错的方案。

参见美团文章：[Solr空间搜索原理分析与实践](https://tech.meituan.com/2014/09/02/solr-spatial-search.html)
