# memblock

内核初始化完成后，系统的内存分配和释放由buddy系统、slab分配器来管理。但在buddy系统可用之前，内存分配和释放由`memblock`分配器来管理`物理内存`。

## 1. memblock 初始化

在 `bootloader` 做好初始化工作后，将 `kernel image` 加载到内存后，就会跳到kernel部分继续执行，跑的先是汇编部分代码，进行各种设置和环境初始化后，就会跳到kernel的第一个函数 `start_kernel`：

```bash
# arm64代码
start_kernel(void)
  | setup_arch(&command_line);
    | early_fixmap_init();  # fixmap区域页表映射初始化
    | early_ioremap_init(); # IO映射初始化

    # __fdt_pointer指针的赋值在head.S汇编代码中，bootloader传递过来，存放的是 dtb的物理地址
    # 	str_l	x21, __fdt_pointer, x5		// Save FDT pointer
    | setup_machine_fdt(__fdt_pointer);
      | fixmap_remap_fdt(...); # 映射pgd、pud、pte等，建立PTE entry映射才能访问物理地址
      | early_init_dt_scan(...); # 扫描 DTB 中的节点
        | memblock_add(...); # 获取到可用物理内存的地址和大小后，调用API添加一个memblock_region实例，用于管理这个物理内存区域

    # 当所有物理内存都添加进系统后，arm64_memblock_init对整个物理内存进行整理，主要工作是将一些特殊区域添加进reserved内存中。（比如：elfcorehdr、crashkernel、reserve、initrd）
    | arm64_memblock_init(); 
    
    # 后续内存相关工作就是进行内存页表映射了，完成从物理地址到虚拟地址的映射。
```

## 2. debug

开启memblock debug后，可以在/sys/kernel/debug/memblock查看相关布局
