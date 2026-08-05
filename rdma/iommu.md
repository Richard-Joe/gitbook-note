# IOMMU

## 一、IOMMU 是什么？

**IOMMU（Input/Output Memory Management Unit）** 是一个硬件单元，功能类似于 CPU 的 MMU，但专门为 I/O 设备（如网卡、GPU、存储控制器）服务。它负责：

- **地址转换**：将设备发出的 DMA 地址（I/O 虚拟地址）转换为真实的物理地址。
- **内存隔离**：为不同设备分配独立的地址空间，防止设备非法访问或 DMA 攻击。
- **中断重映射**（可选）：将设备中断请求映射到指定的 CPU 中断向量，支持虚拟化。
- **虚拟化支持**：实现安全的设备直通（PCIe Passthrough），允许虚拟机直接访问物理设备。

## 二、IOMMU 放在哪里？

**IOMMU 通常集成在 CPU 内部**，不是主板上的独立芯片。

| 平台 | 硬件位置 | 常见名称 |
| ------ | ---------- | ----------- |
| Intel x86 | CPU 内的内存控制器和 PCIe 根联合体 | VT-d |
| AMD x86 | CPU 内的数据架构和 PCIe 控制器 | AMD-Vi |
| ARM SoC | 系统内存管理单元模块 | SMMU |
| 老旧芯片组（已淘汰） | 主板北桥 | 独立 IOMMU 芯片 |

- 用户无法直接看到 IOMMU 芯片，它是 CPU 硅片上的逻辑模块。
- 可以通过 `dmesg \| grep -i iommu` 检查是否启用。

## 三、IOMMU 与网卡的关系

网卡是典型的 DMA 密集型设备，与 IOMMU 高度关联：

- **地址转换**：网卡 DMA 控制器发出的地址经过 IOMMU 转换为物理地址。驱动使用 `dma_map_single` 等 API 时，内核自动建立映射。
- **内存隔离**：防止恶意或 buggy 网卡访问内核内存、其他设备内存或虚拟机内存。
- **虚拟化 (SR-IOV/VFIO)**：将网卡直通给虚拟机时，IOMMU 确保虚拟机只能访问分配给它的内存区域。
- **性能影响**：启用 IOMMU 会增加少量延迟，但通过 IOTLB 缓存可缓解。对大多数网卡影响可接受；极低延迟场景可选择绕过（如 `intel_iommu=off`）。
- **高级特性**：支持 ATS（地址转换服务）、PRI（缺页请求）、PASID（进程地址空间 ID），实现共享虚拟内存（SVM）。

## 四、IOMMU 与 BIOS 的关系

BIOS/UEFI 负责 IOMMU 的早期初始化和配置：

- **启用/禁用**：BIOS 菜单中的 `VT-d`、`IOMMU`、`SVM Mode` 等选项控制硬件是否使能。
- **基本初始化**：设置 IOMMU 寄存器、地址范围、中断重映射结构。
- **构建 ACPI 表**：生成 DMAR（Intel）、IVRS（AMD）或 IORT（ARM）表，描述 IOMMU 设备、PCIe 关联、保留内存等。
- **操作系统引导**：内核读取 ACPI 表，接管 IOMMU（可重写配置或沿用 BIOS 设置）。如果 BIOS 完全禁用，内核无法强制开启。

常见问题：

- BIOS 开启但系统未生效 → 检查内核参数（如 `intel_iommu=on`）。
- BIOS 名称不一致 → Intel 主板叫 `VT-d`，AMD 叫 `IOMMU` / `SVM`。
- BIOS Bug → 可能导致 DMAR 表错误，需要更新 BIOS。

## 五、Linux 内核中 IOMMU 的实现

内核实现了一套完整的 IOMMU 管理框架，而不是简单开关硬件：

| 功能模块 | 说明 |
| ---------- | ------ |
| **硬件初始化和探测** | 解析 ACPI 表，识别 IOMMU 硬件，处理内核参数（`iommu=pt` 等） |
| **IOMMU 域管理** | 分配地址空间域（IDENTITY/DMA/UNMANAGED），为设备绑定域 |
| **DMA 映射后端** | 实现 `dma_map_single`/`dma_map_sg` 的 IOMMU 路径：分配 IOVA、建立页表、刷新 IOTLB |
| **设备直通与 VFIO** | 通过 VFIO 框架管理 IOMMU 组，支持安全地将网卡等设备分配给虚拟机 |
| **中断重映射** | 初始化中断重映射表，将设备 MSI/MSI-X 重定向到指定 CPU 向量 |
| **高级特性（ATS/PRI/PASID）** | 协商启用设备 ATS，处理缺页请求，管理 PASID 空间 |
| **性能优化与调试** | 大页映射减少 TLB 压力，debugfs/sysfs 暴露统计信息，错误记录与恢复 |
| **体系结构封装** | 通用层 `drivers/iommu/iommu.c` 提供 API，架构驱动（intel/amd/arm-smmu）实现细节 |

## 六、关键概念速查

- **IOTLB**：IOMMU 内部的 TLB 缓存，加速地址转换。
- **IOVA**：IO 虚拟地址，设备看到的 DMA 地址。
- **IOMMU 域 (Domain)**：一组页表，对应一个地址空间，可实现设备间隔离。
- **IOMMU 组 (Group)**：硬件隔离的最小单位（通常整个 PCIe 端点及其功能）。同组设备必须一起直通。
- **VFIO**：用户态驱动框架，依赖 IOMMU 实现安全的设备访问。

## 七、总结

> **IOMMU 是 CPU 内部的一个硬件单元，为网卡等所有 PCIe 设备提供地址转换和内存隔离。BIOS 负责启用并报告 IOMMU 拓扑，Linux 内核则实现完整的域管理、DMA 映射、设备直通和中断重映射功能。它是现代系统虚拟化、安全和高性能 I/O 的基础。**
