
# Design of libpmp

## Part I: Isolation request

要求将一个物理地址区间$[start, end]$， 权限设为$X$。
用一个单向链表存储这个请求。
链表：
```c
struct LinkedList
{
    /**
     * start:32位无符号数
     * end:32位无符号数
     * X:pmpxcfg里怎么存这里就怎么存
     * next:指针
     * flag:存这个请求在不在PMP entry里面
     * priority:默认有10,0000个PMP entry,数越小，权限越高
     * */
};
```
每次来一个请求，就存到链表中。


## Part II: Address access

要访问地址$A$。
遍历链表中的所有节点，找到一个优先级最高，$[start_i, end_i]$包含 $A$的一个节点$i$，假设它的优先级是$x$。现在要把这个节点存储的地址范围放到PMP entry中。
* 如果PMP entry没满，排序找到这个节点的位置，然后放到合适的PMP entry。
* 如果PMP entry满了，LRU踢掉一个（怎么实现参考Leetcode 146），然后排序找到这个节点的位置，然后放到PMP entry中。

TODO: 遍历链表的方式是baseline，需要做更高效率的优化。

TODO: PMP entry本身编码方式比较奇怪，做一层16个区间的抽象，然后再抽象为100个。

## Part III: Free PMP entry
假设有一个PMP entry $x$用户不想用了，在链表中把priority是$x$的节点删除，如果它还在PMP entry中，就把这个PMP entry清空。

## Part IV: Refresh

给定8个地址区间的隔离请求$[s_i, e_i]$，对应的权限是$X_i$，要求正确地把这个要求对应到底层的PMP entry中。
首先把cache中的请求按照优先级排序，排完序后，按照如下顺序排放，其中如果$i < j, i, j \in [0, 7]$，那么优先级$[s_i, e_i]$比$[s_j, e_j]$高。

![refresh](https://i.imgur.com/pzB9fNA.png)

## Part V: Interrupt or Exception
用户在S态跑代码，访问地址时，触发中断/异常，处理程序跳转到我们的库。


