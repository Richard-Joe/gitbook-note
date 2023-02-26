# 哪懂什么算法，全是靠刷

## 离线查询

在线查询：请求来一个，立即处理并回答一个。

离线查询：把所有请求收集起来，统一处理，统一回答。

## 离散化思想

在不改变数据相对大小的条件下，对数据进行相应的缩小。

## 树状数组

## 最大化最小值、最小化最大值

二分答案

[2517. 礼盒的最大甜蜜度](https://leetcode.cn/problems/maximum-tastiness-of-candy-basket/)

## 正难则反（逆向思维）

`正向求解 = 总数 - 反向求解`

## 贡献思维

`求所有方案数` 转化为 `求每个元素的贡献值的总和`

[2475. 数组中不等三元组的数目](https://leetcode.cn/problems/number-of-unequal-triplets-in-array/description/)

## 分解质因数

```python
while i * i <= n:
	while n % i == 0:
    	# print(i)
		n //= i
	i += 1
```

## 差分数组

1. 多次操作：一个区间内的数都加1
2. 操作完了，需要看一下每个数变成多少了

```c
# 初始数组
[0, 0, 0, 0, 0]

# 区间[1,3]都加上1，把开始的地方+1，在结束的地方-1
[0, 1, 0, 0, -1]

# 区间[0, 1]都加上1，把开始的地方+1，在结束的地方-1
[1, 1, -1, 0, -1]

# 结果，求前缀和
[1, 2, 1, 1, 0]
```

## 二分查找

```python
# 返回有序数组中第一个 >= target的位置。如果都小于target，返回数组长度。
def lower_bound(nums: List[int], target: int) -> int:
    # 闭区间 [left, right]
    left = 0
    right = len(nums) - 1

    while left <= right:
        mid = left + (right - left) // 2
        if nums[mid] < target:
            left = mid + 1
        else:
            right = mid - 1
    return left

# 三种区间
# [left, right]
# [left, right)
# (left, right)
# 上面代码稍微改动即可

def lower_bound2(nums: List[int], target: int) -> int:
    # 左闭右开 [left, right)
    left = 0
    right = len(nums)

    while left < right:
        mid = left + (right - left) // 2
        if nums[mid] < target:
            left = mid + 1
        else:
            right = mid
    return left  # 返回right也可以

def lower_bound3(nums: List[int], target: int) -> int:
    # 开区间 (left, right)
    left = -1
    right = len(nums)

    while left + 1 < right:
        mid = left + (right - left) // 2
        if nums[mid] < target:
            left = mid
        else:
            right = mid
    return right

# 四种情况：
# >= x      获取第一个 >= x的位置
# > x       可以转换成 >= (x+1)
# < x       可以转换成 (>= x) - 1；表示第一个>=x位置的左边位置
# <= x      可以转换成 (> x) - 1；表示第一个>x位置的左边位置；>x还可以继续转换

# 举例：
# 获取有序数组中目标值target的第一个位置和最后一个位置（假设存在target）
start = lower_bound(nums, target)        # >= x
end = lower_bound(nums, target+1) - 1    # <= x
```

## 01背包

```python
dp = [0]*(n+1)
for i in range(n):
	for j in range(target, nums[i]-1, -1):
		# 求最大价值
		dp[j] = max(dp[j], dp[j-weight[i]]+value[i])
		# 求有多少种方法
		# dp[j] += dp[j-nums[i]]
```

## 完全背包

唯一的区别就是背包的遍历顺序：01背包是要求倒序，完全背包是正序

```python
for n in nums:  # 遍历物品
	for j in range(n, target+1):  # 遍历背包
		# 求最大价值
		# 求有多少种方法
```

### 先遍历物品

由于先遍历的物品，会得到物品的**组合**。

比如物品[1,2,3]，背包容量为3。先遍历1，后遍历2，得到组合[1,2]。

```python
for n in nums:  # 遍历物品
	for j in range(c, target+1):  # 遍历背包
```

### 先遍历背包

由于先遍历的背包，会得到物品的**排列**。

比如物品[1,2,3]，背包容量为3。既能得到[1,2]，又能得到[2,1]。

```python
for j in range(0, target+1):  # 遍历背包
	for n in nums:  # 遍历物品
```

### 判断一个数是否是2的幂次方

```python
x & (x-1) == 0
```

### 取模

取模过程只能乘、加、减，而除法取模只能使用逆元进行操作。

```python
(a + b) % MOD == a % MOD + b % MOD
```

[6368. 找出字符串的可整除数组](https://leetcode.cn/problems/find-the-divisibility-array-of-a-string/)
