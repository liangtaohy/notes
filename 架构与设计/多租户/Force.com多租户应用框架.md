47000+组织
metadata-driven arch
平台信任度：deliver robust, reliable, scale, secure, fast

# 介绍

技术的不断进步和商业模式的变化在软件应用程序的设计、构建和交付给最终用户的方式上产生了重大的范式转变。如今，可靠的宽带互联网接入、面向服务的体系结构（SOA）以及管理专用内部应用程序的成本低效率正在推动向交付可分解、可管理、共享、基于Web的服务（称为软件即服务（SaaS））的过渡。托管应用程序平台是专门为满足构建SaaS应用程序的独特挑战而设计的托管环境，并提供比以往任何时候都更经济高效的应用程序。

本文的重点是多租户，这是一种基本的设计方法，可以极大地帮助提高SaaS应用程序的可管理性。

# Force.com架构总览
关键点：实体及关系统一使用metadata表示；metadata access是瓶颈，通过cache方式优化；外部索引提供查询服务；多租户感知的查询优化器

Force.com优化过的metadata-driven架构为按需、多租户应用程序提供了卓越的性能、可伸缩性和定制（图3）。

所有东西在内部均被表示为Metadata，如表单、报告、工作流、用户权限、租户个性化的业务逻辑，甚至系统所依赖的data table(数据表)和indexes(索引)。

因为元数据是一个关键因素，平台的运行时引擎必须优化对元数据的访问；否则，频繁的元数据访问将严重影响平台的扩展能力。考虑到这个潜在的瓶颈，Force.com网站使用元数据缓存来维护内存中最近使用的元数据，避免因磁盘I/O和代码重新编译带来的性能损耗，并提高应用程序响应时间。

为了优化对系统大表中数据的访问，Force.com网站的引擎依赖于一组专门的透视表，这些表用于维护反范式的数据。这些数据通常用于索引、唯一性、关系等。

Force.com网站的数据处理引擎通过透明地批量执行数据修改操作，帮助简化大型数据负载和在线事务处理应用程序的开销。引擎具有内置的故障恢复机制，可以在分析导致错误的记录后自动重试大容量存储操作。

为了进一步提高应用程序的响应时间，该平台采用了一个优化全文索引和搜索的外部搜索服务。当应用程序更新数据时，搜索服务的后台进程以近乎实时的方式异步更新租户和用户特定的索引。应用程序引擎和搜索服务之间的职责分离使平台应用程序能够高效地处理事务，而无需文本索引更新的开销，同时快速为用户提供准确的搜索结果。

作为Force.com网站的运行时应用程序生成器根据特定用户请求动态构建应用程序，该引擎在很大程度上依赖其“多租户感知”查询优化器来尽可能高效地执行内部操作。查询优化器考虑哪个用户正在执行给定的应用程序函数，然后使用UDD中维护的相关租户特定元数据以及内部系统透视表，将数据访问操作构建和执行为优化的数据库查询。

# 数据定义和存储

不同于试图为每个应用程序和租户管理一组巨大的、不断变化的实际数据库结构，Force.com网站存储模型使用一组元数据metadata、数据data和透视表pivot table管理“虚拟”数据库结构，如图4所示。

当组织创建自定义应用程序对象（即自定义表）时，UDD会跟踪有关对象、它们的字段、关系和其他对象定义特征的元数据。同时，一些大型数据库表存储所有虚拟表的结构化和非结构化数据，一组相关的、专门的透视表维护非规范化数据，使组合的数据集功能强大。

## 对象元数据表 Object Table
Objects元数据表存储有关组织为应用程序定义的自定义对象（也称为表或实体）的信息，包括对象的唯一标识符（ObjID）、拥有对象的组织（OrgID）以及对象的名称（ObjName）。

## 字段元数据表Field Table
字段元数据表存储有关组织为自定义对象定义的自定义字段（也称为列或属性）的信息，包括字段的唯一标识符（FieldID）、拥有包含对象的组织（OrgID）、包含字段的对象（ObjID）、字段的名称（FieldName），字段的数据类型、指示字段是否需要索引的布尔值（IsIndexed）以及字段在对象中相对于其他字段的位置（FieldNum）。

## Data Table
数据表存储应用程序可访问的数据，这些数据映射到所有自定义对象及其字段（由对象和字段中的元数据定义）。每行包含标识字段，例如全局唯一标识符（GUID）、拥有该行的组织（OrgID）和包含对象标识符（ObjID）。数据表中的每一行也有一个名称字段，该字段存储相应对象实例的“自然名称”；例如，Account对象可能使用“Account Name”，Case对象可能使用“Case Number”，等等。Value0。。。Value500列存储分别映射到objects和fields表中声明的对象和字段的应用程序数据；所有“flex”列都使用可变长度的字符串数据类型，以便它们可以存储任何结构化类型的应用程序数据（字符串、数字、日期等）。

自定义字段可以使用许多标准的数据类型（如文本、数字、日期和日期/时间）中的任何一种，也可以使用特殊用途的结构化数据类型，如picklist（枚举字段）、autonumber（自动递增、系统生成的序列号）、formula（只读派生值）、master detail relationship（外键）、checkbox（布尔值）、电子邮件、URL和其他。自定义字段也可以是必需的（非空），并具有自定义验证规则（例如，一个字段必须大于另一个字段），这两个规则都由平台的应用服务器强制执行。

当组织声明或修改自定义应用程序对象时，Force.com网站管理Objects表中定义对象的元数据行。同样，对于每个自定义字段，Force.com网站管理字段表中的行，包括将字段映射到数据表中特定flex列的元数据，以存储相应的字段数据。因为Force.com网站将对象和字段定义管理为元数据而不是实际的数据库结构，平台可以容忍多租户应用程序模式维护活动，而不会阻止其他租户和用户的并发活动。

尽管图5中没有显示，但数据表还包含其他列。例如，有四列用于管理审核数据，包括创建对象实例（行）的时间和用户，以及上次修改对象实例的时间和用户。数据表还包含一个IsDeleted列，该列强制。com用于指示对象实例何时被删除。

## Index Pivot Table
传统的数据库系统依赖索引来快速定位数据库表中具有与特定条件匹配的字段的特定行。但是，为数据表的flex列创建本机数据库索引是不实际的，因为Force.com很可能使用一个flex列来存储具有不同结构化数据类型的许多字段的数据。相反，Force.com管理数据表的索引的方法是将标记为索引的字段数据同步复制到名为Indexes的透视表中的适当列，如简化的ER图所示（图7）。

Indexes表包含强类型的索引列，如StringValue、NumValue和DateValueForce.com网站用于定位相应数据类型的字段数据。例如，Force.com网站将数据表flex列中的字符串值复制到索引中的StringValue字段，将日期值复制到DateValue字段，等等。索引表的底层索引是标准的非唯一数据库索引。当内部系统查询包含引用自定义对象中结构化字段的搜索参数时，平台的查询优化器将使用索引表来帮助优化关联的数据访问操作。

## FallbackIndex Table

## NameDenorm Table

## History Tracking Table
Force.com轻松为任何领域提供完整并可立即使用的(turnkey)历史跟踪。当组织对特定字段启用审核时，系统将使用内部透视表进行审核跟踪，异步记录有关对字段所做的更改（旧值和新值、更改日期等）。
## 数据和元数据分区(Patitioning)

Force.com的全部数据、元数据和pivot表结构（包括底层数据库索引）使用所依赖的数据库分区机制进行分区管理。Force.com使用OrgID（按租户）进行物理分区。数据分区是数据库系统提供的一种经过验证的技术，可以将大型逻辑数据结构物理地划分为更小、更易于管理的部分。分区还可以帮助提高大型数据库系统（如多租户环境）的性能、可伸缩性和可用性。例如，根据定义，每一个Force.com应用程序查询以特定租户的信息为目标，因此查询优化器只需考虑访问包含租户数据的数据分区，而不需要考虑访问整个表或索引。这种常见的优化有时被称为“分区修剪”。

# 应用程序开发、逻辑和处理
Force.com支持两种不同的方法来创建自定义应用程序及其各自的组件：一种是声明式的，使用本机平台应用程序框架；另一种以编程方式使用应用程序编程接口（API）的方法。下面的部分将详细介绍每种方法和相关的应用程序开发主题。

## 应用程序框架

开发人员可以基于应用程序框架声明性地构建自定义的应用程序。该平台的本地点击式界面支持应用程序开发过程的所有方面，包括创建应用程序的数据模型（自定义对象及其字段、关系等）、安全和共享模型（用户、组织层次结构、配置文件等）、用户界面（屏幕布局、数据输入表单、，报告等），以及逻辑和工作流程。

应用程序框架用户界面很容易构建，因为不涉及编码。在后台，它们支持所有常见的数据访问操作，包括查询、插入、更新和删除。本机平台应用程序执行的每个数据操作可以一次修改一个对象，并在单独的事务中自动提交每个更改。

同时，Force.com提供整合的IDE环境。IDE环境提供了许多内置的平台特性。所以，它可以非常容易的实现一个通用的应用功能。这些特性包括声明性工作流、加密/屏蔽字段、验证规则、公式字段、汇总字段和跨对象验证规则。

工作流是由插入或更新对象实例（行）触发的预定义操作。工作流可以触发任务、电子邮件警报、更新数据字段或发送消息。

工作流规则用于指定何时触发一个动作的条件。可以将工作流设置为立即启动，也可以设置为在触发事件后的后续间隔中运行。例如，开发人员可能会声明一个工作流，在更新记录后，该工作流会自动将行的状态字段更新为“已修改”，然后向主管发送模板电子邮件警报。所有工作流操作都发生在触发工作流的事务的上下文中。如果系统回滚事务，则执行的所有相关工作流操作也将回滚。

当为包含敏感数据的对象定义字段时，开发人员可以轻松地配置该字段使用Force.com的加密组件加密数据，并可选地使用输入掩码来隐藏屏幕信息，以防窥探。Force.com使用AES（高级加密标准）算法加密字段128位密钥。

声明性验证规则是组织在不进行任何编程的情况下实施域完整性规则的一种简单方法。例如，图9中的第一个屏幕截图说明了使用Force.com IDE声明一个验证规则，该规则确保LineItem对象的Quantity字段始终大于零。

公式字段是Force.com应用框架的另一个声明性的特性。这个特性使向对象添加计算字段变得容易。例如，图9中的第二个屏幕截图还显示了开发人员如何使用一个简单的IDE表单向LineItem对象添加字段以计算LineTotal值。

一个roll-up(上卷)汇总字段是一个跨对象字段，它便于在父对象中聚合子字段信息。例如，图9中的最后一个屏幕截图显示了如何使用IDE在SalesOrder对象中基于LineItem对象的LineTotal字段创建OrderTotal摘要字段。

## 元数据和Web服务API

(该节值得关注 bulk部分)

Force.com还提供用于构建应用程序的编程api。这些API与基于SOAP的开发环境兼容，包括VisualStudio.NET（C++）和Apache轴（java和C++）。

应用程序可以利用Force.com的API与其他环境集成。例如，应用程序可以利用api访问其他系统中的数据，构建合并来自多个数据源的信息的mashup，将外部系统作为应用程序进程的一部分，或者构建胖客户端来与Force.com平台数据库管理系统交互。

## Apex