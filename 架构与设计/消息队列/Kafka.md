# Kafka的架构和设计原则

由于现有系统的局限性，我们开发了一种新的基于消息传递的日志聚合器Kafka。首先介绍下kafka的基本概念。
* topic:特定类型的消息流
* 生产者可以将消息发布到topic。然后，发布的消息存储在一组称为broker的服务器上。
* 消费者可以从broker订阅一个或多个topic，并通过从broker中拉取数据(pull模式)来消费订阅的消息。

消息传递在概念上很简单，我们试图让kafka api同样简单地反映这一点。我们通过伪代码来看下api是如何被使用的。消息特指只包含有效的payload。用户可以选择最喜欢的序列化方法来编码消息。为了提高效率，生产者可以在一个发布请求中发送一组消息。


生产者示例代码：

```
Sample producer code:
producer = new Producer(…);
message = new Message(“test message str”.getBytes());
set = new MessageSet(message);
producer.send(“topic1”
```

要订阅主题，消费者首先为该topic创建一个或多个消息流。发布到topic的消息将均匀分布到这些子流中。关于kafka如何分发消息的细节将在后面的3.2节中描述。每个消息流在所生成的连续消息流上提供一个迭代器接口。然后，消费者将迭代流中的每个消息，并处理消息的payload。与传统迭代器不同，消息流迭代器从不终止。如果当前没有更多的消息要使用，迭代器将阻塞，直到新消息发布到topic。我们既支持点到点交付模型(p2p deliver model)（在该模型中，多个消费者共同使用一个topic中所有消息的一个副本），也支持发布/订阅模型，其中多个消费者各自检索其自己的topic副本。

消费者示例代码：

```
streams[] = Consumer.createMessageStreams(“topic1”, 1)
for (message : streams[0]) {
	bytes = message.payload(); // do something with the bytes
}
```

kafka的整体架构如图1所示。由于Kafka是分布式的，因此Kafka集群通常由多个broker组成。为了平衡负载，将一个topic分成多个分区（partition），每个broker存储一个或多个分区（partition）。多个生产者和消费者可以同时发布和拉取消息。在第3.1节中，我们将描述broker上单个分区的布局，以及我们选择的几个设计原则，以使访问分区时更高效。在第3.2节中，我们将描述生产者和消费者如何在分布式环境中与多个broker交互。我们在第3.3节讨论了kafka的投递保证(delivery guarantees)。

图1（架构图）

## 3.1单个分区的效率

我们在kafka做了一些决策来提高系统的效率。

### 存储简单

kafka有一个非常简单的存储布局。topic的每个分区对应一个逻辑日志。在物理上，日志被实现为一组大小大致相同的段文件(segment file)（例如，1GB大小）。每次生产者将消息发布到分区时，broker只需将消息附加到最后一个段文件中。为了获得更好的性能，我们只在发布了可配置数量的消息或经过一定时间窗口后才将段文件落盘。消息只有在落盘后才能被消费者消费。

与典型的消息系统不同，存储在Kafka中的消息没有显式的消息id，而是通过日志中的逻辑偏移量(file offset)对每个消息进行寻址。这避免了维护索引结构的开销。通常，索引是寻道密集的且随机访问的。请注意，我们的消息id是递增的，但不是连续的。为了计算下一条消息的id，我们必须在当前id的基础上加上当前消息的长度。简化期间，后文不再区分消息id和offset。

一个消费者总是按顺序消费来自指定topic的消息。如果该消费者确认(ack offset)了某个特定的消息偏移量，则表示消费者已接收到指定分区中该偏移量(offset)之前的所有消息。在后台，消费者正是通过向broker发送异步pull请求的方式，为应用程序准备一个可用的数据缓冲区。每个pull请求包含一个偏移量和可接受的字节数。每个broker在内存中保存一个排好序的偏移量列表，其中也包括每个段文件中第一条消息的偏移量。代理通过搜索偏移量列表找到请求消息所在的段文件，并将数据发送回消费者。消费者收到消息后，需要计算下一条要使用的消息的偏移量，并在下一个pull请求中使用该偏移量。Kafka日志和内存索引的布局如图2所示。每个框显示消息的偏移量。

### 高效传输

我们对进出kafka的数据非常小心。前面，我们展示了生产者可以在一个发送请求中提交一组消息。尽管最终消费者API一次迭代一条消息，但消费者的每个pull请求也会检索多条消息，这些消息的大小通常为数百KB。

我们所做的另一个非常规选择是避免在Kafka层显式缓存消息。相反，我们依赖于底层文件系统页面缓存。这样做的主要好处是避免了双重缓冲——消息只缓存在页面缓存中。这还有一个额外的好处，即: 即使在broker进程重新启动时也能保留热缓存(file system做的，和进程无关)。由于Kafka根本不缓存进程中的消息，所以它在垃圾回收内存方面的开销非常小，这使得在基于VM的语言中高效实现成为可能。

最后，由于生产者和消费者都是顺序访问段文件的，而消费者往往落后于生产者一小部分，所以正常的操作系统缓存试探法是非常有效的（特别是直写式缓存和预读）。我们发现，生产和消费都具有一致的性能，与数据大小成线性关系，即使数据高达数TB。

此外，我们优化了消费者的网络访问。Kafka是一个多用户系统，单个消息可能被不同的消费应用程序多次消费。从本地文件向远程套接字发送字节的典型方法包括以下步骤：（1）在操作系统中将数据从存储媒介读取到页缓存；（2）将页缓存中的数据复制到应用程序缓冲区；（3）将应用程序缓冲区复制到另一个内核缓冲区；（4）将内核缓冲区发送到套接字。这包括4个数据复制和2个系统调用。在Linux和其他Unix操作系统上，存在一个sendfile api[5]，它可以直接将字节从文件通道传输到套接字通道。这避免了步骤（2）和（3）中引入的2个副本和1个系统调用。Kafka利用sendfile api高效地将日志段文件中的字节从broker传递给消费者。

### 无状态代理

与大多数其他消息传递系统不同，在Kafka中，关于每个消费者消费了多少的消息不是由broker维护的，而是由消费者自己维护的。这样的设计大大降低了broker的复杂性和开销。然而，这使得删除消息变得很困难，因为broker不知道是否所有订阅者都消费了该消息。在保留策略上，Kafka使用一个简单的基于时间的SLA来解决这个问题。如果消息在broker中的保留时间超过某个时间段（通常为7天），则会自动删除该消息。这个解决方案在实践中效果很好。大多数消费者，包括线下消费者，每天、每小时或实时完成消费。Kafka的性能不会随着数据量的增大而降低，这使得这种长时间保留成为可能。

这种设计有一个重要附带收益。使用者可以倒回旧的偏移量然后重新消息数据。这违反了队列的常见约定，但事实证明这是多消费者场景下的一个必备特性。例如，当消费者中的应用程序逻辑出现错误时，应用程序可以在错误修复后重新放某些消息。这对于ETL数据加载到我们的数据仓库或Hadoop系统中特别重要。作为另一个例子，所消耗的数据只能周期性地刷新到持久性存储（例如，全文索引器）。如果消费者崩溃，未刷新的数据将丢失。在这种情况下，消费者可以检查未刷新消息的最小偏移量，并在重新启动时重新使用该偏移量。我们注意到，在pull模型中支持重放比push模型更容易。


## 3.2 Distributed Coordination (不知怎么翻译。。。百度翻译为分布式协调)

现在我们来描述生产者和消费者在分布式环境中如何工作。每个生产者可以将消息发布到随机选择的分区或由分区键和分区函数在语义上确定的分区。我们将关注消费者如何与broker互动。

kafka有消费组的概念。每个消费者组由一个或多个消费者组成，这些消费者共同消费一组订阅的topic，也就是说，每条消息只传递给组内的一个消费者。不同的消费组各自独立地消费订阅的消息，消费组之间不需要进行协调。一个消费组内的消费者可以在不同的进程中，也可以在不同的机器上。我们的目标是将存储在broker中的消息平均分配给消费者，而不引入太多的协调开销。

我们的第一个决策是将topic中的分区作为可并行的最小单元。这意味着在任何给定的时间，来自一个分区的所有消息只由每个消费者组中的一个消费者使用。如果我们允许多个消费者同时使用一个分区，他们就必须协调谁使用哪些消息，这就需要锁定和状态维护开销。相反，在我们的设计中，协调只发生在消费者们rebalance负载时，load rebalance是一个非常罕见的事件。为了真正均衡负载，我们要求一个topic中的分区要远大于每个消费组中的消费者数量。通过对topic进行超量分区，我们可以很容易地实现这一点。

我们所做的第二个决策是不设“主”节点（master node），而是让消费者以去中心化的方式相互协调。添加主节点会使系统复杂化，因为我们必须进一步处理主节点的故障。为了便于协调，我们采用了一个高度可用的共识服务Zookeeper[10]。Zookeeper有一个非常简单的文件系统API。可以创建路径、设置路径值、读取路径值、删除路径以及列出路径的子级。它做了一些更有趣的事情：（a）可以在一个路径上注册一个观察者，并在一个路径的子路径或路径的值发生变化时得到通知；（b）路径可以是临时的（与持久相反），这意味着，如果创建客户端不在，该路径将被Zookeeper服务器自动删除；（c）Zookeeper将其数据复制到多个服务器，这使得数据高度可靠和可用。

Kafka使用Zookeeper执行以下任务：（1）检测broker和消费者的添加和删除；（2）当上述事件发生时，在每个消费者中触发一个重新平衡过程；（3）维护消费关系并跟踪每个分区的偏移量。具体地说，当每个broker或消费者启动时，它将其信息存储在Zookeeper中的broker或消费者注册表中。broker注册表包含broker的主机名和端口，以及存储在其中的一组topic和分区。消费者注册中心包括消费者所属的消费组及其订阅的topic集。每个消费组都与Zookeeper中的所有权注册表和偏移注册表相关联。所有权注册中心对每个订阅的分区都有一个路径，路径值是当前从该分区消费的消费者的id（我们使用消费者拥有这个分区的术语）。偏移量注册表为每个订阅的分区存储分区中最后一次使用的消息的偏移量。

为broker注册表、消费者注册表和ownership注册表创建的paths是短暂的，而offset注册表是持久的。如果broker发生故障，它的所有分区信息都会自动从broker注册表中删除。消费者发生故障时，消费者注册表以及它在ownership注册表中拥有的所有分区都会被删除。每个消费者都在broker注册表和消费者注册表上注册一个Zookeeper观察者(watcher)，通过这些watcher，在broker或消费者组发生变更时，消费者会收到通知。

在使用者的初始启动期间，或者当使用者通过观察者收到有关代理/使用者更改的通知时，使用者将启动一个重新平衡过程，以确定它应该从中使用的新分区子集。该过程在算法1中描述。通过从Zookeeper读取代理和消费者注册表，消费者首先计算每个订阅主题T可用的分区集（PT）和订阅T的消费者集（CT），然后将分区PT划分为| CT |块，并确定地选择一个块来拥有。对于使用者选择的每个分区，它将自己作为分区的新所有者写入所有权注册表。最后，使用者开始一个线程，从存储在offset注册表中的偏移量开始，从每个拥有的分区中提取数据。作为从注册表中获取的最新消息的偏移量，定期从使用者分区中获取更新。

## 3.3 Delivery Guarantees(投递承诺)

一般来说，kafka只保证至少一次投递(at-least-once)。exactly-once通常需要两阶段提交(2PC)，对于我们的应用程序来说不是必需的。大多数情况下，一条消息只向每个消费组传递一次。但是，如果消费者进程在没有clean close的情况下崩溃，接管失败消费者拥有的那些分区的消费者进程可能会收到一些重复的消息，这些消息是在成功提交给zookeeper的最后一个偏移量之后。如果应用程序关心重复，它必须添加自己去重逻辑，要么使用返回给消费者的偏移量，要么使用消息中的某个唯一键。这通常比使用两阶段提交更具成本效益(cost-effective)。

Kafka保证将来自单个分区的消息按顺序传递给消费者。但是，不保证来自不同分区的消息的顺序。

为了避免日志损坏，Kafka为日志中的每个消息存储一个CRC。如果代理上有任何I/O错误，Kafka会运行一个恢复过程来删除那些CRC不一致的消息。在消息级别具有CRC还允许我们在消息生成或使用之后检查网络错误。

如果broker失败，则存储在其上的任何尚未使用的消息都将不可用。如果broker上的存储系统永久损坏，任何未使用的消息都将永久丢失。将来，我们计划在Kafka中添加内置复制，以便在多个代理上冗余地存储每条消息。