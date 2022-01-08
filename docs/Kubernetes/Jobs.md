# Jobs

`Job` 和 `Replication Controllers`是彼此互补的。RC管理的是那些不希望被终止的Pod， Job 管理的是那些希望被终止的 Pod。

## 非并行 Job

示例：`job.yaml`
```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: pi
spec:
  template:
    spec:
      containers:
      - name: pi
        image: perl
        command: ["perl",  "-Mbignum=bpi", "-wle", "print bpi(2000)"]
      restartPolicy: Never		# 重启策略（对应的是Pod，不是Job）
  backoffLimit: 4				# 重试次数限制
```

```shell
# 创建Job
kubectl apply -f job.yaml

# 查看Job的状态
kubectl describe jobs/pi

# 查看Job对应的Pods
kubectl get pods --selector=job-name=pi

# 查看Pod的输出
kubectl logs pi-s4qzr
```

编写规范：
- Job 中 Pod 的 `restartPolicy` 只能设置为`Never`或`OnFailure`之一。
- Job 在运行的时候不可能去无限的重试， `backoffLimit` 就是来保证一个 Job 最多能重试多少次。

## 并行 Job
示例：`paral.yaml`
```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: paral
spec:
  template:
    spec:
      containers:
      - name: paral
        image: ubuntu
        command: ["/bin/sh"]
        args: ["-c", "sleep 10; date"]
      restartPolicy: OnFailure
  completions: 8		# Pod队列执行次数
  parallelism: 2		# 并行执行的个数
```

```shell
# 创建并行Job
kubectl apply -f paral.yaml

# 观察Pod运行状况
watch kubectl get pods --selector=job-name=paral
```
对于非并行Job，可以不设置`completions`和`parallelism`，这两个属性不设置时，均取默认值为1。

## 完成模式

`completionMode`指定了完成模式：
- `NonIndexed`（默认值）：成功完成的 Pod 个数达到 completions 所设值时认为 Job 已经完成。
- `Indexed`：Job 的 Pod 会获得对应的完成索引，取值为 0 到 completions-1。当每个索引都对应一个完成完成的 Pod 时，Job 被认为是已完成的。

## backoffLimit 策略

背景：希望 Job 在经历若干次重试之后直接进入失败状态。

`backoffLimit`：Job 失败之前的重试次数。默认值为6。

重试时间将会按指数增长 （从 10 秒、20 秒到 40 秒）最多至 6 分钟。

当 Job 的 Pod 被删除时，或者 Pod 成功时没有其它 Pod 处于失败状态，失效的次数也会被重置（为 0）。

## Job 终止与清理

可以使用`kubectl`来删除Job。

终止Job的另一种方式是设置一个活跃期限`activeDeadlineSeconds`。

一旦 Job 运行时间达到 activeDeadlineSeconds 秒，其所有运行中的 Pod 都会被终止，并且 Job 的状态更新为 type: Failed 及 reason: DeadlineExceeded。

注意： **Job 的 activeDeadlineSeconds 优先级高于其 backoffLimit 设置。**

通过设置 Job 的 `ttlSecondsAfterFinished` 字段，可以让TTL控制器清理掉 已结束的资源（状态为`Complete` 或 `Failed`）。

## 挂起 Job

要挂起一个 Job，可以将 Job 的 `suspend` 字段更新为 true。
当恢复其执行时，可以将 Job 的 `suspend` 字段更新为 false。

创建一个 `suspend` 被设置为 true 的 Job 本质上会将其创建为被挂起状态。

当 Job 被从挂起状态恢复执行时，其`.status.startTime` 字段会被重置为 `当前的时间`。这意味着 `activeDeadlineSeconds` 计时器会在 Job 挂起时 被停止，并在 Job 恢复执行时复位。

挂起 Job 会删除其所有活跃的 Pod。当 Job 被挂起时， Pod 会 收到 SIGTERM 信号而被终止。
Pod 以这种形式终止时，不会被记入 Job 的 `completions` 计数。
