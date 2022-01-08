# CronJob

CronJob 周期性地在给定的调度时间执行 Job。

CronJob 仅负责创建与其调度时间相匹配的 Job，而 Job 又负责管理其代表的 Pod。

示例：`cronjob.yaml`
```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: hello
spec:
  schedule: "*/1 * * * *"
  jobTemplate:
    spec:
      template:
        spec:
          containers:
          - name: hello
            image: busybox
            imagePullPolicy: IfNotPresent
            command:
            - /bin/sh
            - -c
            - date; echo Hello from the Kubernetes cluster
          restartPolicy: OnFailure
  startingDeadlineSeconds: 10
  concurrencyPolicy: Allow
  successfulJobsHistoryLimit: 3
```

- `schedule`：时间都是基于`kube-controller-manager`的时区。
- `startingDeadlineSeconds`：每次运行 Job 的时候，它最长可以等多长时间。
	（有时这个 Job 可能运行很长时间也不会启动。所以这时，如果超过较长时间的话，CronJob 就会停止这个 Job）
- `concurrencyPolicy`：是否允许并行运行。
	（如果这个 policy 设置为 `Allow` 的话，那么不管你前面的 Job 是否运行完成，到点就会去执行；如果是 `Forbid`，它就会等上一个 Job 运行完成之后才会运行下一个；）
- `successfulJobsHistoryLimit`：每一次 CronJob 运行完之后，它都会遗留上一个 Job 的运行历史、查看时间。当然不能是无限的，所以需要设置一下历史存留数。


```bash
# 创建CronJob
kubectl apply -f cronjob.yaml

# 观察运行情况
watch kubectl get jobs
```

## cron时间表语法
```
# ┌───────────── 分钟 (0 - 59)
# │ ┌───────────── 小时 (0 - 23)
# │ │ ┌───────────── 月的某天 (1 - 31)
# │ │ │ ┌───────────── 月份 (1 - 12)
# │ │ │ │ ┌───────────── 周的某天 (0 - 6)（周日到周一；在某些系统上，7 也是星期日）
# │ │ │ │ │
# │ │ │ │ │
# │ │ │ │ │
# * * * * *
```

