# 进阶

## 1. 内存漫谈（Talking About Memory）

### 1.1 三个基本概念：值、位置、指针

- **值（value）**：由「类型」和「该类型取值域中的一个元素」组成。值可以按其类型的表示（representation）转成字节序列，但值的含义与它被存在哪里无关。
  - 例：`u8` 类型的 `6`，内存表示是字节 `0x06`；`"Hello world"` 的值表示是它的 UTF-8 编码。
- **位置（place）**：「可以存放值的地方」的 Rust 术语。位置可以在栈、堆或其他区域。最常见的 place 是**变量**——栈上一个有名字的值槽位。
- **指针（pointer）**：保存某个内存区域地址的值，指向一个 place，可通过解引用访问其中的值。同一个指针可以存进多个变量，从而让多个变量间接引用同一块内存、同一个值。

```rust
// 共 4 个值：42、43、x 的地址、y 的地址；共 4 个变量：x、y、var1、var2
let x = 42;
let y = 43;
let var1 = &x;     // var1、var2 存的是两个独立的指针副本
let mut var2 = &x;
var2 = &y;         // 只改 var2，var1 不受影响；`=` 把右值存入左值命名的 place

// string 的实际值是指向字符串首字符的指针（还含长度，即胖指针），并非字符串本身
let string = "Hello world";
```

### 1.2 变量的两种心智模型

#### 高层模型（High-Level Model）——用于理解生命周期与借用

- 变量只是值的**名字**；借用检查器真正检查的是每个借用从 **创建** 到 **最后一次使用** 的 **生命周期区间**：

```rust
let mut x;              // x 未初始化：赋值前不能使用
x = 42;                 // [1] x 获得值

let y = &x;             // [2] 共享借用 &x 开始（y 借用 x）
x = 43;                 // [3] ✗ 对 x 进行可变访问，但 &x 仍存活 → 冲突
assert_eq!(*y, 42);     // [4] y 最后一次使用：&x 的生命周期延伸到 [4] 才结束

// &x 的生命周期 = [2] ─────────── [4]，覆盖了 [3]，所以借用检查失败。
// NLL（非词法生命周期）：借用结束于「最后一次使用」，而不是作用域末尾。
// 删除 [4] 后，&x 的生命周期在 [2] 之后即结束，[3] 不再冲突，代码可以编译。
```

- 借用规则：同一时刻，一个值要么有**任意多个共享借用（`&`）**，要么只有**一个可变借用（`&mut`）**，二者不能并存；借用也不能比其所有者活得更久。
- **flow 模型的规则**：
  1. 每条 flow 从「创建点」开始，到「最后一次使用点」结束（不是到作用域结束）——这是 NLL 的核心。
  2. 共享 flow（`&`）与可变 flow（写 / `&mut`）不能重叠。
  3. 共享 flow 与共享 flow 可以重叠（多个 `&` 共存没问题）。

- **遮蔽（shadowing）**：同名重新声明得到的是**不同的变量**，两个变量共存，只是后续代码无法再命名旧变量。

```rust
let x = 1;
let y = &x; // y 借用变量 A
let x = "hello"; // shadowing：创建变量 B，和 A 无关
println!("{}", x);
println!("{}", y); // 仍然打印 1，y 借的是 A
```

#### 低层模型（Low-Level Model）——用于推理 unsafe 代码与裸指针

- 变量是内存位置的**值槽（value slot）**，槽里可能有合法值，也可能为空。
  - 赋值 = 填槽，旧值（如有）被 drop 并替换；
  - 访问时编译器检查槽非空，未初始化或值已被 move 都会报错；
  - `&x` 指向变量的底层内存，对 `x` 赋值不会改变 `&x`；
- 与 C/C++ 等底层语言的内存模型一致。
- 注意：低层模型同样是简化——编译器可能把**不需要内存地址**的变量放进**寄存器**（例如未被取地址的局部变量）；而取 `&x` 会要求 `x` 有一个可供引用的内存位置。

```rust
let a = 10;      // 未被取地址：可能只存在于寄存器中
let b = a * 2;   // 甚至可能被常量折叠为 20

let c = &b;      // 取地址：要求 b 有一个可供引用的内存位置
println!("{c}");
```

### 1.3 内存区域（Memory Regions）

对写 Rust 最重要的三个区域：**栈（stack）**、**堆（heap）**、**静态内存（static memory）**。

#### 栈（The Stack）

- 程序用于函数调用的暂存区。每次函数调用在栈顶分配一块连续内存，称为**帧（frame）**；`main` 的帧靠近栈底，函数调用其他函数时不断压入新帧。
- 帧中保存该函数的所有局部变量和参数；函数返回时帧被回收。
- 帧的字节**不会被立即擦除**，但访问不安全：可能已被后续复用的帧覆盖，也可能含非法值（如返回时被 move 掉的值）。
- 栈帧终将消失这一事实与 Rust 的**生命周期**紧密相关：栈上变量的任何引用，其生命周期**最多和该帧一样长**。

#### 堆（The Heap）

- 不依附于当前调用栈的内存池。堆上的值一直存活，直到被**显式释放（deallocate / free）**。
- 适用于值需要活得比当前函数帧更久的场景：
  - 返回值不一定要上堆：调用方可以预先在自己的栈上预留空间，让被调函数写入；
  - 但若要把值发送给另一个线程（可能不共享任何栈帧），就必须放到堆上。
- 堆允许显式分配连续内存段，返回指向段起始处的指针；该内存在释放前一直保留。堆分配得到的指针**生命周期不受约束（unconstrained lifetime）**——不绑定任何栈帧，只要程序让它存活，它就一直有效。
  - 注意：「不受约束」是相对于栈帧而言的；从 `Box` 借用出的 `&T` 仍受借用检查器约束，不能比 `Box` 活得更久。
- Rust 与堆交互的主要机制是 **`Box`**：`Box::new(value)` 把值放到堆上，返回的 `Box<T>` 是指向该堆值的指针；`Box` 被 drop 时释放内存。
- 忘记释放会造成**内存泄漏（leaking memory）**，最终耗尽内存；但有时故意泄漏，例如只读配置需要整个程序访问，可用 `Box::leak` 得到 `'static` 引用。

#### 静态内存（Static Memory）

- 编译进二进制文件中的若干相关内存区域的统称，程序启动时自动加载，其中的值在**整个程序执行期间**存活。
- 包含：程序的二进制代码（text 段，通常映射为只读）、`static` 关键字声明的变量、某些常量值（如字符串字面量）。

#### `'static` 生命周期

- 名字来源于静态内存区，表示引用有效「和静态内存一样久」——直到程序关闭。
- 静态变量的内存在程序启动时分配，因此指向它的引用必然是 `'static`；**反之不成立**：`'static` 引用未必指向静态内存（如 `Box::leak` 得到的堆内存）。但名字仍然贴切：引用一旦具有 `'static` 生命周期，它指向什么对程序其余部分来说就像在静态内存里一样，想用多久就用多久。
- 实际更常遇到的是 trait bound 中的 `'static`：`T: 'static` 表示 T 能活到我们希望它活的那么久（最长到程序结束），并不要求它真的活那么久；本质要求 T **拥有所有权、自给自足**——不借用非静态值，或它借用的东西也是 `'static`。
- 例：`std::thread::spawn` 要求传入的闭包是 `'static`，因为新线程可能比当前线程活得久，不能引用旧线程栈上的任何东西。

#### `const` 与 `static` 的区别

- `const` 声明**常量项**：可在编译期完全计算，引用它的代码在编译时被替换为计算好的值；常量**没有内存或其他存储**（不是 place），可理解为给某个值起的方便名字。
- `static` 变量则拥有实际的内存位置，在整个程序执行期间存活。

## 2. 所有权（Ownership）

### 2.1 单一所有者与移动（Move）

- Rust 内存模型的核心：每个值有且只有一个**所有者（owner）**——通常是一个作用域，由它负责最终释放该值。该规则由**借用检查器**强制执行。
- **移动（move）**：把值赋给新变量、push 进 `Vec`、放到堆上等操作，会把所有权从旧位置转移到新位置。
  - 移动后，通过原所有者的变量**不能再访问该值**——尽管组成该值的比特位仍在原处。
  - 必须通过指向新位置的变量来访问。

### 2.2 Copy 语义

- 实现了 `Copy` trait 的类型是例外：重新赋值时值**不发生移动，而是被复制**，新旧位置都可继续访问。相当于在目的地构造了一个完全相同的实例。
- 实现 `Copy` 的前提：仅靠**复制比特位**就能复制该值。因此排除：
  - 包含非 `Copy` 类型的类型；
  - 任何拥有需要在 drop 时释放的资源的类型。
- 为什么 `Box` 不能是 `Copy`：因为 `Box<T>` 是拥有所有权的智能指针。若 `box2 = box1` 也能复制，两个 Box 都会认为自己拥有同一块堆内存，作用域结束时会**双重释放（double free）**，后果可能是灾难性的。
- Rust 的大多数基本类型（整数、浮点数等）都是 `Copy`。

### 2.3 Drop 与自动清理

- 所有者不再使用值时，负责通过 **drop** 完成清理；Rust 在持有值的变量**离开作用域**时自动执行 drop。
- 类型通常会**递归地** drop 其包含的值，因此 drop 一个复杂类型的变量可能连带 drop 许多值。
- 由于「离散所有权」的要求，同一个值**不可能被 drop 两次**。
- 持有引用的变量**不拥有**被引用的值，因此该变量 drop 时，被引用的值不会被 drop。

### 2.4 移动与复制示例

```rust
let x1 = 42;
let y1 = Box::new(84);
{ // 开启新作用域
    let z = (x1, y1);   // [1] x1 被复制进 z，y1 被移动进 z
    // z 离开作用域被 drop；它又依次 drop 从 x1 复制的值和从 y1 移动来的值，其中 `Box` 被 drop 时还会释放堆上存储 `84` 的内存。
}                       // [2]
let x2 = x1;            // [3] x1 是 Copy，未被移动，可继续使用
// let y2 = y1;         // [4] y1 不是 Copy，已被移动，此行会编译错误
```

### 2.5 Drop 顺序

- **变量（含函数参数）按声明的逆序 drop**。
  - 原因：按声明顺序 drop 会有悬垂引用问题。
- **嵌套值（元组、数组、结构体字段）按源码顺序 drop**（第一个元素 / 字段先 drop）。
  - 原因：对用户更直观；且 Rust 目前不允许单个值内部存在自引用，因此不需要逆序。

## 3. 借用与生命周期（Borrowing and Lifetimes）

### 3.1 引用：带契约的指针

- 所有者可以**出借**值而不放弃所有权——这正是引用的作用。
- 引用是**指针 + 契约**：契约规定它是共享访问还是独占访问、同一时刻是否允许其他引用指向同一个值。

### 3.2 共享引用（`&T`）

- 同一个值可以有**任意多个**共享引用同时存在；`&T` 是借用，不拥有所有权，所以是 `Copy`，复制引用很廉价。
- 共享引用背后的值**不可变**：不能修改、不能重新赋值，也不能把 `&T` 强转成 `&mut T`。

**契约** 和 **编译器优化**

- 写代码要面向「编译器**被允许**做什么」，而不是「这次编译碰巧优化了什么」
- **契约**：编译器可以假设共享引用存活期间，其指向的值不会改变。因此同一函数内多次读 `*input`，编译器  **有权**  只读一次并复用结果。

```rust
fn sum_via_shared(input: &i32, sum: &mut i32) {
    *sum = *input + *input;          // 编译器可以把 *input 优化成只读一次
    assert_eq!(*sum, 2 * *input);    // 因此这个断言恒成立
}
```

- 如果你违反了契约，编译器不保证按你的直觉生成代码。正确的做法是：显式声明内部可变性，让编译器知道这里可能变。

### 3.3 可变引用（`&mut T`）

- **契约：独占性。** 编译器假设目标值没有任何其他访问者——没有共享引用、没有其他可变引用、也没有其他线程。这也是它能做激进优化的依据。

#### noalias 优化

```rust
fn noalias(input: &i32, output: &mut i32) {
    if *input == 1 {
        *output = 2;      // [1]
    }
    if *input != 1 {      // [2]
        *output = 3;
    }
}
// 编译器可假设 input 与 output 不指向同一内存：
// [1] 不可能影响 [2] 的判定 → 整个函数可优化成一个 if-else。
// 若没有独占契约，noalias(&x, &mut x) 在 input == 1 时会走到 output = 3，
// 这个优化就不再成立。
```

#### 可变性只作用于「直接指向的那块内存」

```rust
let x = 42;
let mut y = &x;     // y: &i32
let z = &mut y;     // z: &mut &i32
// 通过 z 可以让 y 改指别处，但不能改 x；
// z 本身也不能改指别的引用。
// 能否修改更深的值得看中间类型提供的方法（Deref/DerefMut、内部可变性等）。
```

#### 拥有值 vs 持有 `&mut`

- 本质区别只有**drop 责任**：所有者负责在不再需要时释放值。
- 其余操作都能通过 `&mut` 完成，但有一条硬约束：**从 `&mut` 背后移出值时，必须在原位留下另一个合法值**。否则调用方仍以为自己拥有旧值、稍后会再 drop 一次 → 等价于 double free。

```rust
fn replace_with_84(s: &mut Box<i32>) {
    // let was = *s;                  // ✗ 直接移出会让 *s 变成空槽，调用方仍会 drop 它
    let was = std::mem::take(s);     // [1] 移出旧值，原位留下 Default::default()
                                     //     等价于 mem::replace(s, Box::default())
    *s = was;                        // [2] 把旧值写回；留下的默认值在此刻被 drop
    let mut r = Box::new(84);
    std::mem::swap(s, &mut r);       // [3] 交换两个 &mut 背后的值，双方所有者都能拿到合法值
    assert_ne!(*r, 84);
}

let mut s = Box::new(42);
replace_with_84(&mut s);
```

### 3.4 内部可变性（Interior Mutability）

- 含义：某些类型允许**通过共享引用**修改内部值。安全性不靠「独占引用」，而靠额外机制（原子 CPU 指令、类型不变量）。
- 分两类：
  1. **能从 `&self` 拿到内部 `&mut` 的类型**：`Mutex`、`RefCell`。它们保证交给外部的可变引用同一时刻有且只有一个，且不并存共享引用。底层都依赖 `UnsafeCell`——**通过共享引用修改值的唯一正确方式**（细节见第 10 章）。
  2. **不给出内部引用、只提供原地操作方法的类型**：`Cell<T>`、`std::sync::atomic` 中的原子类型。拿不到内部 `i32`/`usize` 的引用，只能整体读取或替换。
- `Cell` 的安全性来自两条不变量：
  - 不跨线程共享；
  - 从不把内部值的引用交出去——没有引用存在，就永远可以安全地整体移动内部值；不能跨线程，就保证通过 `&` 修改时不会并发。

### 3.5 生命周期（Lifetimes）

- 生命周期是「某个引用必须有效的一段**代码区域**」的名字。
- 它**经常**与作用域重合，但**不必**等于作用域，甚至可以**不连续**（见 3.6）。
- 新手的「生命周期 = 作用域」模型在多数日常代码里够用，但分析复杂借用时会失真。

### 3.6 生命周期与借用检查器

- 核心规则：**每当使用一个生命周期为 `'a` 的引用时，借用检查器会从使用点回溯到 `'a` 的起点（取引用处），检查这条路径上是否存在冲突使用。**
- 这正是 1.2 的 flow 模型：把 `'a` 看作从创建点到使用点的一条 flow，检查它与其他并行 flow 是否冲突。

#### 生命周期不必连续

```rust
let mut x = Box::new(42);
let r = &x;              // [1] 'a 起点
if rand() > 0.5 {
    *x = 84;             // [2] 需要 &mut x
} else {
    println!("{}", r);   // [3] 'a 的使用（最后一次）
}
// [4]
```

- [2] 处 `r` 仍在作用域内（直到 [4]），但走该分支时 **`r` 之后不再被使用**，即 `'a` 没有延伸进该分支。没有从 `r` 跨越 [2] 的 flow → 可变访问合法。
- [3] 使用 `r`：回溯到 [1] 的路径不经过 [2] → 合法。
- 若在 [4] 再加一次对 `r` 的使用，`'a` 就延长到 [4]，[2] 落在回溯路径上成为冲突 → 编译失败。

#### 生命周期可以有「洞」

```rust
let mut x = Box::new(42);
let mut z = &x;                  // [1] 'a 起点
for i in 0..100 {
    println!("{}", z);           // [2] 'a 使用
    x = Box::new(i);             // [3] 移动并替换 x：旧 'a 在 [2] 之后结束
    z = &x;                      // [4] 'a 重新开始
}
println!("{}", z);               // [5] 'a 使用
```

- [3] 移动 `x` 时旧生命周期已在最后一次使用（[2]）后结束，因此不冲突；[4] 重新给 `z` 赋值，相当于从该点起一条**新的 flow**。
- 此后无论循环回到 [2] 还是走到 [5]，都有有效起点且无冲突 → 通过。
- flow 模型下毫不奇怪：`x` 被移动时 `z` 就不存在了；重新赋值 `z` 即是「新变量从此点开始存在」。

### 3.7 泛型生命周期（Generic Lifetimes）

- 类型可以像泛型于类型一样，**泛型于一个或多个生命周期**，让借用检查器在使用处校验内部存储的引用。
- 两个容易忽略的细节：
  1. **`Drop` 算作一次使用**：若类型实现了 `Drop`，drop 该类型时，借用检查器会检查其泛型生命周期（及类型参数）此时仍然合法——因为 drop 代码可能访问这些引用。若类型不实现 `Drop`，drop 不算使用：用户可以提前让类型内的引用失效，只要此后不再使用该值。
  2. **通常一个生命周期参数就够**：编译器会把插入类型中的所有引用统一取**较短**的那个生命周期。只有当类型确实包含多个引用、且方法返回的引用只应绑定其中某一个引用时，才需要多个参数。

#### 为什么有时必须用多个生命周期

```rust
struct StrSplit<'s, 'p> {
    delimiter: &'p str,
    document: &'s str,
}

impl<'s, 'p> Iterator for StrSplit<'s, 'p> {
    type Item = &'s str;   // 产出只与 document 的生命周期绑定
    fn next(&self) -> Option<Self::Item> { todo!() }
}

fn str_before(s: &str, c: char) -> Option<&str> {
    // delimiter 指向函数内的临时 String；返回值只应来自 document
    StrSplit { document: s, delimiter: &c.to_string() }.next()
}
```

- 若 `StrSplit` 只有一个生命周期，`Item` 会同时绑定 `document` 和 `delimiter`：`str_before` 的返回引用就会带上局部临时量 `c.to_string()` 的生命周期 → 借用检查器拒绝。
- 拆成 `'s` / `'p` 后，返回值的生命周期只与 `'s` 绑定，`'p` 可以是任意长度（包括临时值的生命周期）→ 代码成立。

### 3.8 生命周期型变（Variance）

- **子类型**：谁比谁活得更久。长生命周期引用 是 短生命周期引用 的子类型。
  - 例：`'b: 'a`，读作 `'b outlives 'a`（`'b` 活得比 `'a` 长）⇒ `'b` 是 `'a` 的子类型；
- 子类型的本质是可替换性（substitutability）：一个生命周期更长的引用 `&'long T`，可以安全地当作生命周期更短的引用 `&'short T` 使用。
- `'static` 是所有生命周期里最长的，`'static: 'a`永远成立，`'static` 是任意 `'a` 的子类型。
- 所有类型都有型变，规定「什么相似类型可以替代它出现在该位置」。分三种：

```text
时间 ──────────────────────────►
'b:  [══════════════════════]     （长）
'a:        [══════════]            （短，被 'b 包含）

此时  'b: 'a 成立
      &'b T <: &'a T  成立
```

#### 协变（Covariant）

- 可以直接用**子类型**替代该类型。
- `&'a T` 对 `'a` **协变**：`&'static str` 可以传给要求 `&'a str` 的地方。
- `&'a T` 对 `T` 也协变：`&Vec<&'static str>` 可以传给要求 `&Vec<&'a str>` 的地方。

#### 不变（Invariant）

- 必须精确匹配，子类型和超类型都不能替代。
- `&mut T` 对 `T` **不变**：要求 `&mut Vec<&'a str>` 的函数，不能传入 `&mut Vec<&'static str>`。
  - 原因：若能传，函数可以往 `Vec` 里塞一个短生命周期的字符串；调用方仍以为手里是 `Vec<&'static str>`，继续使用其中的元素就会产生悬垂引用。
- 推广：**提供可变性的类型一般对其类型参数不变**（如 `Cell<T>` 对 `T` 不变）。

#### 逆变（Contravariant）

- 出现在函数参数位置：参数要求越**宽松**，函数越有用。

```rust
let x: &'static str;   // 更有用：活得更久
let x: &'a      str;   // 较弱：活得较短

fn take_func1(&'static str)   // 要求更严 → 函数更不好用
fn take_func2(&'a str)        // 要求更松 → 函数更好用
// 类型本身是协变的，放进函数参数后关系翻转 ⇒ Fn(T) 对 T 逆变
```

#### 为什么生命周期要关心型变

```rust
struct MutStr<'a, 'b> {
    s: &'a mut &'b str,
}

let mut s = "hello";
*MutStr { s: &mut s }.s = "world";   // [1]
println!("{}", s);                   // [2]
```

- 看起来没有方法需要区分结构的不同部分，一个生命周期就该够；但**合并成一个 `'a` 后无法编译**，原因完全是型变：
  - **双生命周期**：`'a` = 借用 `s` 的可变借用的生命周期（[2] 之前结束），`'b` = `"hello"` 的 `'static`。
  - **单生命周期 `'a`**：构造 `MutStr` 时 `&'b str` 是 `&'static str`，编译器只能把 `'a` 推断为 `'static`。[2] 处要打印 `s`，需要缩短对 `s` 的可变借用；而 `'a` 同时是内层 `&'a str` 的生命周期，缩短它意味着要缩短 `&'static str`。`&'a T` 本可协变地把 `&'static str` 缩成 `&'a str`，**但它此刻位于 `&mut T` 之下，而 `&mut T` 对 `T` 不变** → 缩短失败，报「仍被可变借用」。
- **实践结论**：尽量让类型在其泛型参数上保持协变（该逆变处逆变）。若为此需要增加生命周期参数，就在「多一个参数的认知成本」与「不变性带来的易用性成本」之间权衡。

## 4. 内存中的类型（Types in Memory）

### 4.1 类型决定比特如何被解释

- 每个 Rust 值都有类型。类型在本章还有很多用途，但最基本的作用之一是：**告诉编译器如何解释一段内存中的比特**。
- 同一串比特在不同类型下含义完全不同：

```text
0b10111101（十六进制 0xBD）
  按 u8 解释 → 189
  按 i8 解释 → -67（最高位为 1，按补码解释为负数）
```

- 定义自己的类型时，决定「该类型的内存表示中各个部分放在哪」是编译器的工作：
  - 结构体的每个字段出现在这串比特的什么位置？
  - 枚举的判别式（discriminant）存在哪？
- 开始写更进阶的 Rust 代码后，理解这个过程就很重要：这些细节同时影响代码的**正确性**与**性能**。

### 4.2 枚举的判别式（Discriminant）

- 4.1 末尾留下了一个问题：枚举的判别式存在哪？先明确它是什么：枚举同一时刻只持有**一个**变体，程序运行时靠**判别式（discriminant）**区分当前是哪个变体——`match`、`if let` 以及 `std::mem::discriminant` 的比较，最终都落在判别式上。
- 判别式是一个整数，默认按变体声明顺序从 0 开始编号，也可以显式指定：

```rust
enum Color { Red, Green, Blue }             // 判别式：0、1、2

#[repr(u16)]
enum Status { Ok = 200, NotFound = 404 }    // 404 超出 u8 范围，所以用 u16

// #[repr(u8)]
// enum Bad { NotFound = 404 }              // ✗ 404 does not fit into u8 (0..=255)

fn main() {
    assert_eq!(Color::Green as u8, 1);      // 无字段枚举可直接取出判别式
}
```

- **判别式的类型默认是 `isize`**，可以用 `repr(u8)` / `repr(u16)` 这类 primitive representation 指定成对应的整数类型。没有指定时，显式判别式必须写成 `isize` 能表示的值（写 `V = 0u8` 会报 `expected isize, found u8`）。
- **类型不等于内存占用**：布局里编译器可以用更小的整数类型存判别式（3 个变体 → 1 字节），甚至用 niche 完全不单独存（见下文三种情况）；Reference 的示例也只断言 `size_of::<E>() <= size_of::<isize>()`。
- `std::mem::discriminant(&x)` 返回不透明、但可比较的 `Discriminant<T>`：当你只关心「两个值是不是同一个变体」、不想（也无法）拿到具体数值时用它。
- **判别式存在哪里、实际占多少空间，取决于枚举的形态**——这正是下面三种情况的区别。

#### 无字段枚举（fieldless enum）

- 所有变体都不带载荷（C 语言风格）。此时没有别的数据要存，**整个值就是一个整数**：内存里存的就是判别式。
- 存这个整数用多大类型由编译器决定：`repr(Rust)` 不做保证；`rustc` 的做法是选能装下所有判别式值的最小整数类型——变体较少时 1 字节，变体超过 256 个时才升到 `u16`（有负判别式则改用有符号类型）。
- 于是 `size_of::<Color>()` 通常是 1，而不是 `size_of::<isize>()`（8 字节）：判别式虽以 `isize` 为类型，存储上却小得多。

```rust
enum Color { Red, Green, Blue }    // 3 个变体 → 判别式占 1 字节
// size_of::<Color>() == 1，而 size_of::<isize>() == 8（64 位平台）
```

#### 有字段枚举（带载荷的枚举）

- 变体带数据时，各变体的载荷不会同时存在，因而可以**共享同一块空间**；但「当前是哪个变体」必须另外记住 → 布局相当于**带标签的联合体（tagged union，tag + payload）**：
  - 载荷区：取所有变体中最大的载荷，按最大对齐向上取整；
  - 判别式字段（tag）：与载荷并存的隐藏小整数字段，大小取决于变体数量（逻辑类型仍是 `isize`，但内存里的 tag 通常只有 1 字节）；
  - 总大小 = 载荷区 + tag，再按对齐填充。
- 举例：

```rust
enum Shape {
    Point,                    // 无载荷
    Circle(f64),              // 载荷 8 字节
    Rect { w: u32, h: u32 },  // 载荷 8 字节
}
// 64 位平台上通常为 16 字节：载荷 8 字节 + 1 字节 tag，再按 8 字节对齐补齐
```

- `match` 时先读 tag，再决定进入哪个分支。
- 显式表示会把这个 tag 固定下来：`repr(C)` 使用 C 枚举类型（通常是 `i32`），`repr(C, u8)` / `repr(u8)` 使用 `u8`——换来稳定的 ABI，代价是放弃下面要讲的 niche 优化。
- 顺带一提：`repr(C)` 不是 primitive representation，所以它的判别式**值**类型仍是 `isize`，只是布局按 C 枚举走（`i32`）——又一个「逻辑类型 ≠ 存储大小」的例子。
- 默认 `repr(Rust)` 下，既不能假设 tag 的位置、大小，也不能假设它一定独立存在（见下一种情况）。

#### niche 优化（niche optimization）

- **niche**（法语原意是墙上的凹槽）Rust语境下，指某个类型**未被合法值占用的比特模式**，编译器可以把这些"空闲的坑位"拿来编码别的信息。常见的类型：
  - `&T`、`Box<T>`、函数指针：永不为 0（空指针非法）；
  - `NonZeroU32` 等 `NonZero*` 类型：不能为 0；
  - `bool`：只能是 0 或 1，其余 254 个位型全是 niche；
  - `char`：只覆盖合法的 Unicode 标量值。
- 如果某个变体的载荷里存在 niche，编译器可以把**无载荷的那个变体**编码进该 niche，于是不再需要额外的 tag：
  - 载荷落在 niche 位型上 → 表示无载荷变体（如 `None`）；
  - 否则 → 表示带载荷的变体。
- 此时判别式是**虚拟的**：它不在某个固定字段里，而是由载荷中的「非法值」兼职表示，因此不占额外空间。

```rust
use std::num::NonZeroU32;

// 64 位平台上，判别式被编码进空指针 / 0 这些 niche，外型尺寸不变：
// size_of::<Option<&u8>>()        == size_of::<&u8>()        == 8
// size_of::<Option<Box<u8>>>()    == size_of::<Box<u8>>()    == 8
// size_of::<Option<NonZeroU32>>() == size_of::<NonZeroU32>() == 4
```

- 适用的边界：
  - 只在默认 `repr(Rust)` 下成立。写出 `repr(C)`、`repr(u8)` 等显式表示后，tag 必须真实存在，优化不再适用——例如 `enum E { A(&'static u8), B }` 默认占 8 字节，改成 `#[repr(u8)]` 后就变成 16 字节（多出的 tag 被迫独立存放）。
  - 典型形态是「一个带 niche 的载荷变体 + 一个无载荷变体」（`Option<T>` 正是如此）；变体更多、或载荷里没有 niche 时，仍要真实 tag。
- 三种情况可以这样记：
  - **无字段枚举**：判别式 = 枚举值本身，整个值就是一个整数（逻辑类型 `isize`，实际通常压缩到 1 字节）；
  - **有字段枚举**：判别式 = 载荷旁的独立 tag 字段，与载荷并存；
  - **niche 优化**：判别式 = 载荷中的非法位型，没有独立字段，等于「免费」。
- 无论如何，`size_of` 得到的都只是**当前编译器的可观察行为**，不是 ABI 承诺；需要稳定布局时请显式写 `repr`。

### 4.3 对齐（Alignment）

- **为什么必须对齐**：指针指向的是**字节**而非比特，所以所有值至少必须**字节对齐**（地址是 8 比特的整数倍）。更严格的要求来自硬件：CPU 以块（64 位 CPU 通常是 8 字节，即**字长**）访问内存，并要求地址从块边界开始。
- **非对齐访问（misaligned access）的代价**：读一个跨块的 `i64` 要拆成两次访问再拼接——既慢，又可能因另一线程并发写入而读到前后不一致的**撕裂值**。
- **自然对齐（naturally aligned）**：对齐值等于其大小，如 8 字节加载要求 8 字节对齐地址。对齐访问更快、一致性更强，因此编译器为每个类型计算一个对齐值：
  - 内建类型按自身大小对齐：`u8` 1、`u16` 2、`u32` 4、`u64` 8；
  - 复合类型取所含类型中**最大**的对齐值（含 `u8`/`u16`/`u32` 的结构体 → 4 字节对齐）。

### 4.4 布局（Layout）与 `repr(C)`

- 类型在内存中的表示方式称为**布局（layout）**。Rust 默认使用 `repr(Rust)`，编译器几乎不做保证（见 4.5）。
- `#[repr(C)]` 要求按 C/C++ 的规则布局：字段按定义顺序存放，按对齐要求在字段间插入填充。布局可预测、不随版本变化——适合 FFI 交互（第 12 章），也方便 unsafe 代码直接用裸指针访问或做类型转换。
- `#[repr(transparent)]`：只能用于**只有一个字段**的类型，保证外层类型与内层类型的布局**完全一致**；配合 newtype 模式（`struct A` 与 `struct NewA(A)`）时，可以把两者的内存表示当作相同。

同一组字段，两种布局：

```rust
#[repr(C)]
struct Foo {
    tiny: bool,
    normal: u32,
    small: u8,
    long: u64,
    short: u16,
}
// repr(C)    → 32 字节：按声明顺序布局，字段间有填充
// repr(Rust) → 16 字节：按大小重排字段，几乎无填充
```

### 4.5 默认布局（`repr(Rust)`）

- `repr(C)` 的主要限制是字段必须按定义顺序存放。`repr(Rust)` 取消了这一限制：编译器可以按大小降序重排字段，使字段本身满足对齐要求、几乎无需填充——这就是 `Foo` 从 32 字节降到 16 字节的原因。
- 代价是几乎不做布局保证：字段顺序不确定，甚至字段类型、顺序完全相同的两个类型也不保证布局相同。
- 这正是 Rust 默认几乎不保证布局的原因：给编译器更多重排自由，就能生成更高效的代码。

### 4.6 `repr(packed)` 与 `repr(align(n))`

- `#[repr(packed)]`：字段之间**不加任何填充**，每个字段按 **1 字节对齐**（类型自身对齐也变为 1），因此最省内存。代价是可能发生非对齐访问——既慢，某些要求对齐操作数的 CPU 指令还可能直接崩溃。只在内存吃紧、实例极多、或需要按紧凑格式收发数据时使用。
- `#[repr(align(n))]`：反过来，给类型或字段指定**大于自然要求的对齐值**。典型用途是让数组中的连续元素各占一条**缓存行（cache line，通常 64 字节）**，避免**伪共享（false sharing）**：两个 CPU 修改的明明是不同的值，却因共享同一条缓存行而互相争抢，导致性能大幅下降。

### 4.7 零大小类型（Zero-Sized Type, ZST）

- **ZST**：`size_of::<T>() == 0` 的类型，不占任何内存。但它仍是有完整语义的类型：所有权、借用、生命周期和 drop 检查都照常参与。
- 常见来源：
  - 单元类型 `()`；
  - 无字段结构体 `struct Empty;`，以及所有字段都是 ZST 的结构体；
  - 空数组 `[T; 0]`：大小为 0，但**对齐值仍等于 `T` 的对齐值**——`size_of::<[u64; 0]>() == 0`，而 `align_of` 为 8；
  - `PhantomData<T>`：不占空间，仅用于向编译器表达「逻辑上拥有 / 借用了 `T`」，并影响型变、drop 检查和 auto trait 推断。
- 关键性质：
  - **没有比特可读**：每个 ZST 类型至多只有一个值；复制、移动、遍历 ZST 都无需读写内存，相关循环在优化后通常整个消失。
  - **地址没有意义**：多个 ZST 可以共享同一地址；`Box::new(())` 不分配堆内存，`Vec<()>` 存任意多个元素也不占堆（长度照常记录）。
  - **引用仍需合法**：ZST 的引用必须非空、按对齐值对齐，不能拿空指针构造 `&()`。
- **ZST 没有 niche**：没有比特就没有非法位型可借用，因此需要判别式的枚举仍要真实 tag。例如 `()` 是 ZST，但 `Option<()>` 占 1 字节——需要 1 字节区分 `None` 与 `Some(())`。

```rust
use std::mem::{align_of, size_of};

assert_eq!(size_of::<()>(), 0);
assert_eq!(size_of::<Option<()>>(), 1);   // ZST 提供不了 niche，需要独立 tag
assert_eq!(size_of::<[u64; 0]>(), 0);
assert_eq!(align_of::<[u64; 0]>(), 8);    // 对齐跟着元素类型走
```

### 4.8 复合类型的内存表示

其他 Rust 类型的内存表示速查：

- **元组（Tuple）**：像结构体一样表示，字段类型依次为元组中各个值的类型，顺序相同。
- **数组（Array）**：所含类型的连续序列，元素之间没有填充。
- **联合体（Union）**：每个变体独立选择布局；对齐取所有变体中的最大值。
- **枚举（Enumeration）**：与联合体相同，但额外有一个隐藏的共享字段，用来存该枚举值的**变体判别式（discriminant）**。判别式是代码用来判断一个值当前持有哪个变体的值；判别式字段的大小取决于变体的数量。

### 4.9 动态大小类型与胖指针（DSTs and Fat Pointers）

- **DST（动态大小类型）**：编译期没有固定大小的类型，常见的有切片 `[T]`、`str` 和 trait 对象 `dyn Trait`。编译器无法为它们分配栈空间或计算字段偏移，因此 DST 不能按值使用，只能放到指针后面。
- **胖指针（fat pointer）是什么**：普通指针只含一个地址；指向 DST 的指针还要额外携带一个**字大小的元数据字段**来补全类型信息。「地址 + 元数据」的双字指针就是胖指针，它本身是 `Sized` 的。
- **`?Sized` 与胖指针的关系**：每个泛型参数都隐含 `T: Sized`；`?Sized`（意为「可以不满足」）放宽这一约束，让泛型能接受 DST。但 DST 仍然只能间接使用——指向它的引用 / `Box` 就是胖指针。可以记成：`?Sized` 决定「能不能用 DST」，胖指针负责「怎么表示它」。
- **元数据与大小**：
  - 切片 `&[T]`、`&str`：地址 + **长度**；
  - trait 对象 `&dyn Trait`：地址 + **vtable 指针**（方法、size、align、drop 的查找表）；
  - 两者都是 `2 * size_of::<usize>()`：64 位平台 16 字节，32 位平台 8 字节。
- **常见胖指针**：`&[T]`、`&str`、`&dyn Trait`，以及包着它们的 `Box` / `Rc` / `Arc`（如 `Box<[T]>`、`Box<dyn Trait>`）；自定义 DST 结构体（最后一个字段是 DST）的引用也属于此类，元数据取自最后一个字段。

```rust
use std::fmt::Debug;
use std::mem::size_of;

// 对 DST 的引用统一是两个 machine word
assert_eq!(size_of::<&[u8]>(), 2 * size_of::<usize>());
assert_eq!(size_of::<Box<dyn Debug>>(), 2 * size_of::<usize>());
```

## 5. Trait 与 Trait 约束（Traits and Trait Bounds）

本章讨论更技术性的部分——泛型代码编译后变成什么、`dyn Trait` 调用背后发生了什么、实现 trait 时受哪些规则约束，以及 trait 约束的进阶写法。

### 5.1 编译与分发（Compilation and Dispatch）

#### 单态化（Monomorphization）

```rust
impl str {
  #[inline]
  pub fn contains<P: Pattern>(&self, pat: P) -> bool {
    pat.is_contained_in(self)
  }
}

pub trait Pattern: Sized {
  /// Associated searcher for this pattern
  type Searcher<'a>: Searcher<'a>;

  /// Constructs the associated searcher from
  /// `self` and the `haystack` to search in.
  fn into_searcher(self, haystack: &str) -> Self::Searcher<'_>;

  /// Checks whether the pattern matches anywhere in the haystack
  #[inline]
  fn is_contained_in(self, haystack: &str) -> bool {
    self.into_searcher(haystack).next_match().is_some()
  }
}

// Pattern 的实现者只提供「搜索器」：写出 Searcher<'a> 的具体类型并实现 into_searcher；
// is_contained_in 直接复用默认实现，不必逐类型重写。
// 三种 P 对应的搜索器分别是 CharSearcher、StrSearcher、CharPredicateSearcher<F>。
// 调用处出现三种 P，编译器就为每个 P 各实例化一份 contains（源码仍只有一份，无需手写）：
assert_eq!("hello".contains('l'), true);  // P = char：生成 contains<char>，内部走 CharSearcher
assert_eq!("hello".contains("ll"), true); // P = &str：生成 contains<&str>，内部走 StrSearcher
assert_eq!("hello".contains(|c| c == 'l'), true); // P = 闭包 F：生成 contains<F>，内部走 CharPredicateSearcher<F>
```

- 为什么必须为每个 `P` 复制一份 `contains`：`pat.is_contained_in(self)` 最终要变成一条**跳转指令**，CPU 需要一个具体地址；`char`、`&str`、闭包各自的调用链（`into_searcher` → 搜索器）不同，地址也就不同，不存在「对任意类型通用」的地址。所以编译器按每个实际用到的 `P` 各**实例化**一份 `contains`——这就是**静态分发（static dispatch）**。（「静态」指编译期已知，因而可以写进静态内存。）
- 「泛型 → 许多非泛型实例」的过程叫**单态化（monomorphization）**。
  - 并非全文复制：只复制**用到的部分**——若从不调用 `Vec<i32>::find`，`find` 的那份拷贝就不会生成。
- 单态化是泛型 Rust 通常与非泛型 Rust 一样快的原因：优化器开工时泛型已不存在，每个实例带着完整的具体类型信息单独优化；源码里的 `#[inline]` 让 `contains<char>` 内部的调用可以一路内联，甚至化简成对字符的直接比较。
- 代价：
  - 同一泛型的所有实例要**分别编译** → 编译时间变长；
  - 每个实例各占一份机器码 → 二进制更大；
  - 逻辑相同的指令在实例间**不共享** → CPU 指令缓存效率下降。

#### 非泛型内层函数

- 泛型方法中往往只有小部分代码**依赖类型**。例如 `HashMap::insert`：计算 key 的哈希依赖 key 类型，而遍历桶、寻找插入点的代码不依赖。
- 把不依赖类型的部分抽成**非泛型辅助函数**，各单态化实例就能共享它的机器码，只有类型相关的部分被复制。把它声明在方法**内部**（内层函数），还能避免用一个一次性函数污染模块命名空间。
- 也可以声明在方法外面，但**不要**把它写成泛型 impl 块里的方法——那样它仍然会被单态化。

#### 动态分发（Dynamic Dispatch）

- 实践中传入的是指向 **vtable（虚方法表）** 的指针：表中存着该类型对 trait 每个方法实现的地址。vtable 还包含具体类型的**布局与对齐信息**（操作任何类型都离不开它）。想看显式 vtable，可参考 `std::task::RawWakerVTable`。
- 调用 trait 方法时：查 vtable 拿到地址 → 间接跳转执行。这样无论调用方传入什么类型，函数体都只有一份。
- **Trait 对象（trait object）** = 实现了某 trait 的类型 + 该类型的 vtable。
- `dyn Trait` 是 `!Sized`：编译期不知道调用方传入类型的大小，不能按值接收。必须放在**指针**后面（`&`、`&mut`、`Box`、`Arc` 等能容纳胖指针的指针都可以）；又因为要捎带 vtable 指针，它是**胖指针**——额外一个字存放 vtable 指针。

#### 对象安全（Object Safety）

- 大多数 trait 可以转成 trait 对象，不满足**对象安全（object-safe）**的除外：
  - 方法不能是**泛型的**。如 `Extend::extend` 对迭代器类型泛型，`dyn Extend` 的 vtable 需要为它可能被调用过的每个类型各留一个条目，无法固定。
  - 方法不能使用 `Self`（接收者 `&self` / `&mut self` / `self` 之外的 `Self`）。如 `Clone::clone` 返回 `Self`，在 `dyn Clone` 上调用时不知道该返回什么类型 → `Clone` 不是对象安全的。
  - 不能有**静态方法**（第一个参数不解引用到 `Self` 的方法），否则无法确定该调用哪个实例——`FromIterator::from_iter(&[0])` 究竟该执行哪份代码就说不清。
- **`Self: Sized` 约束**蕴含「`Self` 不会通过 trait 对象使用」（trait 对象是 `!Sized`，不满足该约束）：
  - 放在 trait 上 → 要求该 trait 永远不走动态分发；
  - 放在具体方法上 → 该方法在 trait 对象上不可用。
  - 带 `where Self: Sized` 的方法在对象安全检查中**豁免**。

#### 静态分发 vs 动态分发

- 动态分发的收益：不必编译多份拷贝 → 编译更快；指令缓存更友好。
- 动态分发的代价：编译器无法针对具体类型优化（vtable 另一端会执行什么代码不可知，例如无法内联）；每次调用多一次 vtable 查找。
- 如何选（经验法则）：
  - **库 → 静态分发**：把选择权留给用户。库若用动态分发，用户被迫跟随；库若用静态分发，用户仍可自己包一层 `dyn`。
  - **二进制 → 动态分发**：最终代码，没有下游需求要考虑；动态分发代码更干净（省去泛型参数）、编译更快，性能代价通常很小。

### 5.2 泛型 trait：类型参数与关联类型（Generic Traits）

- Trait 可以按两种方式泛型化：
  - 类型参数：`trait Foo<T>`
  - 关联类型：`trait Foo { type Bar; }`
- 区别初看并不明显，但选择规则很简单：**预期同一类型只有一份实现 → 关联类型；否则 → 类型参数**。更直白地说：能用关联类型就用关联类型（它通常好用得多）。
- 类型参数的成本：
  - 使用者必须写全所有参数，并重复参数上的约束；trait 每新增一个参数，所有使用处都要跟着改；
  - 同一类型可能有多份实现，编译器可能推断不出你指哪一份，需要难看的消歧调用，如 `FromIterator::<u32>::from_iter`。
- 类型参数的收益：**一个类型可以实现多份**。例如为自己的类型对多个右侧类型实现 `PartialEq`，或在 `T: Clone` 时同时实现 `FromIterator<T>` 与 `FromIterator<&T>`。
- 关联类型的收益：
  - 编译器只需知道「哪个类型实现了 trait」，关联类型随之全部确定 → 约束可以都写在 trait 里，使用处无需重复；trait 之后新增关联类型也不会影响使用方。
  - 关联类型由实现类型唯一决定 → 永远不会出现消歧问题。
- 关联类型的限制：一个类型只能有一份实现——不能对多个 `Target` 实现 `Deref`，也不能用不同的 `Item` 实现 `Iterator`。

```rust
// 类型参数：同一类型、多份实现
impl PartialEq<u32>    for MyId { /* ... */ }
impl PartialEq<String> for MyId { /* ... */ }

// 关联类型：给定类型只有一份实现
impl Iterator for Counter {
    type Item = u32;    // 只能有一个 Item
    fn next(&mut self) -> Option<u32> { /* ... */ }
}
```

### 5.3 一致性与孤儿规则（Coherence and the Orphan Rule）

- Rust 对「在哪里、为哪些类型实现 trait」有严格规则，目的是保持**一致性（coherence）**：对任意类型与方法组合，只有一个正确的实现可选。
  - 反例：若允许为 `bool` 写自己的 `Display` 实现，任何打印 `bool` 的代码都会同时看到你的实现和标准库的实现，编译器无从选择，也没有哪份更正确；两个相互依赖的 crate 为同一共享类型实现同一 trait 时同样如此。一致性保证编译器永远不必做这种选择。
- 两个朴素的极端都不可行：
  - 只有定义 trait 的 crate 能实现它 → 无法为自己的类型实现 `Debug`、`Serialize`，trait 基本失去价值；
  - 只能为自己的类型实现 trait → 上游 crate 无法为自己的 trait 提供 `Vec` 等标准库类型的实现。
- **孤儿规则（orphan rule）** 取其中间：**trait 或类型至少有一个属于本 crate** 时才能写 impl。于是：
  - `impl Debug for MyType` ✓（类型本地）
  - `impl MyNeatTrait for bool` ✓（trait 本地）
  - `impl Debug for bool` ✗（都是外部的）——编译器会报 conflicting implementations。
  - 术语：**upstream** = 你的代码依赖的东西；**downstream** = 依赖你的代码的东西。

#### Blanket 实现（Blanket Implementations）

- `impl<T> MyTrait for T where ...` 对一大类类型统一实现，称为 **blanket 实现**。
- 只有**定义 trait 的 crate** 能写 blanket 实现。
- 给已有 trait **新增 blanket 实现属于破坏性变更**：否则下游已有的 `impl MyTrait for Foo` 会因冲突而突然编译失败。

#### 基础类型（Fundamental Types）

- 少数类型被标记 `#[fundamental]`：目前是 `&`、`&mut`、`Box`。对孤儿规则而言它们**如同不存在**——检查前先被擦除。
  - 例：`impl IntoIterator for &MyType`。`IntoIterator` 和 `&` 都来自标准库，按孤儿规则本不合法；擦除 `&` 后只剩本地类型 `MyType` → 合法。
  - 给基础类型新增 blanket 实现同样属于破坏性变更。

#### 覆盖实现（Covered Implementations）

- 有时确实需要「外部 trait + 外部类型」，最典型的是 `impl From<MyType> for Vec<i32>`。`From` 与 `Vec` 都是外部的，但这并不威胁一致性：能与之冲突的实现只可能来自标准库的 blanket 实现（标准库否则无法命名 `MyType`），而那本来就属于破坏性变更。
- 为此孤儿规则开了一个**很窄的豁免**：`impl<P1..=Pn> ForeignTrait<T1..=Tn> for T0` 合法，当且仅当：
  1. 至少有一个 `Ti` 是**本地类型**；
  2. 在第一个本地类型之前的各位置（含 `T0`）中，**不出现裸泛型参数**——泛型参数可以出现，但必须被某个中间类型**覆盖**（如 `Vec<T>` 中的 `T`）；裸的 `T` 或隔着基础类型的 `&T` 都不算覆盖。

```rust
// ✓ 合法（ForeignTrait、From、Vec 均为外部；MyType 为本地类型）
impl<T> From<T> for MyType                  // T0 = MyType
impl<T> From<T> for MyType<T>               // T0 中的 T 被本地类型 MyType 覆盖
impl<T> From<MyType> for Vec<T>             // T1 = MyType；T0 中的 T 被 Vec 覆盖
impl<T> ForeignTrait<MyType, T> for Vec<T>  // 首个本地类型是 MyType，其前的 T0 覆盖了 T

// ✗ 不合法：找不到本地类型，或本地类型之前出现裸泛型参数
impl<T> ForeignTrait for T
impl<T> From<T> for T
impl<T> From<T> for Vec<T>
impl<T> ForeignTrait<T> for Vec<T>
```

- 规则是非对称的：

  ```rust
  impl<T> ForeignTrait<LocalType, T> for ForeignType   // ✓
  impl<T> ForeignTrait<T, LocalType> for ForeignType   // ✗
  ```

  看似武断：若允许后者，另一个 crate 就能写 `impl<T> ForeignTrait<TheirType, T> for ForeignType`；两个实现各自在各自 crate 里都没问题，只有两个 crate 被一起引入时才爆发冲突。要求「本地类型先于泛型参数」是一个**平局裁决（tie-break）**：保证每个 crate 单独 coherent 时，组合起来也 coherent。
- 破坏性变更的判定随之细化：给已有 trait 新增实现，**只有**当实现包含至少一个满足上述豁免条件的新本地类型时才是非破坏性的；其他新增实现都算破坏性变更。

### 5.4 Trait 约束（Trait Bounds）

标准库到处都是 trait 约束（`HashMap` 的 key 必须 `Hash + Eq`，`thread::spawn` 的闭包必须 `FnOnce + Send + 'static`）。写更复杂的泛型代码时，需要更强的约束表达能力。

#### 任意形式的约束

- 约束不必是 `T: Trait` 形式（`T` 也不必是泛型参数）。约束是**任意类型断言**，甚至可以不包含泛型参数、参数类型或本地类型：
  - `where String: Clone`——恒真也可以写；
  - `where io::Error: From<MyError<T>>`——泛型参数不必只出现在左边。
- 这种写法还能**省去重复约束**：比如方法要构造 `HashMap<T, usize, S>`，与其抄写 `where T: Hash + Eq, S: BuildHasher + Default`，不如写 `where HashMap<T, usize, S>: FromIterator`——不必去查底层方法确切的约束清单，也更能表达「代码真正需要什么」。

#### 关联类型上的约束

- 用 `Type::AssocType` 语法引用关联类型。例：标准库 `flatten` 产出的 `Flatten<I>` 要成为 `Iterator`，需要外层迭代器 `I: Iterator`，且 `I::Item: IntoIterator`——约束落在「外层迭代器的元素类型」上，而它正是 `I::Item`：

```rust
where
    I: Iterator,
    I::Item: IntoIterator,   // 对 I 产出的元素类型再加约束
```

- 同名关联类型有多个来源时（提供该关联类型的 trait 本身是泛型的，因此存在多份实现），用 `<Type as Trait>::AssocType` 消歧，如 `<I as Iterator>::Item`。

#### 高阶 trait 约束（Higher-Ranked Trait Bounds, HRTB）

- 有时约束要表达「某引用在**任意**生命周期下都实现某 trait」。有泛型生命周期参数就够时用不到它；这种写法很罕见（写作时标准库只有三处用到），但在 `Fn` 系列 trait 上很常见。
- 例：泛型于「接收 `&T`、返回其内部 `&U` 的函数」——`F: Fn(&T) -> &U` 需要为引用指定生命周期，而真正想说的是「输出与输入同生命周期」：
  - `F: for<'a> Fn(&'a T) -> &'a U`：对任意 `'a` 该约束都成立。
- 写 `Fn` 约束且参数带引用时，编译器会**自动补上 `for`**，这也是显式写法罕见的原因，但值得知道。
- 综合示例——为任意「可迭代、且元素可 debug」的集合实现 `Debug`：

```rust
impl Debug for AnyIterable
    where for<'a> &'a Self: IntoIterator,
          for<'a> <&'a Self as IntoIterator>::Item: Debug
{
    fn fmt(&self, f: &mut Formatter) -> Result<(), Error> {
        f.debug_list().entries(self).finish()
    }
}
```

- 这段实现复制到几乎任何集合类型上都能「直接工作」。

#### `#[derive]` 的隐式约束

- 许多 `#[derive(Trait)]` 展开为 `impl Trait for Foo<T> where T: Trait`。通常这正是想要的，但不总是。
- 例：`Foo<T>` 含有一个 `Arc<T>` 字段。`Arc<T>: Clone` 与 `T` 是否 `Clone` 无关，但 derive 出的 `Clone` 仍要求 `T: Clone`——凭空多出一条约束。
- 把类型改名为 `Shared<T>` 更能看清问题：`Shared<NotClone>` 本可以 clone（复制 `Arc` 即可），编译器却说不能，用户会很困惑。
- 写作时标准库的 `#[derive(Clone)]` 就是这样，未来可能改变。

### 5.5 标记 trait 与标记类型（Marker Traits and Marker Types）

- 通常 trait 表示**功能**（`Hash` 可 hash、`Clone` 可 clone、`Debug` 可 fmt）。**标记 trait（marker trait）** 不同：没有方法、没有关联类型，只表明实现类型的某种**性质**。
  - 例：`Send`——实现它表示可以安全地跨线程传递，没实现就是不可以。没有哪个方法对应「send 一下」，这只是一个关于类型的事实。
- `std::marker` 中的标记 trait：`Send`、`Sync`、`Copy`、`Sized`、`Unpin`。大多数（除 `Copy` 外）是 **auto trait**：编译器自动为类型实现，除非类型含有未实现该 trait 的东西。
- 价值：允许编写**代码里没有直接体现的语义要求**。要求 `Send` 的代码是在假设「这个类型可以拿到另一个线程使用」；没有标记 trait，编译器就无从检查这个假设，只能靠程序员记住它并仔细审阅代码——那条路通向数据竞争、段错误等运行时问题。
- **标记类型（marker type）**：与标记 trait 类似，但它是**单元类型**（如 `struct MyMarker;`），无数据无方法，用来在类型系统里标明「类型处于某种状态」。
  - 典型用途：让 API 的误用在编译期就不可能发生。
  - 例：`SshConnection` 可能已认证也可能未认证——加一个泛型参数，并定义两个标记类型 `Unauthenticated`、`Authenticated`：
    - 刚连接时得到 `SshConnection<Unauthenticated>`，其 impl 块**只**提供 `connect`；
    - `connect` 返回 `SshConnection<Authenticated>`，执行命令等其余方法只在已认证状态的 impl 块中提供。
  - 这是 **typestate（类型状态）** 模式的雏形，书中后续会继续展开。
