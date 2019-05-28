# CNN笔记

## 网络架构

输入层->券积层->激活层->池化层->全链接层->输出层

## 卷积层

卷积层由多个filter构成。filter的移动步长为stride，其depth定义为：filter的个数。

### padding和stride

定义填充和步幅如下：
`$padding=P$`,`$stride=S$`, `$filter=F$`

定义filter个数为`$K$`

输入为`$input=(W, H)$`

则，输出为

`$([(W - F + 2P)/S] + 1) * ([(H - F + 2P)/S] + 1) * K$`

但[tensorflow计算公式](https://www.tensorflow.org/api_guides/python/nn#Convolution)略有差别：

In summary TensorFlow uses the following equation for 'SAME' vs 'VALID'

SAME Padding, the output height and width are computed as:

out_height = ceil(float(in_height) / float(strides[1]))

out_width = ceil(float(in_width) / float(strides[2]))

VALID Padding, the output height and width are computed as:

out_height = ceil(float(in_height - filter_height + 1) / float(strides[1]))

out_width = ceil(float(in_width - filter_width + 1) / float(strides[2]))

### Tensorflow卷积层示例

Tensorflow提供[tf.nn.conv2d()](https://www.tensorflow.org/api_docs/python/tf/nn/conv2d)和[tf.nn.bias_add()](https://www.tensorflow.org/api_docs/python/tf/nn/bias_add)两个方法支持创建卷积层。

