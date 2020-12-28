# 需求与目标 [Scenario-场景和功能]

让我们构建一个简单一点的Uber。

在系统中，我们有两类用户：1）司机 2）客户

* 司机需要定期向服务通知他们的位置和接单状态;
* 乘客可以看到附近所有可接单的司机;
* 客户可以发出一个行程请求；附近的司机会收到客户的请求通知;
* 一旦司机和顾客接受了一次乘车，他们可以实时地看到对方的当前位置，直到行程结束;
* 到达目的地后，司机确认行程完成，可以开始接下一单了

# 容量预估与约束 [Necessary-约束]

* 假设我们有3亿客户和100万司机;
* 每天有100万活跃客户和50万活跃司机;
* 我们假设每天有100万次乘车;
* 假设所有活跃的司机每三秒钟通知一次他们的当前位置;
* 一旦客户提出乘车需求，系统应能实时联系司机

# 系统设计与算法


disco shard
S2 Geometry Library
Ringpop使用一致性哈希环
为了能够跨节点分布状态，为了管理集群成员和失败侦测，Ringpop使用SWIM，这是一个可扩展的弱一致性
gossip协议，Ringpop使用TChannel作为其RPC协议
Twiiter Finagle的多路复用RPC协议Mux启发


Uber用的两个技术叫GoldETA和Gurafu. GoldETA是根据history, 在大城市比较准确，但是在新的城市有cold start问题。Gurafu是现在在用的东西，把graph分成Node和Edge，Edge有weight, 并且dynamically update the weight. 这样就可以实时计算得到ETA。另外我觉得这边应用Dijkstra就没问题，因为SPFA是考虑weight < 0, 在这里可能不需要。